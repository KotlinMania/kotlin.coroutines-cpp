## 0. Context and goal restatement

You’re porting Kotlin coroutines to C++ to form a “docking ring” between:

- Kotlin/Native runtime (GC + coroutines state machine lowering + continuation API)
- C++ (as a high‑performance / ABI‑interop target, including MLX / GPU ABIs)
- Future freethreaded Python embedding use cases

Your current method:

- mechanical transliteration of kotlinx.coroutines and Kotlin coroutine runtime pieces,
- plus direct study of kotlinc/Kotlin‑Native suspend lowering to reproduce the state machine exactly,
- plus early C++ prototypes that have since been replaced by the suspend DSL + Clang plugin approach.

You now want to go a step further:

- a C++‑side DSL that lets you write Kotlin‑style suspend logic in “normal looking C++,”
- with a build‑time compiler plugin (Clang/GCC extension or plugin) that rewrites that DSL into a Kotlin‑equivalent state machine,
- scoped to this repo only (so you accept the maintenance burden).

The key technical requirement for the plugin:

- Generated IR should match kotlinc/K/N patterns closely enough that semantics (and perf) are the same: spilled locals, label/resume logic,
  suspend marker propagation, and ideally indirect branch / jump‑table form.

———

## 1. How we found the relevant Kotlin/Native lowering code

### 1.1 Why we searched under tmp/kotlin

Your repo contains:

- tmp/kotlin/ – snapshot of the Kotlin monorepo (compiler + Kotlin/Native runtime + LLVM backend docs).
- tmp/kotlinx.coroutines/ – snapshot of upstream kotlinx.coroutines (the Kotlin source you transliterated).

Because we are specifically chasing Kotlin/Native suspend → LLVM lowering, the correct target is the Kotlin/Native compiler backend, not the
JVM/JS backends.

### 1.2 Search strategy (commands and rationale)

We used ripgrep over tmp/kotlin for the IR nodes and helper functions that Kotlin uses to represent and lower suspension:

1. Find where IrSuspensionPoint and IrSuspendableExpression are created / transformed.

rg -n "IrSuspensionPoint|IrSuspendableExpression|evaluateSuspensionPoint|evaluateSuspendableExpression" tmp/kotlin -S

- Rationale: those are the canonical IR nodes for suspend points in Kotlin IR.
- Output immediately pointed at Kotlin/Native lowering and codegen files:
    - NativeSuspendFunctionLowering.kt
    - CoroutinesVarSpillingLowering.kt
    - IrToBitcode.kt

2. Find low‑level LLVM constructs Kotlin emits (indirectbr, resume blocks, jump tables).

rg -n "indirectbr|jump table|resumePoints|label\s*field|state machine" tmp/kotlin -S

- Rationale: indirectbr is the LLVM IR instruction for computed goto / indirect dispatch; Kotlin/Native uses it for suspendable expressions.
- Output revealed the precise call site in IrToBitcode.kt and the helper in CodeGenerator.kt.

3. Cross‑check runtime intrinsics for spilling state.

rg -n "saveCoroutineState|restoreCoroutineState" tmp/kotlin/kotlin-native -S

- Rationale: Kotlin’s var‑spilling lowering rewrites these intrinsics into field stores/loads. We need their locations to mirror semantics.

This triangulation gives the full pipeline: IR lowering → liveness/spill → LLVM codegen.

———

## 2. What kotlinc/Kotlin‑Native does, and where in code

I’ll walk the pipeline in the same order kotlinc runs it.

### 2.1 Phase A: Decide if a suspend function needs a state machine

File:
tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt

How we located it:
Found by the IrSuspensionPoint / IrSuspendableExpression search.

Key logic:

