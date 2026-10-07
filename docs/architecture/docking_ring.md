## 0. Context and goal restatement

The port must work as a standalone C++ coroutine library with ordinary C++ code,
without requiring a Kotlin installation or runtime. It must also be compatible
with Kotlin/Native through the docking ring described here. Both requirements
govern the compiler pipeline, object lifetimes and executable acceptance.

You’re porting Kotlin coroutines to C++ to form a “docking ring” between:

- A standalone C++ coroutine library and compiler pipeline usable by ordinary C++ applications
- Kotlin/Native runtime (GC + coroutines state machine lowering + continuation API)
- C++ (as a high‑performance / ABI‑interop target, including MLX / GPU ABIs)
- Future freethreaded Python embedding use cases

Your current method:

- mechanical transliteration of kotlinx.coroutines and Kotlin coroutine runtime pieces,
- plus direct study of kotlinc/Kotlin‑Native suspend lowering to reproduce the state machine exactly,
- plus early C++ prototypes that have since been replaced by the suspend DSL + Clang plugin approach.

You now want to go a step further:

- a C++‑side DSL that lets you write Kotlin‑style suspend logic in “normal looking C++,”
- with a build‑time Clang compiler pipeline and mandatory LLVM injection that rewrites that DSL into a Kotlin‑equivalent state machine,
- scoped to this repo only (so you accept the maintenance burden).

The governing requirement is Kotlin-like authoring and direct Kotlin↔C++
handoffs through the actual Kotlin state-machine contracts. Native IR injection
is required; equivalent-looking C++ dispatch or marker cleanup does not establish
the docking ring.

The key technical requirement for the compiler pipeline:

- Generated IR should match kotlinc/K/N patterns closely enough that semantics (and perf) are the same: spilled locals, label/resume logic,
  suspend marker propagation, and Kotlin/Native's required block-address `indirectbr` form.

———

## Standalone C++ use and Kotlin/Native compatibility

Sydney's requirement on 2026-10-06: this is a standalone C++ coroutine port that
must also be compatible with Kotlin/Native. Normal C++ functions, types and MLX C++
code must remain usable with the port. Standalone C++ applications must build and
run without an installed Kotlin compiler, linked Kotlin/Native runtime, or JVM.
They use the translated C++ coroutine implementation and the CMake/Clang plugins.
Upstream Kotlin is the source of the translation and compatibility contract; it is
not a required application runtime for ordinary C++ use.

Standalone independence applies to building the C++ library and its CMake/Clang
plugins as well as to application execution. The translated compiler dependencies
must build as C++ without invoking kotlinc/konanc or linking Kotlin/Native runtime
libraries. Pinned Kotlin source remains the development reference and provenance
source. Kotlin/Native compatibility is a required capability; its runtime becomes
a dependency only for builds that explicitly enable the Native interop boundary.

Normal C++ use includes calling existing nonsuspending functions, using ordinary
classes and standard-library types, and retaining C++ objects and MLX handles in
coroutine locals. Those functions do not need Kotlin annotations or translation.
The frontend lowers coroutine bodies and suspension operations while preserving
the surrounding C++ code, types and calls. Authors must not manually save locals
or construct resume dispatch to make their C++ code usable.

Preserve each C++ object's actual ownership and destruction rules across repeated
suspension, completion, failure and cancellation. A borrowed pointer or reference
does not become owned merely because a coroutine retains it. Only actual Kotlin
GC objects crossing the Native boundary require Kotlin roots; ordinary C++ objects
must not acquire a Kotlin-object representation or runtime dependency.

Translated Kotlin compiler classes, including the compiler's `Any` contract,
are internal implementation dependencies. Ordinary C++ application classes must
not be required to inherit from them, adopt Kotlin object storage, or replace
their existing ownership rules. Build-time coroutine lowering must retain the
actual C++ declarations and values in the coroutine frame. Enabling Native
interoperability does not change that requirement for C++ objects.

