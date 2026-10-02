# IR Suspend Lowering Specification

This document specifies how coroutine suspend points are lowered to LLVM IR in `kotlin.coroutines-cpp`, matching Kotlin/Native's coroutine implementation pattern.

---

## 1. Theoretical Foundation: Coroutine Marker Paradigms

In C and C++, stackless coroutines are typically implemented using state markers that manipulate execution flow across function invocations. There are two primary paradigms:

### Paradigm A: Switch-Based Line Markers (Simon Tatham / Duff's Device)
Uses integer state variables and a top-level `switch` statement:
```c
#define CO_BEGIN(state)        switch (state) { case 0:
#define CO_YIELD(state, value) do { state = __LINE__; return (value); case __LINE__:; } while (0)
#define CO_END                 }
```
- **Mechanism**: The preprocessor macro `__LINE__` acts as a discrete integer marker. On re-entry, `switch (state)` branches back to `case __LINE__:`.
- **Limitation**: Generates binary jump tables or linear search cascades in LLVM IR; does not match Kotlin/Native's continuation ABI; cannot easily interoperate with external runtime frames.

### Paradigm B: Assembly-Based Label Pointers (`&&label` / Computed Goto)
Uses GCC/Clang's labels-as-values extension (`&&label`) to store exact 64-bit code address markers:
```cpp
#define coroutine_begin(c)     if ((c)->_label == nullptr) goto _kxs_start; goto *(c)->_label; _kxs_start:
#define coroutine_yield(c, e)  do { (c)->_label = &&_kxs_resume_##__LINE__; ... return COROUTINE_SUSPENDED; _kxs_resume_##__LINE__:; } while (0)
```
- **Mechanism**: Stores the actual address of the resume block (`void* _label`) into the coroutine frame.
- **LLVM IR Lowering**: Apple Clang lowers `goto *(c)->_label;` directly to `indirectbr ptr %saved_label, [label %resume0, ...]` and `&&label` to `blockaddress(@function, %label)`.
- **Parity**: This **identically mirrors Kotlin/Native's compiler lowering** (`IrToBitcode.kt`).

---

## 2. Upstream Kotlin/Native Lowering Reference

In the Kotlin/Native compiler, coroutines are lowered through two coordinated phases:

1. **Variable Spilling (`CoroutinesVarSpillingLowering.kt:68-105`)**:
   Local variables that are live across suspension points cannot remain stack `alloca`s because resumption jumps directly into internal basic blocks, bypassing entry-block stack allocations and violating LLVM SSA dominance. Kotlin/Native lowers these variables to fields on `thisReceiver` (`IrField` accessors).
2. **LLVM Codegen (`IrToBitcode.kt:2289-2335`)**:
   - `evaluateSuspendableExpression()` emits entry dispatch:
     ```llvm
     %is_first = icmp eq ptr %saved_label, null
     br i1 %is_first, label %start, label %dispatch

     dispatch:
       indirectbr ptr %saved_label, [label %resume0, label %resume1, ...]
     ```
   - `evaluateSuspensionPoint()` stores the resume `blockaddress` into the coroutine struct before returning `COROUTINE_SUSPENDED`.

---

## 3. C++ Source Pattern in `kotlinx.coroutines-cpp`

### Macros in `src/kotlinx/coroutines/dsl/Suspend.hpp`

```cpp
class MyCoroutine : public ContinuationImpl {
public:
    void* _label = nullptr;  // Holds resume blockaddress (NativePtr)

    // Variables crossing suspend points are spilled to member fields:
    int iteration = 0;
    int accumulator = 0;

    explicit MyCoroutine(std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)) {}

    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)

        accumulator = 10;
        coroutine_yield(this, delay(100, completion_));

        accumulator += 20;
        coroutine_yield(this, yield(completion_));

        coroutine_end(this)
    }
};
```

### The `__LINE__` Address Marker Pattern

`Suspend.hpp` combines the unique identification power of `__LINE__` with computed gotos:
```cpp
#define _KXS_CONCAT(a, b) a##b
#define _KXS_LABEL(prefix, line) _KXS_CONCAT(prefix, line)

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
```

### IR Marker Function: `__kxs_suspend_point`
```cpp
extern "C" void __kxs_suspend_point(int id);
```
- Emitted at every suspension point via `::__kxs_suspend_point(__LINE__)`.
- Serves as an explicit, IR-visible hook that survives compilation.
- In runtime builds, defined as a no-op in `src/kotlinx/coroutines/kxs_suspend_point.cpp`.
- In IR-transformed builds, rewritten or stripped by `kxs_transform_ir.cmake`.

---