- The lowering class NativeSuspendFunctionsLowering extends AbstractSuspendFunctionsLowering.
- It scans suspend functions and computes if they have any non‑tail suspend calls.
- If all suspend calls are tail calls, Kotlin does not generate a continuation class/state machine (tail‑suspend optimization).
  You see this check in tryTransformSuspendFunction:
    - collectTailSuspendCalls(context, function) returns (tailSuspendCalls, hasNotTailSuspendCalls)
    - If hasNotTailSuspendCalls == true, it builds the coroutine class and invokeSuspend.
    - Else it simplifies tail suspend calls and leaves the function direct.

Why this matters for you:

Your DSL/plugin needs the same optimization rule, or you’ll diverge in performance and shape. If you initially skip it, that’s fine (semantics
still correct), but keep it on the roadmap.

### 2.2 Phase B: Build the coroutine class + wrap body in IrSuspendableExpression

Still in NativeSuspendFunctionLowering.kt.

Key structure (high level):

- Kotlin/Native lowers a suspend function into:
    1. a coroutine implementation class (inherits ContinuationImpl / BaseContinuationImpl),
    2. an invokeSuspend(result) method on that class which is the state machine,
    3. a wrapper function that instantiates the coroutine and calls invokeSuspend.

Critical detail: Kotlin does not use a plain integer label in Native IR; it uses an opaque “suspensionPointId” which later becomes a block
address.

You see this when the pass constructs:

- IrSuspendableExpressionImpl(...)
- with suspensionPointId = irGetField(irGet(thisReceiver), labelField).

That labelField is the coroutine instance’s label storage.

### 2.3 Phase C: Liveness analysis and variable spilling

File:
tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/CoroutinesVarSpillingLowering.kt

How we located it:
The same IrSuspensionPoint search plus explicit saveCoroutineState search.

What it does:

- Runs on bodies of invokeSuspend (only those overriding invokeSuspendFunction).
- For each IrSuspensionPoint, retrieves liveVariablesAtSuspensionPoint (preferred) or visibleVariablesAtSuspensionPoint fallback.
- Builds private fields on the coroutine class for each spilled local (buildField { name = variable.name; type = variable.type; visibility =
  PRIVATE; isVar = true }).
- Rewrites calls to intrinsics:
    - saveCoroutineState() → stores all live variables into fields.
    - restoreCoroutineState() → loads all live variables from fields.

Runtime intrinsics declarations:
tmp/kotlin/kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/Coroutines.kt

- @TypedIntrinsic(IntrinsicType.SAVE_COROUTINE_STATE) external fun saveCoroutineState()
- @TypedIntrinsic(IntrinsicType.RESTORE_COROUTINE_STATE) external fun restoreCoroutineState()

Why this matters for you:

This file is a direct algorithmic spec for your plugin’s spilling phase:

- Determine live set across each suspend call.
- Allocate spill slots in the generated coroutine frame.
- Insert save/restore on the same schedule Kotlin does.

Your macro approach currently relies on manual spills by the programmer. The plugin can automate it to reach Kotlin parity and better UX.

### 2.4 Phase D: IR → LLVM, including the indirect branch block you vendored

File:
tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt

How we located it:

- evaluateSuspendableExpression and evaluateSuspensionPoint were found by searching those function names (also referenced in your docs/
  SUSPEND_IMPLEMENTATION.md).
- We opened the file around those line regions.

The mirrored core:

1. evaluateSuspendableExpression

High‑level meaning:

- It’s the wrapper for a block that may suspend.
- It creates:
    - bbStart (run the body fresh)
    - bbDispatch (resume path)
- It inspects suspensionPointId:
    - If null: branch to bbStart.
    - Else: branch to bbDispatch.
- In bbDispatch, it emits:
    - indirectBr(suspensionPointId, resumePoints)

This is the exact Kotlin/Native computed‑goto dispatch.

2. evaluateSuspensionPoint

High‑level meaning:

- Each suspension point creates a unique resume basic block bbResume.
- It registers that block as a resume point:
    - id = currentCodeContext.addResumePoint(bbResume)
- It defines a nested scope so that the suspensionPointId parameter maps to a blockAddress(bbResume).

That is the key trick: the “label” stored in the frame is literally a block address, not an integer.

