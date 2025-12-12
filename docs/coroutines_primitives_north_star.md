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
