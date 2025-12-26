# IR Suspend Lowering Specification

This document specifies how suspend points are lowered to LLVM IR, matching
Kotlin/Native's coroutine implementation pattern.

## Source: Kotlin/Native

The patterns here are derived from:
- `kotlin-native/.../llvm/IrToBitcode.kt` - `evaluateSuspendableExpression()`, `evaluateSuspensionPoint()`
- `kotlin-native/.../lower/CoroutinesVarSpillingLowering.kt` - variable spilling

## Overview

Suspend functions use computed goto (`indirectbr` + `blockaddress`) for resumption:

1. **Entry dispatch**: Check if `_label` is null (first call) or contains a resume address
2. **Suspend point**: Store `blockaddress(@func, %resume_label)` to `_label`, return `COROUTINE_SUSPENDED`
3. **Resume**: `indirectbr` jumps directly to the stored label

---

## CMake Infrastructure

The transformation is implemented via CMake modules in `cmake/Modules/`:

### Key Files

| File | Purpose |
|------|---------|
| `kxs_transform_ir.cmake` | Standalone IR transformation script (invoked via `cmake -P`) |
| `KotlinxCoroutineTransform.cmake` | CMake functions: `kxs_transform_ir()`, `kxs_enable_coroutine_transform()` |
| `KotlinxCoroutines.cmake` | Interface library `kotlinx::coroutines` and `kxs_enable_suspend()` |

### Enabling Transformation for a Target

```cmake
include(KotlinxCoroutines)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE kotlinx::coroutines)
kxs_enable_suspend(my_app)  # Enable IR transformation
```

Or use the convenience macro:
```cmake
kxs_add_executable(my_app SOURCES main.cpp)
```

### Transformation Pipeline

```
┌─────────────┐     ┌──────────────────┐     ┌─────────────────┐     ┌─────────┐
│  foo.cpp    │ --> │  foo.ll          │ --> │  foo.kxs.ll     │ --> │  foo.o  │
│  (C++ src)  │     │  (LLVM IR text)  │     │  (transformed)  │     │ (object)│
└─────────────┘     └──────────────────┘     └─────────────────┘     └─────────┘
                    clang -emit-llvm -S      cmake -P                 clang -c
                                             kxs_transform_ir.cmake
```

### What kxs_transform_ir.cmake Does

1. **Finds** calls to `__kxs_suspend_point(i32 N)` in the IR
2. **Generates** resume labels: `kxs.resume.0`, `kxs.resume.1`, ...
3. **Inserts** entry dispatch block with `indirectbr`
4. **Replaces** each suspend call with `blockaddress` store + resume label
5. **Outputs** transformed IR with header comment

---

## C++ Source Pattern

### Macros in `src/kotlinx/coroutines/dsl/Suspend.hpp`

Uses Clang labels-as-values extension (`&&label`). Generates `indirectbr` + `blockaddress` in LLVM IR,
matching Kotlin/Native exactly.

```cpp
class MyCoroutine : public ContinuationImpl {
    void* _label = nullptr;  // blockaddress storage

    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)

        // ... code ...
        coroutine_yield(this, some_suspend_call(completion_));
        // ... more code ...

        coroutine_end(this)
    }
};
```

### The `__LINE__` Trick

The macros use `__LINE__` to generate unique labels at each suspend point:

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

At line 42, this expands to:

```cpp
(c)->_label = &&_kxs_resume_42;      // Store blockaddress
__kxs_suspend_point(42);              // IR marker (found by CMake transform)
auto _kxs_tmp = (expr);
if (is_coroutine_suspended(_kxs_tmp))
    return _kxs_tmp;
goto _kxs_cont_42;
_kxs_resume_42:                       // Resume label
    (void)(result).get_or_throw();
_kxs_cont_42:;                        // Continuation
```

### IR Marker Function

```cpp
// Declared in Suspend.hpp
extern "C" void __kxs_suspend_point(int id);
```

This function:
- Is a **no-op** at runtime (can be empty or just return)
- Serves as an **IR-visible marker** that survives compilation
- Is found by `kxs_transform_ir.cmake` via regex: `call void @__kxs_suspend_point\(i32[^)]*\)`
- The `id` argument (from `__LINE__`) identifies each suspend point

---

## LLVM IR Transformation Details

### Input IR Pattern

```llvm
define ptr @my_coroutine(ptr %coro, ptr %result) {
entry:
  ; ... setup ...
  call void @__kxs_suspend_point(i32 42)
  ; ... more code ...
  call void @__kxs_suspend_point(i32 57)
  ; ...
}
```

### Output IR Pattern (Kotlin/Native style)