## 4. LLVM IR Transformation Details & Multi-Function Architecture

### The Multi-Function Challenge
A single translation unit (e.g. `test_suspension_core.cpp`) generates an LLVM IR file containing hundreds of functions:
- Static initialization (`@__cxx_global_var_init`)
- Standard library templates (`std::vector`, `std::shared_ptr`)
- Multiple distinct coroutine classes (`SimpleYieldCoroutine`, `LoopCoroutine`, etc.)

Early implementations of `kxs_transform_ir.cmake` performed a global regex search for `define ... @func` and injected dispatch into the first function encountered (`@__cxx_global_var_init`). This caused fatal LLVM assembler errors:
1. `use of undefined value '%kxs.resume.0'` (labels from one coroutine referenced in another function).
2. `Instruction does not dominate all uses!` (injected resume jumps bypassing entry-block `alloca`s).

### The Resolved Lowering Pipeline (`cmake/Modules/kxs_transform_ir.cmake`)

`kxs_transform_ir.cmake` executes the following algorithm:

```
Input: foo.ll (LLVM IR text)
  │
  ├── 1. Find all 'call void @__kxs_suspend_point(i32 N)'
  │      If 0 found → pass through unchanged.
  │
  ├── 2. Escape semicolons ('\;' ) to prevent CMake list corruption on LLVM comments.
  │
  ├── 3. Tokenize by '(^|\n)define [^{]+{' to process each function independently.
  │
  └── 4. For each function chunk containing '__kxs_suspend_point':
         │
         ├── Case A: Function already contains 'indirectbr' or 'switch'
         │   (Emitted natively by Clang from coroutine_begin / computed goto)
         │   → Strip 'call void @__kxs_suspend_point(i32 N)' marker calls.
         │   → Dispatch table and SSA dominance are already 100% correct!
         │
         └── Case B: Function lacks computed goto dispatch
             → Strip markers and log diagnostic note.
             (Full indirectbr injection requires member-field variable spilling).
  │
  ├── 5. Strip unused 'declare void @__kxs_suspend_point' declaration.
  │
  └── Output: foo.kxs.ll (clean, valid LLVM IR)
```

---

## 5. Verification & Toolchain Pipeline

To verify the full IR transformation pipeline:

```bash
# 1. Compile C++ source to LLVM IR bitcode text
clang++ -S -emit-llvm -std=c++20 -Wno-gnu-label-as-value \
    -I src -I src/kotlinx/coroutines \
    src/tests/src/test_suspension_core.cpp -o /tmp/core.ll

# 2. Transform IR via standalone CMake script
cmake -DINPUT_FILE=/tmp/core.ll -DOUTPUT_FILE=/tmp/core.kxs.ll -P cmake/Modules/kxs_transform_ir.cmake

# 3. Assemble transformed IR to Mach-O object file
clang++ -c /tmp/core.kxs.ll -o /tmp/core.kxs.o

# 4. Link and execute under AddressSanitizer
clang++ -fsanitize=address /tmp/core.kxs.o \
    -Lbuild/lib -lkotlinx-coroutines-core -lpthread \
    -o /tmp/core_kxs_bin && /tmp/core_kxs_bin
```

Expected result:
```
-- [KXS] Found 5 suspend point(s) in /tmp/core.ll
-- [KXS] Processing function @_ZN20SimpleYieldCoroutine14invoke_suspend...: 2 suspend point(s)
-- [KXS]   Function @_ZN20SimpleYieldCoroutine14invoke_suspend... already has dispatch (indirectbr/switch); stripping markers
...
=== test_suspension_core ===
test_simple_yield... PASSED
test_conditional_suspend... PASSED
test_loop_suspend... PASSED
test_yield_value_resume_result... PASSED
test_resume_with_value... PASSED
test_start_with_exception_throws... PASSED
=== All tests passed ===
```

---

## 6. Key References

- **Kotlin/Native Lowering**:
  - `kotlin-native/.../llvm/IrToBitcode.kt` (lines 2289–2335)
  - `kotlin-native/.../lower/CoroutinesVarSpillingLowering.kt` (lines 68–105)
- **Clang Documentation**:
  - Labels as Values Extension: https://clang.llvm.org/docs/LanguageExtensions.html#labels-as-values
  - LLVM `indirectbr` Instruction: https://llvm.org/docs/LangRef.html#indirectbr-instruction
- **Repository Implementation**:
  - `src/kotlinx/coroutines/dsl/Suspend.hpp`
  - `cmake/Modules/kxs_transform_ir.cmake`
  - `src/kotlinx/coroutines/kxs_suspend_point.cpp`