Kotlin/Native interoperability is an explicitly linked boundary when requested.
That boundary requires the real matching Native runtime, continuation/frame/result
contracts, and roots for actual Kotlin GC objects. Ordinary C++ values, RAII objects
and MLX handles retain their actual C++ lifetimes; using the coroutine port does
not require converting them to Kotlin objects. This distinction does not authorize
an alternate coroutine state machine, substitute runtime or fallback.

Acceptance requires both:

1. A standalone CMake/Clang C++ executable that uses ordinary C++ functions and
   types with the coroutine authoring surface, retains C++ locals/resources across
   suspension, performs real MLX C++ GPU work, and resumes correctly. Verify its
   build and linked dependencies do not require Kotlin tools or runtime libraries.
   Include calls to existing nonsuspending C++ functions and use of ordinary
   classes and standard-library types. Verify resource identity and destruction
   across repeated suspension, completion, failure and cancellation. Check the
   complete build and transitive link dependencies with Native interop disabled.
2. The Kotlin/Native/C++ docking-ring demonstration through the actual Native
   coroutine state machine and direct unsafe MLX bindings, with both handoff
   directions, results, failure/cancellation, resource identity and cleanup.

The C++ implementation must satisfy its own executable contract even when no
Kotlin program participates. Native interoperability tests are separate evidence.
Existing array fixtures prove neither complete standalone authoring/MLX integration
nor the complete shared-state-machine boundary.

## Root CMake authoring integration checkpoint: 2026-10-07

When frontend authoring is enabled, the root registers both compiler targets before
applying kxs_enable_suspend_dsl to the core and coroutine executables. Requested
Clang/LLVM packages and shared compiler registries are required. Configuration
cannot silently skip requested authoring. Existing Continuation-ABI callers may
explicitly disable frontend authoring; requesting the authoring function still
requires the actual frontend and mandatory LLVM stage.

A separate Native-OFF Release CMake application builds the port and both plugins
and executes ordinary C++ captured-object/immediate/resumed/error behavior with only
OS runtime libraries. This is bounded pipeline evidence; real IR identity/scopes,
direct Native shared-state-machine handoffs and both MLX GPU acceptance scenarios
remain required. The existing Native regression uses actual Kotlin continuations
through StableRef/callback adapters, which do not establish the direct shared-frame
requirement. See [the dated source/build checkpoint](../audits/IR_IDENTITY_DEPENDENCIES.md#frontend-root-build-and-standalone-authoring-checkpoint-2026-10-07).

## Governing deliverable: Kotlin-style authoring and unsafe MLX C++ bindings

Sydney's clarification on 2026-10-06: the docking ring is the deliverable. C++ must
expose the Kotlin-style coroutine authoring surface, including suspend and yield
and the translated coroutine operations, through the CMake-enabled Clang frontend
and mandatory LLVM injection. Lowering must follow actual Kotlin/Native IR values,
state-machine construction, saved live values and continuation/result behavior.
Generic C++ coroutine behavior or matching dispatch shape alone does not establish
that requirement.

Kotlin/Native must also be able to call MLX C++ and GPU-driver operations through
explicit unsafe native bindings. Preserve the actual binding's raw handles and
caller-controlled lifetimes; document the actual ownership contract rather than
inventing an automatic safe wrapper or changing the resource lifetime policy.
Native GC roots remain required for actual GC-managed Kotlin objects. Resource
handles are not automatically Kotlin GC objects.

The end-to-end acceptance scenario is a CMake-built C++ coroutine using the authoring
surface, joined to an actual Kotlin/Native coroutine chain, calling real MLX C++ GPU
work through the native binding, suspending and resuming with live state retained,
and returning the correct result or source failure/cancellation outcome. Verify
which GPU work executed and the continuation/frame/resource identities at the
boundary. The matching pinned runtime and bare-metal target acceptance remain
required. No MLX/GPU end-to-end execution has been established by the integer-array
demo.

A "Native map" in prior progress reports means a lookup table from Kotlin's Native
standard library. It is an internal source dependency, not the coroutine authoring
surface, GPU integration, or docking-ring delivery. Array/table tests certify only
those dependency operations. Each further dependency must be tied to its named
consuming Kotlin compiler/runtime function and the production plugin path; report
its effect on the end-to-end deliverable separately from its narrow test evidence.
Source-faithful dependencies and project-wide ast_distance --deep criteria remain
mandatory. Do not replace missing source behavior with a stub to reach a demo.

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

The DSL/plugin must translate this optimization rule. Retaining an unnecessary
frame changes the source lowering and is a measured mismatch. C++ lifetime
requirements need explicit evidence for any deliberate departure.

### 2.2 Phase B: Build the coroutine class + wrap body in IrSuspendableExpression

Still in NativeSuspendFunctionLowering.kt.

Key structure (high level):

- Kotlin/Native lowers a suspend function into:
    1. a coroutine implementation class (inherits ContinuationImpl or RestrictedContinuationImpl, both derived from BaseContinuationImpl),
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
  suspension/SUSPEND_IMPLEMENTATION.md).
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
- A switch state machine does not meet the required native address-dispatch contract.

