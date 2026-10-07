# Suspend Function Implementation

The current implementation uses Clang authoring markers and mandatory LLVM injection via
`src/kotlinx/coroutines/dsl/Suspend.hpp` and the `KotlinxCoroutinePass` LLVM plugin. The design target and compiler contracts
are described in [the docking ring](../architecture/docking_ring.md) and the
[IR specification](IR_SUSPEND_LOWERING_SPEC.md).

## Runtime handoffs

A coroutine frame retains a `void* _label` and every value needed after
suspension. `coroutine_begin` supplies the address of that frame field. The injector selects
fresh execution or its saved resume block and injects the next block-address store
at each yield before the operation. The suspended operation receives that frame's continuation.

- If the call returns an ordinary value, execution continues with that value.
- If it returns `COROUTINE_SUSPENDED`, the frame returns that sentinel immediately.
- A subsequent resume restores the frame's state and consumes its supplied
  `Result`. A successful value becomes the yield expression's value; a failure
  is rethrown. Unit yields still consume failures.
- `BaseContinuationImpl::resume_with` stops on suspension and passes a completed
  value or failure to the parent frame. The runtime's atomic suspension decision
  prevents resume-before-return from completing the operation twice.

The `__LINE__` integer in `__kxs_suspend_point` is a tooling ID. It is separate
from both the runtime suspension sentinel and the saved block address.

## Persistent state

Store live state directly in a frame member rather than a stack local that is
re-created on each `invoke_suspend`. Keep nontrivial C++ object lifetimes valid
when dispatch jumps to a resume block. Neither the authoring markers nor dispatch injection
automatically lower live locals, RAII lifetimes, or arbitrary suspend expressions.

```cpp
class Example : public ContinuationImpl {
public:
    void* _label = nullptr;
    int count = 0;
    void* value = nullptr;

    explicit Example(std::shared_ptr<Continuation<void*>> parent)
        : ContinuationImpl(std::move(parent)) {}

    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)
        count = 42;
        coroutine_yield_value(this, result, fetch_async(this), value);
        use_value(count, value);
        return value;
    }
};
```

The operation must retain the continuation for as long as it can suspend.
Passing the parent completion instead would bypass `use_value` on resumption.
Each yield macro must appear on a unique source line within its function.

## Mandatory IR injection

`__kxs_coroutine_begin(void**)` identifies the persistent frame label field;
`__kxs_suspend_point(int, void**, void*)` identifies the actual function-local resume
address. Neither has a runtime implementation. The in-compiler LLVM pass creates the
saved-label load, entry branch, indirect dispatch and resume-address stores.
It does not allocate/reset a stack label slot or infer `_label` from offset zero.

```cmake
include(KotlinxCoroutines)
add_executable(my_target my_source.cpp)
target_link_libraries(my_target PRIVATE kotlinx::coroutines)
kxs_enable_suspend(my_target)
```

The helper requires Clang and a KotlinxCoroutinePass built against that compiler’s
LLVM development package. It adds `-fpass-plugin` to CMake’s ordinary compile
command. The module stays inside the compiler through injection, optimization,
sanitizer instrumentation and code generation. Header and plugin dependencies,
transitive settings, object names and existing launchers are retained.
Untransformed suspend definitions cannot link; invalid marker contracts produce
compiler diagnostics. See the IR specification for toolchain configuration.

## Checks and remaining work

Build `test_suspension_core`, then run CTest for `test_suspension_core` and
`test_ir_pipeline` and `test_kxs_compiler_pass`. The pipeline suite exercises the actual injected build,
including an optimized AddressSanitizer build. See the IR specification for
commands and covered handoffs.

The injector constructs Kotlin/Native address dispatch from LLVM block values. Full
interoperability additionally needs exact frame/result/ownership/GC contracts,
automatic spill lowering, and cancellation/dispatcher parity. The separate Clang
DSL plugin is experimental; its liveness analysis is not yet connected to this injector for automatic spill
generation.