3. Low‑level LLVM helper

File:
tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt

- fun indirectBr(address, destinations) wraps LLVMBuildIndirectBr and appends allowed destinations.
- Confirms Kotlin is using LLVM’s native indirect branch instruction, not a switch intended to become one.

Why this matters for you:

It proves your intuition was correct:

- Kotlin/Native’s label is “pointer‑to‑resume‑block”.
- Entry dispatch is indirectbr.
- Your macro switch approach is semantically right, but structurally one abstraction layer up.

If your plugin emits label‑address dispatch in C++ (GNU “labels as values”), Clang will lower to the same indirectbr form, matching Kotlin more
tightly.

———

## 3. Mapping Kotlin discoveries to your current C++ port

You already mirrored the essential runtime types and markers. Here’s the explicit mapping.

### 3.1 Suspend marker

Kotlin source:
tmp/kotlin/libraries/stdlib/src/kotlin/coroutines/intrinsics/Intrinsics.kt

- internal enum class CoroutineSingletons { COROUTINE_SUSPENDED, UNDECIDED, RESUMED }
- public val COROUTINE_SUSPENDED: Any get() = CoroutineSingletons.COROUTINE_SUSPENDED

Your port:
include/kotlinx/coroutines/intrinsics/Intrinsics.hpp

- enum class CoroutineSingletons { COROUTINE_SUSPENDED, UNDECIDED, RESUMED }
- get_COROUTINE_SUSPENDED() returns address of static marker
- is_coroutine_suspended(void*) is pointer equality

That’s faithful and already validated by tests/test_suspension_infrastructure_simple.cpp.

### 3.2 Continuation state machine base

Kotlin/Native runtime:
tmp/kotlin/kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt

- BaseContinuationImpl.resumeWith loop
- invokeSuspend virtual

Your port:
include/kotlinx/coroutines/ContinuationImpl.hpp

- Same resume loop structure
- Same suspension marker check
- Same intercepted/release_intercepted semantics placeholder

This is the substrate your DSL lowering should target.

### 3.3 Suspendable expression / resume dispatch

Kotlin/Native: uses block‑address label + indirectbr.

C++ port (canonical authoring surface):

- `src/kotlinx/coroutines/dsl/Suspend.hpp` provides `suspend(expr)` (a runtime identity helper used only as a marker).
- `src/kotlinx/coroutines/tools/clang_suspend_plugin/` rewrites `[[suspend]]` functions with `suspend(...)` points into a
  `ContinuationImpl`-derived state machine that propagates `COROUTINE_SUSPENDED`.

Representation note:

- Kotlin/Native stores a **block address** label (`void*`) for `indirectbr`.
- The current plugin emits an **int** `_label` with a `switch` dispatch (correct semantics, not identical IR shape).

———

## 4. Detailed compiler‑plugin + DSL plan

I’m going to present this as a multi‑phase engineering plan, explicitly mirroring Kotlin’s pipeline (intent → liveness/spill → lowering).

### 4.1 High‑level architecture

Inputs: C++ source files that use the coroutine DSL.

Outputs: Generated C++ (or direct AST rewrite) that:

- compiles with normal clang++/g++,
- lowers to Kotlin‑equivalent state machines,
- uses your runtime (ContinuationImpl, COROUTINE_SUSPENDED, Job, etc.),
- optionally uses computed‑goto label addresses for exact indirectbr.

Key invariants to preserve:

1. Suspend marker propagation:
   If a callee suspends, caller returns the same marker immediately.
2. Flat state machine:
   Nested suspendable expressions are flattened (Kotlin does not build nested machines).
3. Spill set correctness:
   Every local live across a suspend point survives by being stored in frame fields.
4. Kotlin evaluation order:
   Kotlin’s suspend lowering has specific argument slicing order (see ExpressionSlicer in NativeSuspendFunctionLowering.kt). You must either
   match it or define DSL restrictions.
5. RAII / exception semantics:
   Kotlin/Native uses try/finally lowering around suspend points; in C++ you must respect destructors and exception unwinding across points.