If your plugin emits label‑address dispatch in C++ (GNU “labels as values”), Clang will lower to the same indirectbr form, matching Kotlin more
tightly.

———

## 3. Mapping Kotlin discoveries to your current C++ port

The following paths identify the runtime types and markers to compare. Their
current porting criteria come from the project-wide `ast_distance --deep`
reports and executable evidence in `docs/audits/project-wide/`.

### 3.1 Suspend marker

Kotlin source:
tmp/kotlin/libraries/stdlib/src/kotlin/coroutines/intrinsics/Intrinsics.kt

- internal enum class CoroutineSingletons { COROUTINE_SUSPENDED, UNDECIDED, RESUMED }
- public val COROUTINE_SUSPENDED: Any get() = CoroutineSingletons.COROUTINE_SUSPENDED

Your port:
src/kotlinx/coroutines/intrinsics/Intrinsics.hpp

- enum class CoroutineSingletons { COROUTINE_SUSPENDED, UNDECIDED, RESUMED }
- get_COROUTINE_SUSPENDED() returns address of static marker
- is_coroutine_suspended(void*) is pointer equality

Pointer identity is the C++ suspension-marker contract. Source parity and
Kotlin/Native marker interoperability require separate evidence; a matching enum
does not certify them.

### 3.2 Continuation state machine base

Kotlin/Native runtime:
tmp/kotlin/kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt

- BaseContinuationImpl.resumeWith loop
- invokeSuspend virtual

Your port:
src/kotlinx/coroutines/ContinuationImpl.hpp and ContinuationImpl.cpp

- `resume_with` requires a completion before invoking the frame, as Kotlin's `completion!!` requires.
- `ContinuationImpl::get_context` requires its stored context, as Kotlin's `_context!!` requires.
- `RestrictedContinuationImpl` requires any present completion's context to be the canonical empty context.
- `intercepted` and `release_intercepted` have source-derived implementations; remaining runtime mismatches are audit work, not acceptable placeholders.

NativeSuspendFunctionLowering.kt:93-98 selects the base using the restricted
extension-receiver predicate or the restricted-invoke flag. The Clang frontend
now uses that branch for frame inheritance and construction. Its class and
parameter attributes represent Kotlin's annotation and explicit IR receiver
role. Ordinary arguments and dispatch receivers do not become extension
receivers because their class is restricted. Lambda lowering now records the
restriction before receiver-role flattening and sets the copied invoke flag;
the regression verifies it survives AST import. Named-reference wrapper,
property-reference and reflection lowering remain required. Closure captures
currently retain C++ borrowed-field identity; Kotlin GC ownership across arbitrary
temporary closure lifetimes is not established by this implementation.

