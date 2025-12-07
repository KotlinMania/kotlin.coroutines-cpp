// Transliterated from Kotlin to C++ - kotlinx.coroutines.test.TestBuilders
// Original package: kotlinx.coroutines.test
//
// TODO: Kotlin @file annotations (@JvmName, @JvmMultifileClass) not directly translatable
// TODO: Import statements removed; fully qualify types or add appropriate includes
// TODO: suspend functions translated as normal functions; coroutine semantics NOT implemented
// TODO: expect/actual mechanism needs platform-specific implementation strategy
// TODO: Kotlin property delegation, inline classes, and operator overloads may need manual review
// TODO: Annotations like @Deprecated preserved as comments

#include <functional>
#include <memory>
#include <string>
#include <optional>
#include <stdexcept>
#include <atomic>
#include <chrono>

namespace kotlinx {
namespace coroutines {
namespace test {

// @file:JvmName("TestBuildersKt")
// @file:JvmMultifileClass

// import kotlinx.atomicfu.atomic
// import kotlinx.coroutines.*
// import kotlinx.coroutines.selects.*
// import kotlin.coroutines.*
// import kotlin.jvm.*
// import kotlin.time.*
// import kotlin.time.Duration.Companion.milliseconds
// import kotlin.time.Duration.Companion.seconds

/**
 * A test result.
 *
 * - On JVM and Native, this resolves to [Unit], representing the fact that tests are run in a blocking manner on these
 *   platforms: a call to a function returning a [TestResult] will simply execute the test inside it.
 * - On JS, this is a `Promise`, which reflects the fact that the test-running function does not wait for a test to
 *   finish. The JS test frameworks typically support returning `Promise` from a test and will correctly handle it.
 *
 * Because of the behavior on JS, extra care must be taken when writing multiplatform tests to avoid losing test errors:
 * - Don't do anything after running the functions returning a [TestResult]. On JS, this code will execute *before* the
 *   test finishes.
 * - As a corollary, don't run functions returning a [TestResult] more than once per test. The only valid thing to do
 *   with a [TestResult] is to immediately `return` it from a test.
 * - Don't nest functions returning a [TestResult].
 */
// TODO: expect class - needs platform-specific implementation
class TestResult; // expect declaration

/**
 * Executes [testBody] as a test in a new coroutine, returning [TestResult].
 *
 * On JVM and Native, this function behaves similarly to `runBlocking`, with the difference that the code that it runs
 * will skip delays. This allows to use [delay] in tests without causing them to take more time than necessary.
 * On JS, this function creates a `Promise` that executes the test body with the delay-skipping behavior.
 *
 * ```
 * @Test
 * fun exampleTest() = runTest {
 *     val deferred = async {
 *         delay(1.seconds)
 *         async {
 *             delay(1.seconds)
 *         }.await()
 *     }
 *
 *     deferred.await() // result available immediately
 * }
 * ```
 *
 * The platform difference entails that, in order to use this function correctly in common code, one must always
 * immediately return the produced [TestResult] from the test method, without doing anything else afterwards. See
 * [TestResult] for details on this.
 *
 * The test is run on a single thread, unless other [CoroutineDispatcher] are used for child coroutines.
 * Because of this, child coroutines are not executed in parallel to the test body.
 * In order for the spawned-off asynchronous code to actually be executed, one must either [yield] or suspend the
 * test body some other way, or use commands that control scheduling (see [TestCoroutineScheduler]).
 *
 * ```
 * @Test
 * fun exampleWaitingForAsyncTasks1() = runTest {
 *     // 1
 *     val job = launch {
 *         // 3
 *     }
 *     // 2
 *     job.join() // the main test coroutine suspends here, so the child is executed
 *     // 4
 * }
 *
 * @Test
 * fun exampleWaitingForAsyncTasks2() = runTest {
 *     // 1
 *     launch {
 *         // 3
 *     }
 *     // 2
 *     testScheduler.advanceUntilIdle() // runs the tasks until their queue is empty
 *     // 4
 * }
 * ```
 *
 * ### Task scheduling
 *
 * Delay skipping is achieved by using virtual time.
 * If [Dispatchers.Main] is set to a [TestDispatcher] via [Dispatchers.setMain] before the test,
 * then its [TestCoroutineScheduler] is used;
 * otherwise, a new one is automatically created (or taken from [context] in some way) and can be used to control
 * the virtual time, advancing it, running the tasks scheduled at a specific time etc.
 * The scheduler can be accessed via [TestScope.testScheduler].
 *
 * Delays in code that runs inside dispatchers that don't use a [TestCoroutineScheduler] don't get skipped:
 * ```
 * @Test
 * fun exampleTest() = runTest {
 *     val elapsed = TimeSource.Monotonic.measureTime {
 *         val deferred = async {
 *             delay(1.seconds) // will be skipped
 *             withContext(Dispatchers.Default) {
 *                 delay(5.seconds) // Dispatchers.Default doesn't know about TestCoroutineScheduler
 *             }
 *         }
 *         deferred.await()
 *     }
 *     println(elapsed) // about five seconds
 * }
 * ```
 *
 * ### Failures
 *
 * #### Test body failures
 *
 * If the created coroutine completes with an exception, then this exception will be thrown at the end of the test.
 *
 * #### Timing out
 *
 * There's a built-in timeout of 60 seconds for the test body. If the test body doesn't complete within this time,
 * then the test fails with an [AssertionError]. The timeout can be changed for each test separately by setting the
 * [timeout] parameter.
 *
 * Additionally, setting the `kotlinx.coroutines.test.default_timeout` system property on the
 * JVM to any string that can be parsed using [Duration.parse] (like `1m`, `30s` or `1500ms`) will change the default
 * timeout to that value for all tests whose [timeout] is not set explicitly; setting it to anything else will throw an
 * exception every time [runTest] is invoked.
 *
 * On timeout, the test body is cancelled so that the test finishes. If the code inside the test body does not
 * respond to cancellation, the timeout will not be able to make the test execution stop.
 * In that case, the test will hang despite the attempt to terminate it.
 *
 * On the JVM, if `DebugProbes` from the `kotlinx-coroutines-debug` module are installed, the current dump of the
 * coroutines' stack is printed to the console on timeout before the test body is cancelled.
 *
 * #### Reported exceptions
 *
 * Unhandled exceptions will be thrown at the end of the test.
 * If uncaught exceptions happen after the test finishes, they are propagated in a platform-specific manner:
 * see [handleCoroutineException] for details.
 * If the test coroutine completes with an exception, the unhandled exceptions are suppressed by it.
 *
 * #### Uncompleted coroutines
 *
 * Otherwise, the test will hang until all the coroutines launched inside [testBody] complete.
 * This may be an issue when there are some coroutines that are not supposed to complete, like infinite loops that
 * perform some background work and are supposed to outlive the test.
 * In that case, [TestScope.backgroundScope] can be used to launch such coroutines.
 * They will be cancelled automatically when the test finishes.
 *
 * ### Configuration
 *
 * [context] can be used to affect the environment of the code under test. Beside just being passed to the coroutine
 * scope created for the test, [context] also can be used to change how the test is executed.
 * See the [TestScope] constructor function documentation for details.
 *
 * @throws IllegalArgumentException if the [context] is invalid. See the [TestScope] constructor docs for details.
 */
// TODO: suspend function - coroutine semantics not implemented
TestResult run_test(
    CoroutineContext context = empty_coroutine_context(),
    Duration timeout = kDefaultTimeout.get_or_throw(),
    std::function<void(TestScope&)> test_body
) {
    // TODO: check() is Kotlin's require-like function; map to assertion or exception
    if (context[running_in_run_test] != nullptr) {
        throw std::invalid_argument("Calls to `runTest` can't be nested. Please read the docs on `TestResult` for details.");
    }
    return TestScope(context + running_in_run_test).run_test(timeout, test_body);
}

/**
 * Executes [testBody] as a test in a new coroutine, returning [TestResult].
 *
 * On JVM and Native, this function behaves similarly to `runBlocking`, with the difference that the code that it runs
 * will skip delays. This allows to use [delay] in without causing the tests to take more time than necessary.
 * On JS, this function creates a `Promise` that executes the test body with the delay-skipping behavior.
 *
 * ```
 * @Test
 * fun exampleTest() = runTest {
 *     val deferred = async {
 *         delay(1.seconds)
 *         async {
 *             delay(1.seconds)
 *         }.await()
 *     }
 *
 *     deferred.await() // result available immediately
 * }
 * ```
 *
 * The platform difference entails that, in order to use this function correctly in common code, one must always
 * immediately return the produced [TestResult] from the test method, without doing anything else afterwards. See
 * [TestResult] for details on this.
 *
 * The test is run in a single thread, unless other [CoroutineDispatcher] are used for child coroutines.
 * Because of this, child coroutines are not executed in parallel to the test body.
 * In order for the spawned-off asynchronous code to actually be executed, one must either [yield] or suspend the
 * test body some other way, or use commands that control scheduling (see [TestCoroutineScheduler]).
 *
 * ```
 * @Test
 * fun exampleWaitingForAsyncTasks1() = runTest {
 *     // 1
 *     val job = launch {
 *         // 3
 *     }
 *     // 2
 *     job.join() // the main test coroutine suspends here, so the child is executed
 *     // 4
 * }
 *
 * @Test
 * fun exampleWaitingForAsyncTasks2() = runTest {
 *     // 1
 *     launch {
 *         // 3
 *     }
 *     // 2
 *     advanceUntilIdle() // runs the tasks until their queue is empty
 *     // 4
 * }
 * ```
 *
 * ### Task scheduling
 *
 * Delay-skipping is achieved by using virtual time.
 * If [Dispatchers.Main] is set to a [TestDispatcher] via [Dispatchers.setMain] before the test,
 * then its [TestCoroutineScheduler] is used;
 * otherwise, a new one is automatically created (or taken from [context] in some way) and can be used to control
 * the virtual time, advancing it, running the tasks scheduled at a specific time etc.
 * Some convenience methods are available on [TestScope] to control the scheduler.
 *
 * Delays in code that runs inside dispatchers that don't use a [TestCoroutineScheduler] don't get skipped:
 * ```
 * @Test
 * fun exampleTest() = runTest {
 *     val elapsed = TimeSource.Monotonic.measureTime {
 *         val deferred = async {
 *             delay(1.seconds) // will be skipped
 *             withContext(Dispatchers.Default) {
 *                 delay(5.seconds) // Dispatchers.Default doesn't know about TestCoroutineScheduler
 *             }
 *         }
 *         deferred.await()
 *     }
 *     println(elapsed) // about five seconds
 * }
 * ```
 *
 * ### Failures
 *
 * #### Test body failures
 *
 * If the created coroutine completes with an exception, then this exception will be thrown at the end of the test.
 *
 * #### Reported exceptions
 *
 * Unhandled exceptions will be thrown at the end of the test.
 * If the uncaught exceptions happen after the test finishes, the error is propagated in a platform-specific manner.
 * If the test coroutine completes with an exception, the unhandled exceptions are suppressed by it.
 *
 * #### Uncompleted coroutines
 *
 * This method requires that, after the test coroutine has completed, all the other coroutines launched inside
 * [testBody] also complete, or are cancelled.
 * Otherwise, the test will be failed (which, on JVM and Native, means that [runTest] itself will throw
 * [AssertionError], whereas on JS, the `Promise` will fail with it).
 *
 * In the general case, if there are active jobs, it's impossible to detect if they are going to complete eventually due
 * to the asynchronous nature of coroutines. In order to prevent tests hanging in this scenario, [runTest] will wait
 * for [dispatchTimeoutMs] from the moment when [TestCoroutineScheduler] becomes
 * idle before throwing [AssertionError]. If some dispatcher linked to [TestCoroutineScheduler] receives a
 * task during that time, the timer gets reset.
 *
 * ### Configuration
 *
 * [context] can be used to affect the environment of the code under test. Beside just being passed to the coroutine
 * scope created for the test, [context] also can be used to change how the test is executed.
 * See the [TestScope] constructor function documentation for details.
 *
 * @throws IllegalArgumentException if the [context] is invalid. See the [TestScope] constructor docs for details.
 */
// @Deprecated(
//     "Define a total timeout for the whole test instead of using dispatchTimeoutMs. " +
//         "Warning: the proposed replacement is not identical as it uses 'dispatchTimeoutMs' as the timeout for the whole test!",
//     ReplaceWith("runTest(context, timeout = dispatchTimeoutMs.milliseconds, testBody)",
//         "kotlin.time.Duration.Companion.milliseconds"),
//     DeprecationLevel.WARNING
// ) // Warning since 1.7.0, was experimental in 1.6.x
// TODO: Deprecated - mark for removal or document
// TODO: suspend function - coroutine semantics not implemented
[[deprecated("Define a total timeout for the whole test instead of using dispatchTimeoutMs")]]
TestResult run_test(
    CoroutineContext context = empty_coroutine_context(),
    int64_t dispatch_timeout_ms,
    std::function<void(TestScope&)> test_body
) {
    if (context[running_in_run_test] != nullptr)
        throw std::invalid_argument("Calls to `runTest` can't be nested. Please read the docs on `TestResult` for details.");
    // @Suppress("DEPRECATION")
    return TestScope(context + running_in_run_test).run_test(dispatch_timeout_ms, test_body);
}

/**
 * Performs [runTest] on an existing [TestScope]. See the documentation for [runTest] for details.
 */
// TODO: suspend function - coroutine semantics not implemented
TestResult run_test(
    TestScope& scope,
    Duration timeout = kDefaultTimeout.get_or_throw(),
    std::function<void(TestScope&)> test_body
) {
    auto& scope_impl = scope.as_specific_implementation();
    scope_impl.enter();
    return create_test_result([&]() {
        AtomicBoolean test_body_finished(false);
        /** TODO: moving this [AbstractCoroutine.start] call outside [createTestResult] fails on JS. */
        scope_impl.start(CoroutineStart::kUndispatched, scope_impl, [&]() {
            /* we're using `UNDISPATCHED` to avoid the event loop, but we do want to set up the timeout machinery
            before any code executes, so we have to park here. */
            yield();
            try {
                test_body(scope_impl);
            } catch (...) {
                test_body_finished.value = true;
                throw;
            }
            test_body_finished.value = true;
        });
        Throwable* timeout_error = nullptr;
        CancellationException* cancellation_exception = nullptr;
        auto work_runner = launch(CoroutineName("kotlinx.coroutines.test runner"), [&]() {
            while (true) {
                bool executed_something = scope_impl.test_scheduler().try_run_next_task_unless([&]() { return !is_active(); });
                if (executed_something) {
                    /** yield to check for cancellation. On JS, we can't use [ensureActive] here, as the cancellation
                     * procedure needs a chance to run concurrently. */
                    yield();
                } else {
                    // waiting for the next task to be scheduled, or for the test runner to be cancelled
                    scope_impl.test_scheduler().receive_dispatch_event();
                }
            }
        });
        try {
            with_timeout(timeout, [&]() {
                coroutine_context().job().invoke_on_completion(true, [&](std::exception_ptr exception) {
                    // TODO: Kotlin exception handling differs from C++; needs adaptation
                    if (/* exception is TimeoutCancellationException */) {
                        dump_coroutines();
                        auto active_children = scope_impl.children().filter([](Job& j) { return j.is_active(); }).to_list();
                        std::string message = "After waiting for " + std::to_string(timeout.count()) + ", ";
                        if (test_body_finished.value && !active_children.empty()) {
                            message += "there were active child jobs: " /* + activeChildren */ +
                                ". Use `TestScope.backgroundScope` " +
                                "to launch the coroutines that need to be cancelled when the test body finishes";
                        } else if (test_body_finished.value) {
                            message += "the test completed, but only after the timeout";
                        } else {
                            message += "the test body did not run to completion";
                        }
                        timeout_error = new UncompletedCoroutinesError(message);
                        cancellation_exception = new CancellationException("The test timed out");
                        // (scope as Job).cancel(cancellationException!!)
                        static_cast<Job&>(scope_impl).cancel(*cancellation_exception);
                    }
                });
                scope_impl.join();
                work_runner.cancel_and_join();
            });
        } catch (const TimeoutCancellationException&) {
            scope_impl.join();
            auto completion = scope_impl.get_completion_exception_or_null();
            if (completion != nullptr && completion != cancellation_exception) {
                timeout_error->add_suppressed(*completion);
            }
            work_runner.cancel_and_join();
        } catch (...) {
            scope_impl.background_scope().cancel();
            scope_impl.test_scheduler().advance_until_idle_or([]() { return false; });
            auto uncaught_exceptions = scope_impl.leave();
            throw_all(timeout_error != nullptr ? timeout_error : scope_impl.get_completion_exception_or_null(), uncaught_exceptions);
            throw;
        }
        scope_impl.background_scope().cancel();
        scope_impl.test_scheduler().advance_until_idle_or([]() { return false; });
        auto uncaught_exceptions = scope_impl.leave();
        throw_all(timeout_error != nullptr ? timeout_error : scope_impl.get_completion_exception_or_null(), uncaught_exceptions);
    });
}

/**
 * Performs [runTest] on an existing [TestScope].
 *
 * In the general case, if there are active jobs, it's impossible to detect if they are going to complete eventually due
 * to the asynchronous nature of coroutines. In order to prevent tests hanging in this scenario, [runTest] will wait
 * for [dispatchTimeoutMs] from the moment when [TestCoroutineScheduler] becomes
 * idle before throwing [AssertionError]. If some dispatcher linked to [TestCoroutineScheduler] receives a
 * task during that time, the timer gets reset.
 */
// @Deprecated(
//     "Define a total timeout for the whole test instead of using dispatchTimeoutMs. " +
//         "Warning: the proposed replacement is not identical as it uses 'dispatchTimeoutMs' as the timeout for the whole test!",
//     ReplaceWith("this.runTest(timeout = dispatchTimeoutMs.milliseconds, testBody)",
//         "kotlin.time.Duration.Companion.milliseconds"),
//     DeprecationLevel.WARNING
// ) // Warning since 1.7.0, was experimental in 1.6.x
// TODO: Deprecated - mark for removal or document
// TODO: suspend function - coroutine semantics not implemented
[[deprecated("Define a total timeout for the whole test instead of using dispatchTimeoutMs")]]
TestResult run_test(
    TestScope& scope,
    int64_t dispatch_timeout_ms,
    std::function<void(TestScope&)> test_body
) {
    auto& scope_impl = scope.as_specific_implementation();
    scope_impl.enter();
    // @Suppress("DEPRECATION")
    return create_test_result([&]() {
        run_test_coroutine_legacy(scope_impl, std::chrono::milliseconds(dispatch_timeout_ms),
            &TestScopeImpl::try_get_completion_cause, test_body, [&]() {
                scope_impl.background_scope().cancel();
                scope_impl.test_scheduler().advance_until_idle_or([]() { return false; });
                return scope_impl.legacy_leave();
            });
    });
}

/**
 * Runs [testProcedure], creating a [TestResult].
 */
// TODO: expect function - needs platform-specific implementation
TestResult create_test_result(std::function<void(CoroutineScope&)> test_procedure);

/** A coroutine context element indicating that the coroutine is running inside `runTest`. */
// TODO: object singleton pattern
class RunningInRunTest : public CoroutineContext::Key<RunningInRunTest>, public CoroutineContext::Element {
public:
    CoroutineContext::Key<RunningInRunTest>* key() const override {
        // TODO: Kotlin object self-reference pattern
        return const_cast<RunningInRunTest*>(this);
    }