### 4.2 DSL surface design (phase 0 decisions)

You need an authoring syntax that is:

- C++‑parsable before plugin rewrite (so the compiler can build an AST).
- Unambiguous for discovering suspend points.

Canonical pattern (single API):

- Mark suspend functions with `[[suspend]]`.
- Mark suspension points with `suspend(expr)`.
- Do not introduce additional markers/macros in new code; keep authoring uniform so the plugin can stay mechanical.

### 4.3 Compiler choice: why Clang plugin first

Even though you can do GCC plugins, Clang aligns with Kotlin/Native’s LLVM world:

- You care about LLVM IR matching (indirectbr, blockaddress).
- Clang already supports coroutine lowering + CFG tools.
- You only need to support the toolchain you use in this repo.

GCC plugin later is fine for Linux builds once Clang path is stable.

### 4.4 Clang plugin plan (front‑end rewrite)

We follow Clang’s plugin structure you pasted, but tuned to your needs.

#### 4.4.1 Plugin skeleton

- Build a dylib (libKotlinxSuspendPlugin.dylib) that registers a PluginASTAction.

Core pieces:

1. ParseArgs
    - Recognize options like:
        - -state-machine=indirectbr|switch
        - -dump-spills
        - -dump-ir
        - -dsl-strictness=...
2. CreateASTConsumer
    - Instantiate visitor/rewriter.
3. getActionType
    - AddBeforeMainAction is ideal so you can still use -clear-ast-before-backend.
    - But if you modify AST, Clang recommends AddAfterMainAction.
      You can still do modification “late” if needed.

#### 4.4.2 Register custom attributes/pragmas

- ParsedAttrInfoRegister::Add<> for e.g. kotlinx::suspend and kotlinx::suspend_call.
- Optionally PragmaHandlerRegistry::Add<> for:
    - #pragma kotlinx_suspend_point
    - #pragma kotlinx_no_state_machine (tail optimization override)

This gives you stable syntax across compilers that support attributes.

#### 4.4.3 AST traversal: collect intent

Visitor responsibilities:

1. Detect suspend functions
    - Find FunctionDecl with [[kotlinx::suspend]]
    - Record its body, signature, and source range.
2. Detect suspend points
    - Within those functions, find:
        - calls annotated [[kotlinx::suspend_call]], or
        - calls to a known wrapper (kx::suspend_call(...)), or
        - (optionally) call expressions returning kx::Any and taking Continuation*.
    - Record:
        - call expression AST node
        - evaluation context (loop / try / argument list)
        - lexical ordering
3. Normalize nested suspendable expressions
   Kotlin flattens these.
    - You can do this by:
        - refusing nested suspend blocks in DSL (phase 1), or
        - collecting nested blocks and hoisting (phase 2+).

For phase 1, choose the strict route, then relax.

#### 4.4.4 Liveness/spill analysis (mirror Kotlin)

Kotlin uses CoroutinesVarSpillingLowering which relies on liveness computed earlier in the IR pipeline. In Clang you have two main options:

Option 1: Use Clang CFG + dataflow

- Build CFG of the suspend function body (Clang provides CFG::buildCFG).
- Treat each suspend point as a “cut.”
- Run backward liveness:
    - Variables whose definitions reach beyond a suspend point and are used after it are “live across.”
- Edge cases:
    - Variables in outer scopes.
    - Captured by lambdas (if DSL allows).
    - this / captures.

Option 2: Do not use C++20 coroutines

- This project does not use C++20 coroutines (`co_await`/`co_return`).
- Spill analysis must be driven by our own suspend DSL + lowering to keep Kotlin parity.

Given your goal of Kotlin parity, start with Option 1.

Deliverable of phase 1:
A set of VarDecl* live across each suspend point.

#### 4.4.5 Frame synthesis (generate spill fields)

For each suspend function:

