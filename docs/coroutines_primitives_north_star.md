# Kotlin Coroutines Primitives North Star

This document defines the fundamental primitives required to implement Kotlin Coroutines in C++, distinguishing between Compiler Intrinsics (Plugin support) and Library Code.

## 1. The Compiler Intrinsics (Kernel)
These are the absolute minimum requirements provided by the language implementation (Compiler/Plugin). They cannot be implemented in pure C++ library code.

### A. The `suspend` Transformation
*   **Concept**: Transformation of a function `R foo(Args)` into `void foo(Args, Continuation<R>)` (CPS transformation).
*   **C++ Mapping**: `[[suspend]]` attribute.
*   **Requirement**: The plugin must rewrite calls to `[[suspend]]` functions to inject the current continuation.

### B. `suspendCoroutineUninterceptedOrReturn`
*   **Concept**: Providing access to the *implicit* continuation object injected by the compiler.
*   **Kotlin Signature**: `suspend fun <T> suspendCoroutineUninterceptedOrReturn(block: (Continuation<T>) -> Any?): T`
*   **Behavior**:
    1.  Obtains the current `Continuation<T>`.
    2.  Passes it to `block`.
    3.  If `block` returns `COROUTINE_SUSPENDED`, the function returns early.
    4.  If `block` returns a value, the function returns that value immediately.
*   **C++ Mapping**: This is the "magic" function the plugin must support (or emulate via `suspend` keyword macro hacks in early stages).
*   **Status**: Currently emulated via manual `Continuation` passing in our C++ implementation, but ideally target of the plugin.

### C. `COROUTINE_SUSPENDED`
*   **Concept**: A sentinel value indicating that the function has suspended and will resume later.
*   **C++ Mapping**: `nullptr` (void*) or a special global token.

---

## 2. Low-Level Library Primitives (Runtime ABI)
These define the contract for the runtime.

### A. `Continuation<T>`
*   **Role**: A callback interface wrapping the state machine.
*   **Methods**: `resumeWith(Result<T>)`.
*   **Structure**: Points to `CoroutineContext`.

### B. `ContinuationInterceptor` ("The Mode")
*   **Role**: Intercepts the continuation to wrap it in a `DispatchedContinuation`.
*   **Usage**: Controls **threading** and execution policy (Main thread, IO pool, Unconfined).
*   **Mechanism**: `interceptContinuation(Continuation<T>) -> Continuation<T>`.

### C. `startCoroutine` / `createCoroutine`
*   **Role**: Bootstrapping the coroutine execution.
*   **Implementation**: Creates the initial state machine.

---

## 3. High-Level Library Primitives (User Facing)
These are implemented *in the library* using the intrinsics headers. They do **not** require their own compiler magic, only generic support for `suspend`.

### A. `suspendCancellableCoroutine`
*   **Role**: The primary user-facing primitive for suspension.
*   **Logic**:
    1.  Calls `suspendCoroutineUninterceptedOrReturn`.
    2.  Wraps the raw continuation in a `CancellableContinuationImpl`.
    3.  Sets up parent-child cancellation relations.
    4.  Handles `ContinuationInterceptor` dispatching.
*   **Dependency**: Built entirely on top of `suspendCoroutineUninterceptedOrReturn` and `Job`.

### B. `Deferred.await()` / `Job.join()`
*   **Role**: Suspending functions to wait for results/completion.
*   **Implementation**:
    *   **Is it an intrinsic?** NO.
    *   **Is IR/Plugin required?** YES, for *calling* it.
    *   **Why?** Because it is a `suspend` function. Calling `deferred.await()` must be rewritten by the plugin to pass the continuation, just like any other suspend function.
    *   **Internal Logic**: It calls `suspendCancellableCoroutine`.

---

## 4. Status Check & Recommendations

### Primitives We Have
*   `[[suspend]]` (Plugin-aware stubs).
*   `Continuation<T>`, `CoroutineContext`.
*   `Job`, `Deferred`.

### Primitives We Need (The North Star)
1.  **Fully Working `suspendCancellableCoroutine`**:
    *   Currently, we use a hybrid of stubs and `CancellableContinuationImpl`.
    *   **Goal**: Emulate `Kotlin`'s implementation exactly, calling `suspendCoroutineUninterceptedOrReturn`.