```llvm
; KXS-TRANSFORMED: 2 suspend points
; Resume labels: kxs.resume.0;kxs.resume.1
; Dispatch: indirectbr + blockaddress (Kotlin/Native pattern)

define ptr @my_coroutine(ptr %coro, ptr %result) {
kxs.entry:
  ; KXS: Allocate label slot for computed goto
  %kxs.label.slot = alloca ptr, align 8
  store ptr null, ptr %kxs.label.slot, align 8
  ; KXS: Load resume label (null on first call)
  %kxs.saved.label = load ptr, ptr %kxs.label.slot, align 8
  %kxs.is.first = icmp eq ptr %kxs.saved.label, null
  br i1 %kxs.is.first, label %kxs.start, label %kxs.dispatch

kxs.dispatch:
  ; KXS: Resume via computed goto (indirectbr)
  indirectbr ptr %kxs.saved.label, [label %kxs.resume.0, label %kxs.resume.1]

kxs.start:
  ; ... original entry code ...

  ; KXS: Store resume address (blockaddress pattern)
  store ptr blockaddress(@my_coroutine, %kxs.resume.0), ptr %kxs.label.slot, align 8
  ; KXS: suspend point 42 - actual suspend call would precede this
  br label %kxs.resume.0

kxs.resume.0:  ; KXS resume point 42
  ; ... code continues ...

  ; KXS: Store resume address (blockaddress pattern)
  store ptr blockaddress(@my_coroutine, %kxs.resume.1), ptr %kxs.label.slot, align 8
  ; KXS: suspend point 57
  br label %kxs.resume.1

kxs.resume.1:  ; KXS resume point 57
  ; ...
}
```

---

## Algorithm: kxs_transform_ir.cmake

The CMake script (`cmake -P kxs_transform_ir.cmake`) performs these steps:

### Step 1: Find Suspend Points

```cmake
string(REGEX MATCHALL "call void @__kxs_suspend_point\\(i32[^)]*\\)" SUSPEND_CALLS "${IR}")
```

### Step 2: Extract Function Name

```cmake
string(REGEX MATCH "define [^@]+@([A-Za-z_][A-Za-z0-9_]*)" FUNC_MATCH "${IR}")
```

### Step 3: Generate Resume Labels

```cmake
set(IDX 0)
while(IDX LESS N_SUSPEND)
    list(APPEND LABELS "kxs.resume.${IDX}")
    math(EXPR IDX "${IDX} + 1")
endwhile()
```

### Step 4: Build Dispatch Label List

```cmake
# Generates: label %kxs.resume.0, label %kxs.resume.1, ...
foreach(L ${LABELS})
    string(APPEND LABEL_LIST "label %${L}")
endforeach()
```

### Step 5: Replace Each Suspend Call

Each `call void @__kxs_suspend_point(i32 N)` is replaced with:
```llvm
  ; KXS: Store resume address (blockaddress pattern)
  store ptr blockaddress(@${FUNC_NAME}, %${RESUME_LABEL}), ptr %kxs.label.slot, align 8
  ; KXS: suspend point ${SID}
  br label %${RESUME_LABEL}

${RESUME_LABEL}:  ; KXS resume point ${SID}
```

### Step 6: Insert Entry Dispatch Block

After the function opening `{`, insert:
```llvm
kxs.entry:
  %kxs.label.slot = alloca ptr, align 8
  store ptr null, ptr %kxs.label.slot, align 8
  %kxs.saved.label = load ptr, ptr %kxs.label.slot, align 8
  %kxs.is.first = icmp eq ptr %kxs.saved.label, null
  br i1 %kxs.is.first, label %kxs.start, label %kxs.dispatch

kxs.dispatch:
  indirectbr ptr %kxs.saved.label, [${LABEL_LIST}]

kxs.start:
```

---

## Variable Spilling (Future)

From `CoroutinesVarSpillingLowering.kt`:

At each suspend point, live variables must be saved to the coroutine struct
before suspension and restored after resumption:

```kotlin
// Before suspend:
saveCoroutineState -> {
    for (variable in liveVariables) {
        coroutine.field[variable] = variable
    }
}

// After resume:
restoreCoroutineState -> {
    for (variable in liveVariables) {
        variable = coroutine.field[variable]
    }
}
```

This requires liveness analysis to determine which variables cross suspend points.

**Current approach**: Manual spilling - developers declare spill fields in their ContinuationImpl class.

**Future**: `kxs-inject` tool will perform liveness analysis and generate spill code automatically.

---

## Coroutine Struct Layout

The coroutine struct must have `_label` as a field (typically first for efficiency):

```cpp
struct MyCoroutine : public ContinuationImpl {
    void* _label = nullptr;  // blockaddress storage (Kotlin/Native NativePtr)

    // ... spilled variables ...
    int saved_count;
    std::string saved_name;
};
```

In LLVM IR terms:
```llvm
%Coroutine = type { ptr, ... }  ; first field is label pointer
```

---

## Compiler Flags for Tests

Tests using the DSL macros need:

```cmake
target_compile_options(my_test PRIVATE
    -Wno-gnu-label-as-value    # Allow &&label (labels as values)
)
```

---

## References

- Kotlin/Native IrToBitcode.kt lines 2377-2424
- Kotlin/Native CoroutinesVarSpillingLowering.kt
- Clang Labels as Values: https://clang.llvm.org/docs/LanguageExtensions.html#labels-as-values
- LLVM indirectbr: https://llvm.org/docs/LangRef.html#indirectbr-instruction
- CMake Modules: `cmake/Modules/kxs_transform_ir.cmake`, `KotlinxCoroutineTransform.cmake`, `KotlinxCoroutines.cmake`
