# Clang Suspend DSL Plugin (kotlinx.coroutines-cpp)

This Clang plugin transforms C++ suspend functions into Kotlin/Native-style state machines.

## Features

### Phase 1 (Complete): Switch-based Dispatch
- Detects suspend functions annotated with `[[suspend]]` or `[[kotlinx::suspend]]`
- Detects suspend points via `suspend(expr)` wrapper or `[[clang::annotate("suspend")]]`
- Generates sidecar `.kx.cpp` files with switch-based state machines
- Captures all parameters as coroutine class fields

### Phase 2 (Complete): Liveness-based Spilling
- CFG construction using Clang's analysis infrastructure
- True backward dataflow liveness analysis (matching Kotlin/Native)
- Only spills variables that are actually live across suspension points
- Reduces memory footprint of coroutine objects

### Phase 3 (Complete): Computed-Goto Dispatch
- Uses `void* _label` instead of `int _label`
- Generates `&&label` (GCC/Clang labels-as-values extension)
- Uses `goto *_label` for computed goto dispatch
- Compiles to LLVM `indirectbr` + `blockaddress` - exact Kotlin/Native parity

## Building (Apple/Clang)

Requires a Clang/LLVM installation with CMake package configs.

```bash
mkdir build && cd build
cmake -DKOTLINX_BUILD_CLANG_SUSPEND_PLUGIN=ON ..
make KotlinxSuspendPlugin
```

The plugin dylib/so is emitted into `build/lib/` with platform suffix.

## Usage

### Basic (Phase 1 defaults)
```bash
clang++ -fsyntax-only \
  -Xclang -load -Xclang build/lib/KotlinxSuspendPlugin.dylib \
  -Xclang -plugin -Xclang kotlinx-suspend \
  -Xclang -plugin-arg-kotlinx-suspend -Xclang out-dir=build/kxs_generated \
  path/to/file.cpp
```

### With Liveness Analysis (Phase 2)
```bash
clang++ -fsyntax-only \
  -Xclang -load -Xclang build/lib/KotlinxSuspendPlugin.dylib \
  -Xclang -plugin -Xclang kotlinx-suspend \
  -Xclang -plugin-arg-kotlinx-suspend -Xclang out-dir=build/kxs_generated \
  -Xclang -plugin-arg-kotlinx-suspend -Xclang spill=liveness \
  path/to/file.cpp
```

### With Computed Gotos (Phase 3 - Kotlin/Native parity)
```bash
clang++ -fsyntax-only \
  -Xclang -load -Xclang build/lib/KotlinxSuspendPlugin.dylib \
  -Xclang -plugin -Xclang kotlinx-suspend \
  -Xclang -plugin-arg-kotlinx-suspend -Xclang out-dir=build/kxs_generated \
  -Xclang -plugin-arg-kotlinx-suspend -Xclang dispatch=goto \
  -Xclang -plugin-arg-kotlinx-suspend -Xclang spill=liveness \
  path/to/file.cpp
```

## Plugin Arguments

| Argument | Values | Default | Description |
|----------|--------|---------|-------------|
| `out-dir=<path>` | directory path | `kxs_generated` | Output directory for generated `.kx.cpp` files |
| `dispatch=<mode>` | `switch`, `goto` | `switch` | State machine dispatch mode |
| `spill=<mode>` | `all`, `liveness` | `all` | Variable spilling strategy |

## Example

Input:
```cpp
using namespace kotlinx::coroutines::dsl;

[[suspend]]
void* my_suspend_fn(int x, std::shared_ptr<Continuation<void*>> completion) {
    int y = x + 1;
    suspend(delay(100, completion));  // suspension point
    return reinterpret_cast<void*>(y);
}
```

Generated output (dispatch=goto, spill=liveness):
```cpp
struct __kxs_coroutine_my_suspend_fn_1 : public ContinuationImpl {
    void* _label = nullptr;  // Block address for computed goto
    int y_spill;  // Only 'y' is live across suspend

    explicit __kxs_coroutine_my_suspend_fn_1(
        std::shared_ptr<Continuation<void*>> completion, int x)
        : ContinuationImpl(completion) {}

    void* invoke_suspend(Result<void*> result) override {
        (void)result;

        // Entry dispatch (Kotlin/Native indirectbr pattern)
        if (_label == nullptr) goto __kxs_start;
        goto *_label;  // Computed goto -> LLVM indirectbr

    __kxs_start:
        int y = x + 1;
        y_spill = y;  // Save live variable
        _label = &&__kxs_resume0;  // Store block address
        {
            void* _tmp = delay(100, completion);
            if (is_coroutine_suspended(_tmp)) return COROUTINE_SUSPENDED;
        }
    __kxs_resume0:
        y = y_spill;  // Restore live variable
        return reinterpret_cast<void*>(y);
    }
};
```

## LLVM IR Output

The computed-goto pattern compiles to:
```llvm
entry:
  %label = load ptr, ptr %_label
  %is_null = icmp eq ptr %label, null
  br i1 %is_null, label %start, label %dispatch

dispatch:
  indirectbr ptr %label, [label %resume0, label %resume1, ...]

start:
  ; ... normal execution ...
  store ptr blockaddress(@invoke_suspend, %resume0), ptr %_label
  ; ... suspend call ...

resume0:
  ; ... resume execution ...
```

This is exact parity with Kotlin/Native's coroutine implementation.

## Architecture

- `KotlinxSuspendPlugin.cpp` - Main plugin: attribute registration, AST visitor, code generation
- `SuspendFunctionAnalyzer.hpp/cpp` - CFG construction and backward dataflow liveness analysis
- Generated code inherits from `ContinuationImpl` and uses the Kotlin/Native continuation ABI