2.  **`Deferred.await()` Implementation**:
    *   **Current**: `virtual void* await(Continuation*)`.
    *   **Goal**: Keep this as the *backend*, but expose a `[[suspend]] T await()` method (or extension) that delegates to it. The `dsl/Await.hpp` is the correct step here.

3.  **Threading Modes**:
    *   We have `Dispatchers::Default` (Thread Pool) and `Dispatchers::Main` (Stubs).
    *   We have `ContinuationInterceptor`.
    *   **Goal**: Ensure `suspendCancellableCoroutine` correctly uses the interceptor to dispatch resumption.

### Conclusion on Deferred.await()
**Deferred.await() is a standard suspending function.**
*   **Design**: It should be defined as `[[suspend]] T await()`.
*   **IR Role**: The plugin transforms the call `x.await()` -> `x.await(current_cont)`.
*   **Impl Role**: The implementation uses `suspendCancellableCoroutine` (or optimized internal equivalent like `awaitInternal`) to register the continuation with the job.

---

## 5. Concrete Implementation Patterns

### A. State Machine Pattern (Computed Goto - Kotlin/Native Parity)

The correct state machine uses computed gotos (`goto *_label`) with labels-as-values. This compiles to LLVM `indirectbr` - exactly what Kotlin/Native generates.

```cpp
// User writes:
[[suspend]]
void* my_suspend_fn(int x, std::shared_ptr<Continuation<void*>> completion) {
    std::cout << "Before suspend" << std::endl;
    suspend(yield(completion));
    std::cout << "After suspend" << std::endl;
    return nullptr;
}

// Plugin generates (in .kx.cpp):
struct __kxs_coroutine_my_suspend_fn : public ContinuationImpl {
    void* _label = nullptr;  // NativePtr - stores &&label address
    int x_;  // spilled parameter

    __kxs_coroutine_my_suspend_fn(std::shared_ptr<Continuation<void*>> completion, int x)
        : ContinuationImpl(completion), x_(x) {}

    void* invoke_suspend(Result<void*> result) override {
        // Entry dispatch - Kotlin/Native pattern
        if (_label == nullptr) goto __kxs_start;
        goto *_label;  // computed goto (indirectbr in LLVM)

    __kxs_start:
        std::cout << "Before suspend" << std::endl;

        // At suspension point:
        _label = &&__kxs_resume0;  // store block address
        {
            void* _tmp = yield(this->completion);
            if (is_coroutine_suspended(_tmp)) return COROUTINE_SUSPENDED;
        }

    __kxs_resume0:
        std::cout << "After suspend" << std::endl;
        return nullptr;
    }
};

void* my_suspend_fn(int x, std::shared_ptr<Continuation<void*>> completion) {
    auto coro = std::make_shared<__kxs_coroutine_my_suspend_fn>(completion, x);
    return coro->invoke_suspend(Result<void*>::success(nullptr));
}
```

**Key differences from switch/case:**
- `void* _label` instead of `int _label`
- `goto *_label` for dispatch (computed goto)
- `&&label_name` to get block address
- No switch/case overhead - direct jump
- LLVM compiles to `indirectbr` instruction

### B. Manual Suspend Function Pattern (Library Code)

For library suspend functions like `yield()`, `delay()`, and `await()`, we implement them manually using `suspendCoroutineUninterceptedOrReturn` semantics:

```cpp
// yield() implementation pattern - Transliterated from Yield.kt
void* yield_impl(std::shared_ptr<Continuation<void*>> completion) {
    // 1. Get the context
    auto context = completion->get_context();

    // 2. Check for cancellation (ensure active)
    context_ensure_active(*context);

    // 3. Get the dispatcher (continuation interceptor)
    auto interceptor = context->get(ContinuationInterceptor::type_key);
    auto* dispatcher = dynamic_cast<CoroutineDispatcher*>(interceptor.get());

    if (!dispatcher) {
        // No dispatcher - yield is a no-op (or thread yield)
        std::this_thread::yield();
        return nullptr;  // Completed immediately
    }

    // 4. Schedule resumption via dispatcher and suspend
    // Create a DispatchedContinuation that wraps the completion
    auto dispatched = std::make_shared<DispatchedContinuation<Unit>>(
        dispatcher, completion->get_context(), completion
    );

    // 5. Dispatch and return COROUTINE_SUSPENDED
    dispatched->dispatch_yield(*context, Unit{});
    return get_COROUTINE_SUSPENDED();
}
```