Within a replaced enclosing suspend frame, nongeneric lambda locals now lower
to concrete callable classes with capture fields, ordered constructor arguments
and the current transformed invoke body. This follows the construction and
body-remapping responsibilities in AbstractFunctionReferenceLowering.kt:135-295.
The enclosing frame owns that callable; its completion chain retains it while
the nested invoke is suspended. CMake execution covers capture snapshots,
borrowed capture identity, move-only release, captured owners and failures at
both suspension stages. Indexed private capture fields now use original declaration
identity for invoke remapping; ordinary parameter/local shadowing remains intact.
Function and parameter annotations, including the restricted-invoke flag and
flattened receiver origins, survive callable construction. A copied receiver
constructs an owned bound-value field; invoke receiver expressions refer to that
field's address with the method's constness. Constant complete array fields
construct from their elements, retaining dimensions and native copy/destruction
behavior; array references retain the original storage. Generic captures,
reflection parity, full declaration lifting and general temporary-closure GC
ownership remain required. Existing native address dispatch and Kotlin-emitted
handoff tests do not establish full frame-layout interoperability.

This is the substrate your DSL lowering should target.

### 3.3 Suspendable expression / resume dispatch

Kotlin/Native: uses block‑address label + indirectbr.

C++ port (canonical authoring surface):

- `src/kotlinx/coroutines/dsl/Suspend.hpp` provides `suspend(expr)` (a runtime identity helper used only as a marker).
- `src/kotlinx/coroutines/tools/clang_suspend_plugin/` rewrites `[[suspend]]` functions with `suspend(...)` points into a
  state machine with the selected continuation base that propagates `COROUTINE_SUSPENDED`.

Representation note:

- Kotlin/Native stores a **block address** label (`void*`) for `indirectbr`.
- The experimental plugin supplies LLVM injection regions. Switch generation has been removed; complete state/result/lifetime parity still requires compiler integration.

———

## 4. Current Implementation: Macros + Computed Goto + IR Markers

**Status:** Clang authoring macros supply frame-field and resume-block identities;
the mandatory LLVM injector constructs Kotlin/Native address dispatch. The
frontend now automatically constructs retained frames for the tested free-function and member-function
DSL entries during ordinary compilation. Actual Kotlin-generated continuations
are exercised through C interop in a generated C++ caller. Broader frontend and
ABI/GC parity remain unfinished. See [the IR specification](../suspension/IR_SUSPEND_LOWERING_SPEC.md)
for the verified handoff contracts and injection boundary.

### 4.1 Architecture

```
+-------------------------------------------------------------------+
|  Source Code (Macros)                                              |
|  coroutine_begin(this) / coroutine_yield(this, expr) / ...        |
+-------------------------------------------------------------------+
                              |
                              v
+-------------------------------------------------------------------+
|  Clang Compilation                                                 |
|  - Computed goto (&&label) -> blockaddress                         |
|  - begin marker -> persistent frame-field identity                                       |
|  - point marker -> actual function-local block address           |
+-------------------------------------------------------------------+
                              |
                              v (required)
+-------------------------------------------------------------------+
|  KotlinxCoroutinePass (inside Clang, before optimization)          |
|  - Finds __kxs_suspend_point() calls                               |
|  - Injects saved-address stores, entry branch and indirectbr     |
|  - Consumes markers; verifies IR and result/spill accesses                                            |
+-------------------------------------------------------------------+
```

### 4.2 Frontend contracts and LLVM values

`__kxs_coroutine_begin(&(c)->_label)` supplies the persistent label field's actual
address. `__kxs_suspend_point(id, &(c)->_label, &&resume)` supplies a function-local LLVM block
address. The marker ID does not replace the address, field, suspension sentinel,
resumed Result or live-state storage.

The injector constructs the saved-label load, null/start conditional branch,
resume `indirectbr` and destination list, and each resume-address store. It never
allocates a new stack label slot or initializes the label on function re-entry.
Initial null belongs to frame construction. The frontend prologue stays before
the dispatch so argument and Result storage remain valid on resumed entry.

### 4.3 Authoring macros

`src/kotlinx/coroutines/dsl/Suspend.hpp` declares compiler markers without runtime
implementations. The begin macro supplies the field, and yield macros supply
resume addresses plus immediate/suspended/resumed result regions. Label stores
and resume dispatch are injected at LLVM level. Missing injection must fail to
link. Live values currently require retained frame storage; automatic compiler
spill generation remains unfinished.

