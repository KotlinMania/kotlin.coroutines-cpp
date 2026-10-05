# Clang Suspend DSL Plugin (kotlinx.coroutines-cpp)

This experimental Clang plugin generates coroutine sidecars. Its computed-goto
and liveness-analysis code is separate from the marker cleanup tools. Binary ABI,
Result handoffs, frame lifetimes and automatic spill parity are not established
by the generated address-dispatch shape; see `docs/suspension/IR_SUSPEND_LOWERING_SPEC.md`.

## Features

- Detects suspend functions annotated with `[[suspend]]` or `[[kotlinx::suspend]]`
- Detects suspend points via `suspend(expr)` wrapper or `[[clang::annotate("suspend")]]`
- Generates sidecar `.kx.cpp` files with computed-goto state machines
- Uses `void* _label` (Kotlin/Native NativePtr)
- Generates `&&label` (labels-as-values) + `goto *_label` (computed goto)
- Compiles to LLVM `indirectbr` + `blockaddress`, the same address-dispatch pattern
- CFG-based liveness analysis with complete fixed-point convergence and source-order site IDs
- Direct/no-suspension and sole-tail entries compiled and executed by the handoff regression

Target: Apple clang only.

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
| `dispatch=<mode>` | `goto` | `goto` | State machine dispatch (computed goto) |
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

Direct and sole-tail functions retain their original continuation entry. This
matches the native lowering decision to allocate a state machine only for non-tail
suspensions. Parameter declarations preserve array/reference declarators and
`noexcept`; the incoming continuation is forwarded to the tail callee unchanged.
The generated-code regression executes immediate/delayed success and failure,
checks continuation release, and checks a borrowed array reference.

Non-tail generation still needs value-result consumption, callee continuation
rebinding, spill declaration/reference rewriting and frame lifetime handling.
Generated sidecars are not yet consumed automatically by production CMake. The
working marker cleanup preserves manually lowered frames; it does not implement
these missing extraction steps.

## Verified plugin and handoff tests

Load the plugin with the Clang version matching its LLVM development packages.
The macOS test build uses Homebrew LLVM/Clang 23 and shared `clang-cpp`/`LLVM` so
plugin loading shares the compiler runtime registries. The analyzer is compiled
and linked into the plugin. CTest checks source-order suspension IDs and a
300-block fixed-point liveness regression; a reconstructed 100-iteration cap
fails that regression.

With the optional plugin enabled in the main build, `kxs_plugin_handoff` extracts
real annotated source, compiles its generated C++ against the runtime, and runs
those continuation handoff cases. Sanitizer link options follow the configured
runtime library. This establishes the tested direct/tail entry behavior, not
GPU execution or Kotlin/Native binary ABI compatibility.

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

This expresses the address-dispatch pattern. Full Kotlin/Native parity needs separate frame, result and lifetime validation.

## Architecture

- `KotlinxSuspendPlugin.cpp` - Main plugin: attribute registration, AST visitor, code generation
- `SuspendFunctionAnalyzer.hpp/cpp` - CFG construction and backward dataflow liveness analysis
- Generated code inherits from `ContinuationImpl` and uses the Kotlin/Native continuation ABI