### C. `suspendCancellableCoroutine` Pattern

The canonical way to implement suspend functions that need cancellation support:

```cpp
template<typename T>
void* suspend_cancellable_coroutine(
    std::function<void(CancellableContinuation<T>&)> block,
    Continuation<void*>* completion
) {
    // 1. Create a CancellableContinuationImpl wrapping the completion
    auto cont = std::make_shared<CancellableContinuationImpl<T>>(
        completion, CancellableMode::CANCELLABLE
    );

    // 2. Initialize parent job relationship
    cont->init_cancellability();

    // 3. Call the user's block with the continuation
    block(*cont);

    // 4. Get the result - either value or COROUTINE_SUSPENDED
    return cont->get_result();
}
```

### D. `await()` Pattern (on Deferred)

```cpp
// In AbstractCoroutine or JobSupport
void* await_internal(Continuation<void*>* continuation) {
    // Fast path: already completed
    auto* state = get_state();
    if (is_completed_state(state)) {
        if (auto* ex = dynamic_cast<CompletedExceptionally*>(state)) {
            std::rethrow_exception(ex->cause);
        }
        // Return the value directly (no suspension needed)
        return state;
    }

    // Slow path: need to suspend and wait
    return suspend_cancellable_coroutine<void*>([this](CancellableContinuation<void*>& cont) {
        // Register handler to resume when job completes
        invoke_on_completion([&cont](std::exception_ptr cause) {
            if (cause) {
                cont.resume_with_exception(cause);
            } else {
                cont.resume(get_completed_value());
            }
        });
    }, continuation);
}
```

### E. Key Patterns Summary

| Function | Returns | Suspends? | Pattern |
|----------|---------|-----------|---------|
| `yield()` | `void*` | Yes | Dispatch-yield, return `COROUTINE_SUSPENDED` |
| `delay(ms)` | `void*` | Yes | Schedule timer, return `COROUTINE_SUSPENDED` |
| `await()` | `void*` | Conditionally | Check completion, else `suspend_cancellable_coroutine` |
| `join()` | `void*` | Conditionally | Check completion, else `suspend_cancellable_coroutine` |
| `receive()` | `void*` | Conditionally | Check channel, else `suspend_cancellable_coroutine` |
| `emit()` | `void*` | Conditionally | Check backpressure, else `suspend_cancellable_coroutine` |

### F. The Event Loop Contract

For blocking tests (using `run_blocking`), the `BlockingEventLoop` must:

1. Process dispatched tasks in a loop
2. Wake up when new tasks arrive
3. Exit when the root coroutine completes

```cpp
class BlockingEventLoop : public EventLoop {
public:
    void run() {
        while (!should_exit_) {
            // Process all pending tasks
            while (auto task = dequeue_task()) {
                task->run();
            }
            // Park until woken or exit
            if (!should_exit_) {
                park();
            }
        }
    }

    void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> task) override {
        enqueue_task(task);
        unpark();  // Wake up the event loop
    }
};
```

---

## 6. Migration Checklist for Suspend Functions

When implementing a new suspend function:

- [ ] Declare with `[[suspend]]` attribute and `Continuation<void*>*` last parameter
- [ ] Use `suspend_cancellable_coroutine` for cancellation-aware suspension
- [ ] Check `is_coroutine_suspended()` result when calling other suspend functions
- [ ] Return `COROUTINE_SUSPENDED` when actually suspending
- [ ] Return actual value (or `nullptr` for Unit) when completing immediately
- [ ] Register with dispatcher for resumption scheduling
- [ ] Handle cancellation via `invoke_on_cancellation`
- [ ] Test in blocking context via `run_blocking` with working event loop