- Synthesize a hidden frame struct:
    - _label : void* (for indirectbr mode) or int (switch mode fallback)
    - one field per spilled local, typed exactly like the local
    - optionally a Result<void*> / exception slot

This is the C++ analogue of Kotlin’s coroutine class fields.

You can generate it as:

- an injected struct Foo$Coroutine : ContinuationImpl { ... }, or
- an internal namespace kotlinx::coroutines::generated { ... } wrapper.

Given your runtime expects a continuation subclass, do the former.

#### 4.4.6 Lowering rewrite: emit Kotlin‑style state machine

This is the core transformation.

You will rewrite the suspend function into:

1. Wrapper function (original name)
    - Instantiate frame
    - Initialize completion/context
    - Call invoke_suspend once
    - Return value or marker
2. State machine method (invoke_suspend)
    - Entry dispatch:

      Indirectbr mode (exact Kotlin shape):

      if (this->_label == nullptr) goto start;
      goto *this->_label;
      start:
      ...

      This uses GNU labels‑as‑values. Clang will emit blockaddress + indirectbr.

      Switch mode (portable fallback):

      switch (this->_label_int) { case 0: goto start; case 1: goto resume1; ... }
3. At each suspend point

Rewrite:

auto tmp = baz(arg1, arg2, this);
if (tmp == COROUTINE_SUSPENDED) return COROUTINE_SUSPENDED;

into:

- Save spills (if any)
- Set label to resume address
- Call
- Propagate marker
- Resume label:
    - Restore spills
    - Continue

In indirectbr mode:

// before call
save_spills();
this->_label = &&resumeN;

void* tmp = baz(..., this);
if (is_coroutine_suspended(tmp)) return COROUTINE_SUSPENDED;

resumeN:
restore_spills();

That is a textual mirror of evaluateSuspensionPoint + evaluateSuspendableExpression.

#### 4.4.7 Tail‑suspend optimization (later phase)

Mirror Kotlin’s rule from NativeSuspendFunctionLowering.kt:

- If a suspend function has no non‑tail suspend calls, do not build state machine:
    - rewrite suspend tail calls into return delegatedCall(...) form.
- Detect tail positions in C++:
    - final statement return of suspend call.
    - last expression in lambda body, etc.

Phase 2+ feature; safe to skip initially.

———

## 5. GCC plugin plan (if/when you want it)

You provided the GCC plugin API details; here’s how it translates.

### 5.1 GCC event usage

- PLUGIN_ATTRIBUTES
    - Register kotlinx_suspend attribute.
- PLUGIN_PRE_GENERICIZE / PLUGIN_FINISH_PARSE_FUNCTION
    - Walk front‑end trees to collect:
        - suspend functions
        - suspend points
        - local uses

Important rule: treat front‑end trees as read‑only.

- PLUGIN_PASS_MANAGER_SETUP
    - Insert a custom GIMPLE pass after gimplification.

### 5.2 GIMPLE pass responsibilities

For each marked function:

1. Build GIMPLE CFG.
2. Find suspend points (calls annotated or matching signature).
3. Run liveness across points.
4. Synthesize spill vars and a label slot.
5. Rewrite CFG into a state machine:
    - entry switch on label int, or
    - if GCC supports computed goto lowering you can use it, but practically a switch is fine.

Your main “IR matching” target is LLVM via Clang; GCC path can be semantically equivalent even if not indirectbr.

———

## 6. Build integration (CMake + tooling)

Because this is project‑scoped, the build system can be opinionated.

### 6.1 Plugin build

Add a subdir, e.g. tools/clang_suspend_plugin/.

- CMake target to compile plugin against a specific Clang version installed on the Mac.
- Produce libKotlinxSuspendPlugin.dylib.

### 6.2 Compilation wrapper

For DSL‑using files:

- Use a wrapper script kx-clang++ that injects:

clang++ \
-Xclang -load -Xclang /path/libKotlinxSuspendPlugin.dylib \
-Xclang -plugin -Xclang kotlinx-suspend \
...

