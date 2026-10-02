# Suspend Function Implementation

The current implementation uses Clang computed-goto macros in
`src/kotlinx/coroutines/dsl/Suspend.hpp`. The design target and compiler contracts
are described in [the docking ring](cpp_port/docking_ring.md) and the
[IR specification](IR_SUSPEND_LOWERING_SPEC.md).

## Runtime handoffs

A coroutine frame retains a `void* _label` and every value needed after
suspension. `coroutine_begin` selects fresh execution or the saved resume block.
At each yield the frame stores its next block address before calling the
operation. The suspended operation receives that frame's continuation.

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
when dispatch jumps to a resume block. Neither the macros nor marker cleanup
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

## Optional IR cleanup

The reserved marker is a `noexcept` no-op supplied by
`src/kotlinx/coroutines/kxs_suspend_point.cpp` in ordinary builds.
`kxs_transform_ir.cmake` removes only recognized marker instructions and
preserves all other IR bytes. `kxs-inject` uses LLVM parsing and verifies the
module, then removes direct marker calls. Both preserve Clang's existing label
stores, dispatch, spill fields and value/failure branches. Neither performs
liveness analysis, generates spill fields, nor assumes `_label` is at offset 0.

```cmake
include(KotlinxCoroutines)
add_executable(my_target my_source.cpp)
target_link_libraries(my_target PRIVATE kotlinx::coroutines)
kxs_enable_suspend(my_target)
```

The helper requires Clang, Python 3.8+, and Ninja or Unix Makefiles. It wraps the
actual CMake compile command and leaves CMake in charge of source properties,
transitive settings, object names and dependencies. Other generators and C++
module/BMI workflows have not been validated. `Suspend.hpp` requires Clang;
there is no current GCC/MSVC switch fallback.

## Checks and remaining work

Build `test_suspension_core`, then run CTest for `test_suspension_core` and
`test_ir_pipeline`. The pipeline suite compares ordinary and cleaned builds,
including an optimized AddressSanitizer build. See the IR specification for
commands and covered handoffs.

Computed goto yields the same address-dispatch pattern as Kotlin/Native. Full
interoperability additionally needs exact frame/result/ownership/GC contracts,
automatic spill lowering, and cancellation/dispatcher parity. The separate Clang
DSL plugin is experimental; its liveness analysis is not a feature of marker
cleanup.