    std::string to_string() const override {
        return "RunningInRunTest";
    }
};

// Singleton instance
inline RunningInRunTest running_in_run_test;

/** The default timeout to use when waiting for asynchronous completions of the coroutines managed by
 * a [TestCoroutineScheduler]. */
constexpr int64_t kDefaultDispatchTimeoutMs = 60000L;

/**
 * The default timeout to use when running a test.
 *
 * It's not just a [Duration] but a [Result] so that every access to [runTest]
 * throws the same clear exception if parsing the environment variable failed.
 * Otherwise, the parsing error would only be thrown in one tests, while the
 * other ones would get an incomprehensible `NoClassDefFoundError`.
 */
// TODO: Kotlin Result type needs translation; using std::optional or custom Result type
// TODO: runCatching lambda needs translation
// private val DEFAULT_TIMEOUT: Result<Duration> = runCatching {
//     systemProperty("kotlinx.coroutines.test.default_timeout", Duration::parse, 60.seconds)
// }
// TODO: Implement result-based default timeout initialization
Result<Duration> kDefaultTimeout; // placeholder

/**
 * Run the [body][testBody] of the [test coroutine][coroutine], waiting for asynchronous completions for at most
 * [dispatchTimeout] and performing the [cleanup] procedure at the end.
 *
 * [tryGetCompletionCause] is the [JobSupport.completionCause], which is passed explicitly because it is protected.
 *
 * The [cleanup] procedure may either throw [UncompletedCoroutinesError] to denote that child coroutines were leaked, or
 * return a list of uncaught exceptions that should be reported at the end of the test.
 */
// @Deprecated("Used for support of legacy behavior")
// TODO: Deprecated - mark for removal or document
// TODO: suspend function - coroutine semantics not implemented
template<typename T>
[[deprecated("Used for support of legacy behavior")]]
void run_test_coroutine_legacy(
    CoroutineScope& scope,
    T& coroutine,
    Duration dispatch_timeout,
    std::function<Throwable*(T&)> try_get_completion_cause,
    std::function<void(T&)> test_body,
    std::function<std::vector<Throwable*>()> cleanup
) {
    auto scheduler = coroutine.coroutine_context()[TestCoroutineScheduler::Key];
    /** TODO: moving this [AbstractCoroutine.start] call outside [createTestResult] fails on JS. */
    coroutine.start(CoroutineStart::kUndispatched, coroutine, [&]() {
        test_body(coroutine);
    });
    /**
     * This is the legacy behavior, kept for now for compatibility only.
     *
     * The general procedure here is as follows:
     * 1. Try running the work that the scheduler knows about, both background and foreground.
     *
     * 2. Wait until we run out of foreground work to do. This could mean one of the following:
     *    - The main coroutine is already completed. This is checked separately; then we leave the procedure.
     *    - It's switched to another dispatcher that doesn't know about the [TestCoroutineScheduler].
     *    - Generally, it's waiting for something external (like a network request, or just an arbitrary callback).
     *    - The test simply hanged.
     *    - The main coroutine is waiting for some background work.
     *
     * 3. We await progress from things that are not the code under test:
     *    the background work that the scheduler knows about, the external callbacks,
     *    the work on dispatchers not linked to the scheduler, etc.
     *
     *    When we observe that the code under test can proceed, we go to step 1 again.
     *    If there is no activity for [dispatchTimeoutMs] milliseconds, we consider the test to have hanged.
     *
     *    The background work is not running on a dedicated thread.
     *    Instead, the test thread itself is used, by spawning a separate coroutine.
     */
    bool completed = false;
    while (!completed) {
        scheduler->advance_until_idle();
        if (coroutine.is_completed()) {
            /* don't even enter `withTimeout`; this allows to use a timeout of zero to check that there are no
           non-trivial dispatches. */
            completed = true;
            continue;
        }
        // in case progress depends on some background work, we need to keep spinning it.
        auto background_work_runner = launch(CoroutineName("background work runner"), [&]() {
            while (true) {
                bool executed_something = scheduler->try_run_next_task_unless([&]() { return !is_active(); });
                if (executed_something) {
                    // yield so that the `select` below has a chance to finish successfully or time out
                    yield();
                } else {
                    // no more tasks, we should suspend until there are some more.
                    // this doesn't interfere with the `select` below, because different channels are used.
                    scheduler->receive_dispatch_event();
                }
            }
        });
        try {
            // TODO: select expression needs translation
            select<void>([&](auto& selector) {
                selector.on_join(coroutine, [&]() {
                    // observe that someone completed the test coroutine and leave without waiting for the timeout
                    completed = true;
                });
                selector.on_dispatch_event_foreground(*scheduler, [&]() {
                    // we received knowledge that `scheduler` observed a dispatch event, so we reset the timeout
                });
                selector.on_timeout(dispatch_timeout, [&]() {
                    throw handle_timeout(coroutine, dispatch_timeout, try_get_completion_cause, cleanup);
                });
            });
        } catch (...) {
            background_work_runner.cancel_and_join();
            throw;
        }
        background_work_runner.cancel_and_join();
    }
    if (auto exception = coroutine.get_completion_exception_or_null()) {
        std::vector<Throwable*> exceptions;
        try {
            exceptions = cleanup();
        } catch (const UncompletedCoroutinesError&) {
            // it's normal that some jobs are not completed if the test body has failed, won't clutter the output
            exceptions = {};
        }
        throw_all(exception, exceptions);
    }
    throw_all(nullptr, cleanup());
}

/**
 * Invoked on timeout in [runTest]. Just builds a nice [UncompletedCoroutinesError] and returns it.
 */
template<typename T>
AssertionError handle_timeout(
    T& coroutine,
    Duration dispatch_timeout,
    std::function<Throwable*(T&)> try_get_completion_cause,
    std::function<std::vector<Throwable*>()> cleanup
) {
    std::vector<Throwable*> uncaught_exceptions;
    try {
        uncaught_exceptions = cleanup();
    } catch (const UncompletedCoroutinesError&) {
        // we expect these and will instead throw a more informative exception.
        uncaught_exceptions = {};
    }
    auto active_children = coroutine.children().filter([](auto& it) { return it.is_active(); }).to_list();
    Throwable* completion_cause = coroutine.is_cancelled() ? try_get_completion_cause(coroutine) : nullptr;
    std::string message = "After waiting for " + std::to_string(dispatch_timeout.count());
    if (completion_cause == nullptr)
        message += ", the test coroutine is not completing";
    if (!active_children.empty())
        message += ", there were active child jobs: " /* + activeChildren */;
    if (completion_cause != nullptr && active_children.empty()) {
        message += coroutine.is_completed() ?
            ", the test coroutine completed" :
            ", the test coroutine was not completed";
    }
    AssertionError error(message);
    if (completion_cause) {
        error.add_suppressed(*completion_cause);
    }
    for (auto* e : uncaught_exceptions) {
        error.add_suppressed(*e);
    }
    return error;
}

void throw_all(Throwable* head, const std::vector<Throwable*>& other) {
    if (head != nullptr) {
        for (auto* t : other) {
            head->add_suppressed(*t);
        }
        throw *head;
    } else {
        if (!other.empty()) {
            auto* first = other[0];
            for (size_t i = 1; i < other.size(); ++i) {
                first->add_suppressed(*other[i]);
            }
            throw *first;
        }
    }
}

// TODO: expect function - needs platform-specific implementation
void dump_coroutines();

template<typename T>
T system_property(
    const std::string& name,
    std::function<T(const std::string&)> parse,
    T default_value
) {
    auto value = system_property_impl(name);
    if (!value) return default_value;
    return parse(*value);
}

// TODO: expect function - needs platform-specific implementation
std::optional<std::string> system_property_impl(const std::string& name);

// @Deprecated(
//     "This is for binary compatibility with the `runTest` overload that existed at some point",
//     level = DeprecationLevel.HIDDEN
// )
// @JvmName("runTest\$default")
// @Suppress("DEPRECATION", "UNUSED_PARAMETER")
// TODO: Deprecated and hidden - for binary compatibility only
// TODO: JvmName annotation not applicable in C++
[[deprecated("This is for binary compatibility")]]
TestResult run_test_legacy(
    TestScope& scope,
    int64_t dispatch_timeout_ms,
    std::function<void(TestScope&)> test_body,
    int marker,
    void* unused2
) {
    return scope.run_test(
        (marker & 1) != 0 ? dispatch_timeout_ms : 60000L,
        test_body
    );
}

// Remove after https://youtrack.jetbrains.com/issue/KT-62423/
class AtomicBoolean {
private:
    std::atomic<bool> container;
public:
    bool value;

    explicit AtomicBoolean(bool initial) : container(initial), value(initial) {}

    // TODO: Kotlin property with getter/setter needs manual synchronization
    // For now, direct access to value field (not thread-safe as in Kotlin version)
};

} // namespace test
} // namespace coroutines
} // namespace kotlinx
