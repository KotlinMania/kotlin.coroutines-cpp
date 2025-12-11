# Clang Suspend DSL Plugin (kotlinx.coroutines-cpp)

This is the **Phase‑1** Clang plugin scaffold for the C++ suspend DSL.

Current behavior:
- Detects suspend functions annotated with `[[kotlinx::suspend]]` (registered by the plugin) or `[[clang::annotate("kotlinx_suspend")]]`.
- Detects suspend points via `[[clang::annotate("kotlinx_suspend_call")]]` on a statement, or a call to `kx::suspend_call(...)`.
- Emits a generated sidecar translation unit (`.kx.cpp`) containing a Kotlin‑Native‑shape state machine.
  The original translation unit is not modified yet.

## Building (Apple/Clang)

Requires a Clang/LLVM installation with CMake package configs.

```bash
mkdir build && cd build
cmake -DKOTLINX_BUILD_CLANG_SUSPEND_PLUGIN=ON ..
make KotlinxSuspendPlugin
```

The plugin dylib/so is emitted into `build/lib/` with platform suffix.

## Using (generate sidecar)

```bash
clang++ -fsyntax-only \
  -Xclang -load -Xclang build/lib/KotlinxSuspendPlugin.dylib \
  -Xclang -plugin -Xclang kotlinx-suspend \
  -Xclang -plugin-arg-kotlinx-suspend -Xclang out-dir=build/kxs_generated \
  path/to/file.cpp
```

Example annotations:

```cpp
[[kotlinx::suspend]]
void* my_suspend_fn(std::shared_ptr<Continuation<void*>> completion) {
    int x = 1;
    [[clang::annotate("kotlinx_suspend_call")]]
    foo_suspend(completion);
    return nullptr;
}
```

## Next milestones

1. Register statement attribute `[[kotlinx::suspend_call]]`.
2. CFG liveness + automatic spill inference.
3. In‑memory AST rewrite into Kotlin‑style state machine (computed‑goto on Clang).