- CMake set_source_files_properties(... PROPERTIES COMPILE_FLAGS "...") to apply only to DSL sources.

### 6.3 Generated‑source option

If you prefer to keep compilation plain:

- Plugin runs in a “rewrite‑to‑file” mode:
    - reads original .cpp
    - emits generated/foo.kx.cpp
- CMake builds generated files instead.

This makes debugging easier (you can inspect the rewritten C++).

### 6.4 Verification step

Add a ctest or custom target:

1. Compile a Kotlin suspend snippet with kotlinc‑native to LLVM IR.
2. Compile equivalent C++ DSL snippet with your plugin to LLVM IR.
3. Diff key patterns:
    - presence of indirectbr
    - number of resume blocks
    - label storage type
    - spill fields count
    - marker propagation shape

Capture a Kotlin/Native-generated LLVM IR snapshot and turn it into an automated check (e.g., store it under `docs/cpp_port/ir_snapshots/`).

———

## 7. Phased roadmap with concrete milestones

### Phase 0: Scaffold

- Implement Clang plugin skeleton.
- Register attributes.
- Detect suspend functions + points; dump diagnostics only.
- No rewriting yet.

Exit criterion:
Build passes, diagnostics show correct discovery.

### Phase 1: Minimal lowering without spills

- Rewrite suspend functions into state machines.
- Require user to manually spill locals in DSL (like current macro world).
- Use switch mode first for portability.

Exit criterion:
Correct suspension semantics on simple tests; marker propagation identical.

### Phase 2: Liveness + automatic spills

- Implement CFG liveness.
- Synthesize spill fields.
- Insert save/restore around suspend points.

Exit criterion:
User no longer writes manual spill code; tests pass.

### Phase 3: Indirectbr exactness

- Switch label storage to void* + labels‑as‑values.
- Emit computed‑goto dispatch.
- Verify Clang emits blockaddress + indirectbr.

Exit criterion:
LLVM IR matches Kotlin/Native for representative cases.

### Phase 4: Tail‑suspend optimization

- Detect all‑tail suspend cases.
- Skip state machine generation.
- Maintain parity with Kotlin’s optimization.

Exit criterion:
Perf parity on tail‑call benchmarks.

### Phase 5: Advanced Kotlin parity

- Nested suspendable expression flattening.
- try/finally correctness across suspend.
- inline suspend lambdas handling.
- optional debug probes parity.

———

## 8. Risks and mitigation (explicit)

1. C++ evaluation order vs Kotlin slicing
    - Kotlin uses ExpressionSlicer to preserve evaluation order and avoid nested machines.
    - Mitigation:
        - Phase 1 restrict DSL to simple statements, no suspend in complex expressions.
        - Phase 5 implement slicing.
2. Destructors / RAII across suspend
    - Kotlin IR lowering handles finally blocks specially.
    - Mitigation:
        - Disallow suspend inside scopes with non‑trivial destructors initially.
        - Later, transform to explicit lifetime blocks.
3. Templates / generics
    - AST rewriting in templates can explode complexity.
    - Mitigation:
        - Require explicit instantiation or forbid suspend in templates early.
4. Multiple toolchains
    - Clang first may break GCC builds.
    - Mitigation:
        - Keep macro fallback; plugin only on Clang builds.

———

## 9. Why this plan is faithful to kotlinc

- State machine necessity check comes from NativeSuspendFunctionLowering.kt.
- Spilling algorithm mirrors CoroutinesVarSpillingLowering.kt + runtime intrinsics.
- Dispatch exactness mirrors IrToBitcode.kt:
    - blockAddress(resumeN) → labels‑as‑values
    - indirectBr → goto *label

So your C++ DSL, after rewrite, is essentially “Kotlin Native lowering, but in C++ source‑to‑source form.”

———

If you want, I can now:

- propose a concrete DSL syntax for your taste (attribute vs macro hybrid),
- draft the Phase‑0 Clang plugin scaffold in‑repo,
- and add a minimal IR‑diff test harness to lock parity early.
