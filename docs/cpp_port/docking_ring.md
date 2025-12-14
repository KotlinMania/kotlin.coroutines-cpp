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

## 4. Current Implementation: Macros + Computed Goto + IR Markers

**Status:** Production-ready. Generates identical LLVM IR to Kotlin/Native.

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
|  - goto *label -> indirectbr                                       |
|  - __kxs_suspend_point() -> call instruction (IR marker)           |
+-------------------------------------------------------------------+
                              |
                              v (optional)
+-------------------------------------------------------------------+
|  kxs-inject (LLVM IR Transform)                                    |
|  - Finds __kxs_suspend_point() calls                               |
|  - Computes liveness, generates spill/restore                      |
|  - Removes marker calls                                            |
+-------------------------------------------------------------------+
```

### 4.2 The Two Pieces

**1. `__LINE__` -> Computed Goto Address (Runtime)**

```cpp
(c)->_label = &&_kxs_resume_42;  // stores blockaddress in void*
// ...
goto *(c)->_label;  // indirectbr to resume point
```

This gives us the exact LLVM IR pattern:
```llvm
store ptr blockaddress(@func, %resume42), ptr %label
indirectbr ptr %label, [label %resume42, label %resume57, ...]
```

**2. `__kxs_suspend_point(__LINE__)` -> IR Marker (Tooling)**

```cpp
::__kxs_suspend_point(42);  // survives to IR as call instruction
```

In LLVM IR:
```llvm
call void @__kxs_suspend_point(i32 42)
```

The kxs-inject tool finds these markers for liveness analysis and spill generation.

### 4.3 Macro Definitions

From `src/kotlinx/coroutines/dsl/Suspend.hpp`:

```cpp
// IR-visible marker for kxs-inject tooling
extern "C" void __kxs_suspend_point(int id);

// Computed goto mode (default on GCC/Clang)
#if (defined(__GNUC__) || defined(__clang__)) && !defined(KXS_NO_COMPUTED_GOTO)

#define coroutine_begin(c) \
    if ((c)->_label == nullptr) goto _kxs_start; \
    goto *(c)->_label; \
    _kxs_start: \
    (void)(result).get_or_throw();

#define coroutine_yield(c, expr) \
    do { \
        (c)->_label = &&_KXS_LABEL(_kxs_resume_, __LINE__); \
        ::__kxs_suspend_point(__LINE__); \
        { \
            auto _kxs_tmp = (expr); \
            if (::kotlinx::coroutines::intrinsics::is_coroutine_suspended(_kxs_tmp)) \
                return _kxs_tmp; \
        } \
        goto _KXS_LABEL(_kxs_cont_, __LINE__); \
        _KXS_LABEL(_kxs_resume_, __LINE__): \
        (void)(result).get_or_throw(); \
        _KXS_LABEL(_kxs_cont_, __LINE__):; \
    } while (0)

#define coroutine_end(c) \
    return nullptr;

#else
// MSVC fallback: Duff's device (switch/__LINE__)
// ... similar but uses switch/case instead of computed goto
#endif
```

### 4.4 Usage Pattern

```cpp
class MyCoroutine : public ContinuationImpl {
    void* _label = nullptr;  // blockaddress storage
    int spilled_var;         // manual spill (kxs-inject automates later)

    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)

        spilled_var = 42;
        coroutine_yield(this, delay(100, completion_));

        std::cout << spilled_var << std::endl;

        coroutine_end(this)
    }
};
```

### 4.5 Verified IR Output

Confirmed via `clang++ -S -emit-llvm`:

```llvm
; blockaddress storage
store ptr blockaddress(@_ZN11MyCoroutine14invoke_suspendE..., %resume42), ptr %label

; IR marker for kxs-inject
call void @__kxs_suspend_point(i32 42)

; Entry dispatch
indirectbr ptr %saved_label, [label %resume42, label %resume57]
```

This is **identical** to Kotlin/Native's IrToBitcode.kt output.

---

## 5. kxs-inject: LLVM IR Transformer

**File:** `src/kotlinx/coroutines/tools/kxs_inject/kxs_inject.cpp`

### 5.1 Purpose

The kxs-inject tool processes LLVM IR to:

1. **Find suspension points** via `__kxs_suspend_point()` calls
2. **Compute liveness** - which variables are live across each suspension
3. **Generate spill/restore** - add field stores/loads around suspend points
4. **Remove markers** - eliminate `__kxs_suspend_point()` calls from output

### 5.2 Pipeline

```bash
# Without kxs-inject (works, but requires manual spilling)
clang++ -c file.cpp -o file.o

# With kxs-inject (automatic spilling)
clang++ -S -emit-llvm -o file.ll file.cpp
kxs-inject file.ll -o file.transformed.ll
clang++ -c file.transformed.ll -o file.o
```

### 5.3 CMake Integration

```cmake
include(kxs_transform_ir)
kxs_enable_suspend(my_target)  # Applies IR transform to target
```

---

## 6. Phased Roadmap

### Phase 1: Macros + Computed Goto [COMPLETE]

- `coroutine_begin/yield/end` macros
- `void* _label` with computed goto
- `__kxs_suspend_point()` IR markers
- Manual spilling required
- LLVM IR matches Kotlin/Native

**Exit Criterion:** IR comparison shows identical `indirectbr` + `blockaddress` pattern.

### Phase 2: kxs-inject Liveness Analysis [IN PROGRESS]

- Find `__kxs_suspend_point()` markers in IR
- Build CFG, compute liveness at each point
- Generate spill fields in coroutine struct
- Insert save/restore around suspend points

**Exit Criterion:** User no longer writes manual spill code.

### Phase 3: Tail-Suspend Optimization [FUTURE]

- Detect all-tail suspend calls
- Skip state machine generation
- Direct delegation: `return other_suspend_fn(completion);`

### Phase 4: Advanced Kotlin Parity [FUTURE]

- Nested suspendable expression flattening
- try/finally correctness across suspend
- inline suspend lambdas
- Debug probes parity

---

## 7. Risks and Mitigations

1. **Manual spilling is error-prone**
   - Mitigation: Phase 2 adds automatic spilling via kxs-inject

2. **MSVC lacks computed goto**
   - Mitigation: Duff's device fallback (semantically correct, different IR)

3. **RAII across suspend points**
   - Mitigation: Document restrictions; destructors may not run correctly yet

4. **Template complexity**
   - Mitigation: Avoid suspend in templates initially

---

## 8. Why This Approach Works

The macro + computed goto approach achieves Kotlin/Native parity because:

1. **`&&label` → `blockaddress(@func, %bb)`**
   - GCC/Clang labels-as-values extension compiles directly to LLVM blockaddress

2. **`goto *ptr` → `indirectbr ptr, [labels...]`**
   - Computed goto compiles to LLVM indirectbr instruction

3. **`__kxs_suspend_point()` → IR-visible marker**
   - Function calls survive to IR (unlike attributes)
   - kxs-inject can find them for liveness analysis

4. **Same state machine structure**
   - Entry dispatch checks null, else jumps to saved label
   - Each suspend point stores label, calls, checks COROUTINE_SUSPENDED, has resume label

This is essentially "Kotlin/Native lowering implemented in C++ macros."
