# Research Note: Kotlin Compiler IR Lowering & Cancellable

## User Question
"What is Kotlin's compiler doing (kotlinc) in tmp/kotlin? ... Specifically with the IR bits for Cancellable"

## Findings

### 1. Compiler Lowering (The "IR Bits")
Source: `tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt`

The Kotlin/Native compiler uses a "Lowering" phase to transform high-level `suspend` lambdas into low-level state machine classes.
- **Transformation**: Every `suspend` lambda is converted into a class that inherits from `BaseContinuationImpl`.
- **`invokeSuspend`**: The code inside the lambda is moved to the `invokeSuspend` method of this class.
- **The Critical Hook (`create`)**: The compiler generates a `create(completion)` method for these classes. This method is responsible for creating a **fresh instance** of the coroutine state machine. This is the specific "IR bit" that enables `Cancellable` start logic.

### 2. Runtime Intrinsic (The Glue)
Source: `tmp/kotlin/kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt`

 The fundamental intrinsic `createCoroutineUnintercepted` connects the runtime to the compiler-generated code:

```kotlin
public actual fun <T> (suspend () -> T).createCoroutineUnintercepted(completion: Continuation<T>): Continuation<Unit> {
    val probeCompletion = probeCoroutineCreated(completion)
    // Check if `this` is a compiler-generated class
    return if (this is BaseContinuationImpl)
        create(probeCompletion) // Call the generated hook
    else
        // Fallback for generic suspend functions (like C++ std::function wrappers)
        createCoroutineFromSuspendFunction(probeCompletion) { ... }
}
```

### 3. Execution Flow for `startCoroutineCancellable`

1.  **`startCoroutineCancellable`** calls `createCoroutineUnintercepted`.
2.  **`createCoroutineUnintercepted`** uses the compiler-generated `create()` to get a new state machine instance.
3.  **`intercepted()`** wraps this instance in a `DispatchedContinuation` (if a dispatcher is involved).
4.  **`resumeCancellableWith`** is called on the wrapper, pushing the task to the dispatcher (e.g., `Dispatchers.Default`).

## Relevance to C++ Port

In `Cancellable.hpp`, we simulate this entire pipeline:
- **Compiler Replacement**: We use `LambdaContinuation<T>` as a generic substitute for the compiler-generated `BaseContinuationImpl` classes.
- **Intrinsic Replacement**: Our `create_coroutine_unintercepted` manually implements the logic of `IntrinsicsNative.kt`, instantiating our generic wrapper to hold the user's C++ lambda.
