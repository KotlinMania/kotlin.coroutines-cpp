#pragma once

namespace kotlinx {
namespace coroutines {

/**
 * Defines start options for coroutines builders.
 *
 * It is used in the `start` parameter of coroutine builder functions like
 * [launch][CoroutineScope.launch] and [async][CoroutineScope.async]
 * to describe when and how the coroutine should be dispatched initially.
 *
 * This parameter only affects how the coroutine behaves until the code of its body starts executing.
 * After that, cancellability and dispatching are defined by the behavior of the invoked suspending functions.
 *
 * The summary of coroutine start options is:
 * - [DEFAULT] immediately schedules the coroutine for execution according to its context.
 * - [LAZY] delays the moment of the initial dispatch until the result of the coroutine is needed.
 * - [ATOMIC] prevents the coroutine from being cancelled before it starts, ensuring that its code will start
 *   executing in any case.
 * - [UNDISPATCHED] immediately executes the coroutine until its first suspension point _in the current thread_.
 */
enum class CoroutineStart {
    /**
     * Immediately schedules the coroutine for execution according to its context. This is usually the default option.
     *
     * [DEFAULT] uses the default dispatch procedure described in the [CoroutineDispatcher] documentation.
     *
     * If the coroutine's [Job] is cancelled before it started executing, then it will not start its
     * execution at all and will be considered [cancelled][Job.isCancelled].
     *
     * Examples:
     *
     * ```
     * // Example of starting a new coroutine that goes through a dispatch
     * runBlocking {
     *     println("1. About to start a new coroutine.")
     *     // Dispatch the job to execute later.
     *     // The parent coroutine's dispatcher is inherited by default.
     *     // In this case, it's the single thread backing `runBlocking`.
     *     launch { // CoroutineStart.DEFAULT is launch's default start mode
     *         println("3. When the thread is available, we start the coroutine")
     *     }
     *     println("2. The thread keeps doing other work after launching the coroutine")
     * }
     * ```
     *
     * ```
     * // Example of starting a new coroutine that doesn't go through a dispatch initially
     * runBlocking {
     *     println("1. About to start a coroutine not needing a dispatch.")
     *     // Dispatch the job to execute.
     *     // `Dispatchers.Unconfined` is explicitly chosen.
     *     launch(Dispatchers.Unconfined) { // CoroutineStart.DEFAULT is the launch's default start mode
     *         println("2. The body will be executed immediately")
     *         delay(50.milliseconds) // give up the thread to the outer coroutine
     *         println("4. When the thread is next available, this coroutine proceeds further")
     *     }
     *     println("3. After the initial suspension, the thread does other work.")
     * }
     * ```
     *
     * ```
     * // Example of cancelling coroutines before they start executing.
     * runBlocking {
     *     // dispatch the job to execute on this thread later
     *     launch { // CoroutineStart.DEFAULT is the launch's default start mode
     *         println("This code will never execute")
     *     }
     *     cancel() // cancels the current coroutine scope and its children
     *     launch(Dispatchers.Unconfined) {
     *         println("This code will never execute")
     *     }
     *     println("This code will execute.")
     * }
     * ```
     */
    DEFAULT,

    /**
     * Starts the coroutine lazily, only when it is needed.
     *
     * Starting a coroutine with [LAZY] only creates the coroutine, but does not schedule it for execution.
     * When the completion of the coroutine is first awaited
     * (for example, via [Job.join]) or explicitly [started][Job.start],
     * the dispatch procedure described in the [CoroutineDispatcher] documentation is performed in the thread
     * that did it.
     *
     * The details of what counts as waiting can be found in the documentation of the corresponding coroutine builders
     * like [launch][CoroutineScope.launch] and [async][CoroutineScope.async].
     *
     * If the coroutine's [Job] is cancelled before it started executing, then it will not start its
     * execution at all and will be considered [cancelled][Job.isCancelled].
     *
     * **Pitfall**: launching a coroutine with [LAZY] without awaiting or cancelling it at any point means that it will
     * never be completed, leading to deadlocks and resource leaks.
     * For example, the following code will deadlock, since [coroutineScope] waits for all of its child coroutines to
     * complete:
     * ```
     * // This code hangs!
     * coroutineScope {
     *     launch(start = CoroutineStart.LAZY) { }
     * }
     * ```
     *
     * The behavior of [LAZY] can be described with the following examples:
     *
     * ```
     * // Example of lazily starting a new coroutine that goes through a dispatch
     * runBlocking {
     *     println("1. About to start a new coroutine.")
     *     // Create a job to execute on `Dispatchers.Default` later.
     *     auto job = launch(Dispatchers.Default, start = CoroutineStart.LAZY) {
     *         println("3. Only now does the coroutine start.")
     *     }
     *     delay(10.milliseconds) // try to give the coroutine some time to run
     *     println("2. The coroutine still has not started. Now, we join it.")
     *     job.join()
     * }
     * ```
     *
     * ```
     * // Example of lazily starting a new coroutine that doesn't go through a dispatch initially
     * runBlocking {
     *     println("1. About to lazily start a new coroutine.")
     *     // Create a job to execute on `Dispatchers.Unconfined` later.
     *     auto lazyJob = launch(Dispatchers.Unconfined, start = CoroutineStart.LAZY) {
     *         println("3. The coroutine starts on the thread that called `join`.")
     *     }
     *     // We start the job on another thread for illustrative purposes
     *     launch(Dispatchers.Default) {
     *         println("2. We start the lazyJob.")
     *         job.start() // runs lazyJob's code in-place
     *         println("4. Only now does the `start` call return.")
     *     }
     * }
     * ```
     *
     * ## Alternatives
     *
     * The effects of [LAZY] can usually be achieved more idiomatically without it.
     *
     * When a coroutine is started with [LAZY] and is stored in a property,
     * it may be a better choice to use [lazy] instead:
     *
     * ```
     * // instead of `auto page = scope.async(start = CoroutineStart.LAZY) { getPage() }`, do
     * auto page by lazy { scope.async { getPage() } }
     * ```
     *
     * This way, the child coroutine is not created at all unless it is needed.
     * Note that with this, any access to this variable will start the coroutine,
     * even something like `page.invokeOnCompletion { }` or `page.isActive`.
     *
     * If a coroutine is started with [LAZY] and then unconditionally started,
     * it is more idiomatic to create the coroutine in the exact place where it is started:
     *
     * ```
     * // instead of `auto job = scope.launch(start = CoroutineStart.LAZY) { }; job.start()`, do
     * scope.launch { }
     * ```
     */
    LAZY,

    /**
     * Atomically (i.e., in a non-cancellable way) schedules the coroutine for execution according to its context.
     */
     ATOMIC,

    /**
     * Immediately executes the coroutine until its first suspension point _in the current thread_.
     */
    UNDISPATCHED
};

/**
 * Returns true if the start option is LAZY.
 */
inline bool is_lazy(CoroutineStart start) {
    return start == CoroutineStart::LAZY;
}

} // namespace coroutines
} // namespace kotlinx