### 4.4 Usage Pattern

```cpp
class MyCoroutine : public ContinuationImpl {
    void* _label = nullptr;  // blockaddress storage
    int spilled_var;         // retained live state; injection does not yet infer spills

    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)

        spilled_var = 42;
        coroutine_yield(this, delay(100, this));

        std::cout << spilled_var << std::endl;

        coroutine_end(this)
    }
};
```

### 4.5 Verified IR Output

The injector produces these LLVM instructions from frontend marker contracts:

```llvm
; blockaddress storage
store ptr blockaddress(@_ZN11MyCoroutine14invoke_suspendE..., %resume42), ptr %label

; IR marker for kxs-inject
call void @__kxs_suspend_point(i32 42, ptr %label, ptr blockaddress(@invoke_suspend, %resume42))

; Entry dispatch
indirectbr ptr %saved_label, [label %resume42, label %resume57]
```

This matches Kotlin/Native's block-address dispatch shape. Frame layout, ownership, GC and full semantics need separate verification.

---

## 5. KotlinxCoroutinePass: in-compiler LLVM lowering

**Files:** `src/kotlinx/coroutines/tools/kxs_inject/KotlinxCoroutinePass.cpp` and
`CoroutineInjection.cpp`

### 5.1 Purpose

The module pass verifies Clang's in-memory module, validates frontend frame/resume
contracts, and constructs Kotlin/Native label stores and indirect dispatch.
It diagnoses stack/null label fields, missing entries, foreign destinations,
unsupported marker uses and invalid transformed SSA. It preserves the frontend's
immediate/suspended/resumed Result and live-state field accesses.

Clang registers the required pass at pipeline start, including unoptimized
compilation, before optimization and sanitizer instrumentation. The module never
crosses a text serialization boundary. The obsolete Python compile launcher and
cross-version floating-point/lifetime writer adaptations have been removed.
The standalone `kxs-inject` diagnostic driver uses the same lowering engine and
LLVM's native writer; it is not the production build path.

The LLVM package must come from the selected compiler toolchain and export its
shared `LLVM` target. The injector no longer selects a vendored LLVM build or
component-library substitute. Resume-point collection uses the compiled
`SuspendableExpressionScope` translation in `kotlinc_native_ref/IrToBitcode_coroutines.cpp`.
That file also generates real normal/resume merge blocks and value phis,
retaining Kotlin’s unnamed dispatch comparisons and merge instructions. Its
continuation-block helper invokes the code-generation callback after phi
construction; the exception-handler caller and location/type lowering remain
untranslated. A generated two-point expression exercises ordered resume
destinations and successive stored block addresses. The
earlier uncompiled pseudocode and local void-pointer aliases have been replaced.
Full Kotlin IR evaluation and scope resolution remain separate missing compiler
dependencies, rather than being claimed by these LLVM operand helpers.

Mandatory injection and the expression helpers share the translated
`FunctionGenerationContext` branch and instruction-position methods from
`kotlinc_native_ref/CodeGenerator.cpp`. These emit LLVM conditional/indirect
branches and preserve Kotlin's after-terminator state, including creation of an
unreachable insertion block when requested. This is compiler generation state,
not a runtime suspension substitute. Complete native context initialization and
source-location maps remain missing dependencies. Each generation context now
binds one compiler-owned LLVM function definition, matching Kotlin's `function`
property. `block_address` uses that definition; resume addresses do not derive
their owning function from the current insertion point. Expression and
after-terminator block creation now follow Kotlin's insertion/move algorithm,
placing a new block immediately after the context's current block. Compiled
`if_then_else` and `if_then` helpers translate Kotlin's phi and terminator-aware
conditional generation; general IR expression and exception scopes remain
untranslated.

### 5.2 Pipeline

```bash
clang++ -fpass-plugin=build/lib/KotlinxCoroutinePass.so -c file.cpp -o file.o
```

Use the Clang belonging to the LLVM development package used to build the plugin.
The file suffix follows the platform's CMake module suffix. CMake adds the flag
and tracks plugin dependencies; it preserves ordinary compile commands and
existing launchers. In-tree and installed-package integrations check versions.

