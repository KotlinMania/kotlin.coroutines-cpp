# IR Suspend Lowering Specification

The docking-ring target is Kotlin/Native's continuation protocol: saved resume
addresses, persistent live values, and the correct immediate/suspended/resumed
result handoffs. Matching two LLVM instructions alone does not establish ABI,
GC, ownership, cancellation, or complete compiler parity.

## Resume addresses and tooling IDs

A switch state machine stores an integer selecting a resume case. Kotlin/Native
instead stores a `NativePtr` identifying a block in the generated `invokeSuspend`
function. Clang's labels-as-values extension can express that same address
pattern with `void* _label`, `&&resume_label`, and `goto *_label`.

The resulting `blockaddress(@function, %resume)` and `indirectbr` destinations
belong to that function. The integer passed to `__kxs_suspend_point(__LINE__)`
is a tooling ID, not the saved address and not the runtime suspension sentinel.
Two coroutine functions may use the same tooling ID without sharing a label.
Neither a code address nor a C++ object's layout can be inferred from that ID.
In a polymorphic C++ frame the first field may be its vtable, not `_label`.

## Kotlin source contracts

Ground truth is the local source, rather than the text format of Clang's IR:

1. `tmp/kotlin/.../lower/NativeSuspendFunctionLowering.kt:55-90,119-170,253-335`
   decides whether non-tail suspension needs a frame and adds a `NativePtr`
   label. Before calling the suspended operation it saves state and the resume
   address, after evaluating side-effecting arguments. The normal path returns
   `COROUTINE_SUSPENDED` only if the call actually suspended; otherwise it uses
   that call's value. The resume path restores state and uses
   `getOrThrow(resultArgument)`. A Unit coercion may discard the value but must
   still propagate a failure.
2. `tmp/kotlin/.../lower/CoroutinesVarSpillingLowering.kt:68-105` replaces
   `saveCoroutineState` and `restoreCoroutineState` with frame-field stores and
   loads for variables live at each suspension point.
3. `tmp/kotlin/.../llvm/IrToBitcode.kt:2289-2348` creates function-local resume
   blocks and the start/dispatch/result merge. Reading a suspension-point ID
   emits its `blockaddress`; earlier suspend lowering supplied the label store.
4. `tmp/kotlin/kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:21-45`
   runs the continuation loop. A suspension marker stops the loop. A value or
   caught exception becomes the next frame's `Result`; interceptor release
   happens when the frame terminates.
5. `SafeContinuationNative.kt` coordinates resume-before-return with suspension.
   The library's `CancellableContinuationImpl.kt:269-328,467-472` similarly
   chooses immediate completion or dispatch using its atomic decision state.
   The runtime sentinel, decision state, resumed payload/failure, and dispatch
   mode are distinct parts of the handoff. Marker cleanup must preserve them.

The vendored `tmp/kotlinx.coroutines/gradle.properties` identifies that library
snapshot as `1.10.2-SNAPSHOT`, with `kotlin_version=2.1.0`. The independently
checked-out Kotlin compiler is `fee29910d8dddd2b1f7b44036c00533cee493351` and its
build defaults name Kotlin/Native `2.5.0-dev-5907` and LLVM 21. This sparse
compiler checkout does not establish which kotlinx.coroutines artifact is
bundled with a compiler distribution.

## Source lowering and continuation routing

`src/kotlinx/coroutines/dsl/Suspend.hpp` generates the dispatch and normal/resume
branches. Live state must already be in a retained frame. `coroutine_yield_value`
uses the immediate call value on the normal path and `result.get_or_throw()` on
resume. `coroutine_yield` discards a successful Unit value but still checks for
failure. The no-op tooling marker is declared `noexcept`.

A suspended operation must receive the current frame's continuation so its
completion re-enters that frame. Passing only the frame's parent completion
would skip the frame's remaining work. For example:

```cpp
class ExampleCoroutine : public ContinuationImpl {
public:
    void* _label = nullptr;
    int accumulator = 0;

    explicit ExampleCoroutine(std::shared_ptr<Continuation<void*>> parent)
        : ContinuationImpl(std::move(parent)) {}

    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)
        accumulator = 10;
        coroutine_yield(this, delay(100, this));
        accumulator += 20;
        return &accumulator;  // Example only: caller must retain this frame.
    }
};
```

C++ locals and objects whose lifetimes cross suspension require explicit frame
storage and valid lifetimes. The macros do not implement automatic spilling,
RAII lifetime lowering, or the compiler's argument-evaluation rewrite.

## Cleanup pipeline

The source/compiler owns coroutine lowering. These tools remove the reserved
no-op `__kxs_suspend_point` calls from already lowered IR:

- `kxs-inject` uses LLVM's parser, validates the module before and after cleanup,
  and removes direct `CallInst` uses of the correctly typed marker. It preserves
  existing blocks, instructions, result paths, and frame accesses. An unused
  marker declaration can be erased only when LLVM reports no remaining uses.
  Invokes, aliases, and function-pointer uses keep the declaration.
- `kxs_transform_ir.cmake` invokes the conservative Python text fallback. It
  recognizes complete single-line direct calls with a constant i32 ID, optional
  `noundef`, call attributes and numbered metadata attachments. Every other byte
  is preserved, including comments, quoted strings, CRLF, declarations and
  unsupported calls. It does not split the module into regex-defined functions.
- Neither tool invents a dispatch path or spill fields when a marker appears in
  an otherwise ordinary function. An existing `switch` or `indirectbr` also
  does not prove a coroutine is correct.

`kxs_enable_coroutine_transform(target)` installs a C++ compiler launcher for
Clang with Ninja or Unix Makefiles, requiring Python 3.8+. It wraps CMake's real
compile command, so source/target/transitive flags, toolchain settings, generated
sources, unique object paths and header dependencies are retained. It emits
frontend IR with LLVM passes disabled, cleans markers, then runs optimization
and code generation once. The original source produces one linked object.
Existing compiler launchers are chained. Header/PCH generation passes through.
Other generators and C++ module/BMI workflows have not been validated.

```cmake
include(KotlinxCoroutines)
add_executable(my_target my_source.cpp)
target_link_libraries(my_target PRIVATE kotlinx::coroutines)
kxs_enable_suspend(my_target)
```

## Verification

```bash
cmake -S . -B build -DKOTLINX_BUILD_CLANG_SUSPEND_PLUGIN=OFF -DKOTLINX_BUILD_KXS_INJECT=OFF
cmake --build build --target test_suspension_core
ctest --test-dir build -R '^(test_suspension_core|test_ir_pipeline)$' --output-on-failure
```

`test_ir_pipeline` compares untransformed and cleaned builds using the actual
macro tests. It covers immediate and resumed values, failure propagation,
repeated suspension, independent frames, transitive C++20/include/definition
settings, source-specific options, generated sources, duplicate basenames,
paths with spaces, header-triggered rebuilds, and an optimized AddressSanitizer
build. It also checks byte preservation and unsupported marker uses. Ninja
Multi-Config is exercised when Ninja is installed.

These checks validate cleanup and the tested handoffs. Complete Kotlin/Native
ABI and automatic spilling parity still require compiler-frame metadata,
liveness lowering, ownership/GC integration and upstream parity tests.