### 5.3 CMake Integration

```cmake
include(KotlinxCoroutines)
kxs_enable_suspend(my_target)  # Mandatory native LLVM injection
```

---

## 6. Phased Roadmap

### Phase 1: Frontend markers + LLVM address injection

- `coroutine_begin/yield/end` macros
- Persistent `void* _label` field supplied to the injector
- `__kxs_suspend_point()` IR markers
- Manual spilling required
- LLVM dispatch constructed against Kotlin/Native lowering contracts

**Exit Criterion:** IR comparison shows identical `indirectbr` + `blockaddress` pattern.

### Phase 2: Compiler-Driven Automatic Spilling

- Find `__kxs_suspend_point()` markers in IR
- Build CFG, compute liveness at each point
- Generate spill fields in coroutine struct
- Insert save/restore around suspend points

**Exit Criterion:** User no longer writes manual spill code.

### Phase 3: Tail-Suspend Optimization

- Detect all-tail suspend calls
- Skip state machine generation
- Direct delegation: `return other_suspend_fn(completion);`

### Phase 4: Advanced Kotlin Parity

- Nested suspendable expression flattening
- try/finally correctness across suspend
- inline suspend lambdas
- Debug probes parity

These phases state required behavior. Current completion claims must be derived
from refreshed project-wide deep reports and tests covering that behavior; the
phase headings are not a static implementation inventory.

---

## 7. Risks and Mitigations

1. **Manual spilling is error-prone**
   - Mitigation: verify manual frame storage now; Phase 2 requires compiler-driven spill lowering

2. **Compiler/LLVM compatibility**
   - Require supported Clang and compatible LLVM parsing/writing/codegen.
   - GCC/MSVC and Duff's device are outside this pipeline; no runtime fallback.

3. **RAII across suspend points**
   - Verify construction, retained lifetimes and destruction across immediate,
     suspended and exceptional paths; document the exact covered cases.

4. **Template complexity**
   - Preserve source type and parameter identity through template instantiation;
     measure and execute the concrete compiler cases rather than avoiding them.

---

## 8. Evidence required for the docking ring

Injected `blockaddress` and `indirectbr` establish address dispatch, not complete
Kotlin/Native interoperability. Actual Kotlin↔C++ state-machine handoffs must
verify frame/result representation, continuation routing, retained live state,
exception/cancellation propagation, lifetime and GC contracts. Current C++
injection and runtime regressions do not establish those cross-language claims.

## 9. State and result handoffs verified in October 2026

The saved address, spilled state, runtime `COROUTINE_SUSPENDED`, resumed
`Result`, and the Safe/CancellableContinuation atomic decision are distinct.
The marker's `__LINE__` ID replaces none of them. The suspended operation must
resume the current frame; only completed outcomes proceed to its parent.
`coroutine_yield_value` also needs separate immediate-value and resumed-value
paths, with failure checking before subsequent work.

The in-compiler plugin integration preserves the actual target/source/toolchain flags,
unique object paths and dependency graph. `test_ir_pipeline` executes native-injected builds, including optimized AddressSanitizer execution, and checks
multiple frames, repeated suspension, resumed failures and header and plugin rebuilds. Debug metadata remains in Clang, avoiding the
previous cross-version reader loss; `test_kxs_compiler_pass` verifies it.
See [the handoff review](../audits/IR_HANDOFF_REVIEW.md) for source/version
evidence, the defects repaired, and remaining validation limits.

`kxs_enable_suspend_dsl` enables the AST frontend together with LLVM injection.
The frontend imports a retained frame from an in-memory parser using the same
compiler invocation and emits it before code generation. Ordinary compilation
does not consume a sidecar. The actual Kotlin regression retains Kotlin's emitted
resume IR, checks native address dispatch, resumes the same Kotlin continuation
from C++ worker threads, and returns through the original generated C++ frame.
Stable-reference handles and C++ result boxes have explicit release policies.
See the handoff review for the exact tested boundary and remaining compiler cases.
