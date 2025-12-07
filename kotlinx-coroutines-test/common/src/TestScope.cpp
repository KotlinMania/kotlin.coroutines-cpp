// Transliterated from Kotlin to C++ - kotlinx.coroutines.test.TestScope
// Original package: kotlinx.coroutines.test
//
// TODO: Import statements removed; fully qualify types or add appropriate includes
// TODO: suspend functions translated as normal functions; coroutine semantics NOT implemented
// TODO: Kotlin sealed interface translated to abstract class
// TODO: Kotlin extension properties/functions translated to free functions or methods
// TODO: Annotations (@ExperimentalCoroutinesApi, @PublishedApi) preserved as comments
// TODO: Kotlin internal visibility needs C++ equivalent (namespace-level or conditional compilation)

#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <stdexcept>

namespace kotlinx {
namespace coroutines {
namespace test {

// package kotlinx.coroutines.test

// import kotlinx.coroutines.*
// import kotlinx.coroutines.internal.*
// import kotlinx.coroutines.test.internal.*
// import kotlin.coroutines.*
// import kotlin.time.*

/**
 * A coroutine scope that for launching test coroutines.
 *
 * The scope provides the following functionality:
 * - The [coroutineContext] includes a [coroutine dispatcher][TestDispatcher] that supports delay-skipping, using
 *   a [TestCoroutineScheduler] for orchestrating the virtual time.
 *   This scheduler is also available via the [testScheduler] property, and some helper extension
 *   methods are defined to more conveniently interact with it: see [TestScope.currentTime], [TestScope.runCurrent],
 *   [TestScope.advanceTimeBy], and [TestScope.advanceUntilIdle].
 * - When inside [runTest], uncaught exceptions from the child coroutines of this scope will be reported at the end of
 *   the test.
 *   It is invalid for child coroutines to throw uncaught exceptions when outside the call to [TestScope.runTest]:
 *   the only guarantee in this case is the best effort to deliver the exception.
 *
 * The usual way to access a [TestScope] is to call [runTest], but it can also be constructed manually, in order to
 * use it to initialize the components that participate in the test.
 *
 * #### Differences from the deprecated [TestCoroutineScope]
 *
 * - This doesn't provide an equivalent of [TestCoroutineScope.cleanupTestCoroutines], and so can't be used as a
 *   standalone mechanism for writing tests: it does require that [runTest] is eventually called.
 *   The reason for this is that a proper cleanup procedure that supports using non-test dispatchers and arbitrary
 *   coroutine suspensions would be equivalent to [runTest], but would also be more error-prone, due to the potential
 *   for forgetting to perform the cleanup.
 * - [TestCoroutineScope.advanceTimeBy] also calls [TestCoroutineScheduler.runCurrent] after advancing the virtual time.
 * - No support for dispatcher pausing, like [DelayController] allows. [TestCoroutineDispatcher], which supported
 *   pausing, is deprecated; now, instead of pausing a dispatcher, one can use [withContext] to run a dispatcher that's
 *   paused by default, like [StandardTestDispatcher].
 * - No access to the list of unhandled exceptions.
 */
// TODO: sealed interface in Kotlin; use abstract class in C++
class TestScope : public CoroutineScope {
public:
    /**
     * The delay-skipping scheduler used by the test dispatchers running the code in this scope.
     */
    virtual TestCoroutineScheduler& test_scheduler() = 0;

    /**
     * A scope for background work.
     *
     * This scope is automatically cancelled when the test finishes.
     * The coroutines in this scope are run as usual when using [advanceTimeBy] and [runCurrent].
     * [advanceUntilIdle], on the other hand, will stop advancing the virtual time once only the coroutines in this
     * scope are left unprocessed.
     *
     * Failures in coroutines in this scope do not terminate the test.
     * Instead, they are reported at the end of the test.
     * Likewise, failure in the [TestScope] itself will not affect its [backgroundScope],
     * because there's no parent-child relationship between them.
     *
     * A typical use case for this scope is to launch tasks that would outlive the tested code in
     * the production environment.
     *
     * In this example, the coroutine that continuously sends new elements to the channel will get
     * cancelled:
     * ```
     * @Test
     * fun testExampleBackgroundJob() = runTest {
     *     val channel = Channel<Int>()
     *     backgroundScope.launch {
     *         var i = 0
     *         while (true) {
     *             channel.send(i++)
     *         }
     *     }
     *     repeat(100) {
     *         assertEquals(it, channel.receive())
     *     }
     * }
     * ```
     */
    virtual CoroutineScope& background_scope() = 0;

    virtual ~TestScope() = default;
};

/**
 * The current virtual time on [testScheduler][TestScope.testScheduler].
 * @see TestCoroutineScheduler.currentTime
 */
// @ExperimentalCoroutinesApi
// TODO: Kotlin extension property; translate to free function
int64_t current_time(const TestScope& scope) {
    return scope.test_scheduler().current_time();
}

/**
 * Advances the [testScheduler][TestScope.testScheduler] to the point where there are no tasks remaining.
 * @see TestCoroutineScheduler.advanceUntilIdle
 */
// @ExperimentalCoroutinesApi
// TODO: Kotlin extension function; translate to free function
void advance_until_idle(TestScope& scope) {
    scope.test_scheduler().advance_until_idle();
}

/**
 * Run any tasks that are pending at the current virtual time, according to
 * the [testScheduler][TestScope.testScheduler].
 *
 * @see TestCoroutineScheduler.runCurrent
 */
// @ExperimentalCoroutinesApi
// TODO: Kotlin extension function; translate to free function
void run_current(TestScope& scope) {
    scope.test_scheduler().run_current();
}

/**
 * Moves the virtual clock of this dispatcher forward by [the specified amount][delayTimeMillis], running the
 * scheduled tasks in the meantime.
 *
 * In contrast with `TestCoroutineScope.advanceTimeBy`, this function does not run the tasks scheduled at the moment
 * [currentTime] + [delayTimeMillis].
 *
 * @throws IllegalStateException if passed a negative [delay][delayTimeMillis].
 * @see TestCoroutineScheduler.advanceTimeBy
 */
// @ExperimentalCoroutinesApi
// TODO: Kotlin extension function; translate to free function
void advance_time_by(TestScope& scope, int64_t delay_time_millis) {
    scope.test_scheduler().advance_time_by(delay_time_millis);
}

/**
 * Moves the virtual clock of this dispatcher forward by [the specified amount][delayTime], running the
 * scheduled tasks in the meantime.
 *
 * @throws IllegalStateException if passed a negative [delay][delayTime].
 * @see TestCoroutineScheduler.advanceTimeBy
 */
// @ExperimentalCoroutinesApi
// TODO: Kotlin extension function; translate to free function
void advance_time_by(TestScope& scope, Duration delay_time) {
    scope.test_scheduler().advance_time_by(delay_time);
}

/**
 * The [test scheduler][TestScope.testScheduler] as a [TimeSource].
 * @see TestCoroutineScheduler.timeSource
 */
// @ExperimentalCoroutinesApi
// TODO: Kotlin extension property; translate to free function
TimeSource::WithComparableMarks* test_time_source(TestScope& scope) {
    return scope.test_scheduler().time_source();
}

/**
 * Creates a [TestScope].
 *
 * It ensures that all the test module machinery is properly initialized.
 * - If [context] doesn't provide a [TestCoroutineScheduler] for orchestrating the virtual time used for delay-skipping,
 *   a new one is created, unless either
 *     - a [TestDispatcher] is provided, in which case [TestDispatcher.scheduler] is used;
 *     - at the moment of the creation of the scope, [Dispatchers.Main] is delegated to a [TestDispatcher], in which case
 *       its [TestCoroutineScheduler] is used.
 * - If [context] doesn't have a [TestDispatcher], a [StandardTestDispatcher] is created.
 * - A [CoroutineExceptionHandler] is created that makes [TestCoroutineScope.cleanupTestCoroutines] throw if there were
 *   any uncaught exceptions, or forwards the exceptions further in a platform-specific manner if the cleanup was
 *   already performed when an exception happened. Passing a [CoroutineExceptionHandler] is illegal, unless it's an
 *   [UncaughtExceptionCaptor], in which case the behavior is preserved for the time being for backward compatibility.
 *   If you need to have a specific [CoroutineExceptionHandler], please pass it to [launch] on an already-created
 *   [TestCoroutineScope] and share your use case at
 *   [our issue tracker](https://github.com/Kotlin/kotlinx.coroutines/issues).
 * - If [context] provides a [Job], that job is used as a parent for the new scope.
 *
 * @throws IllegalArgumentException if [context] has both [TestCoroutineScheduler] and a [TestDispatcher] linked to a
 * different scheduler.
 * @throws IllegalArgumentException if [context] has a [ContinuationInterceptor] that is not a [TestDispatcher].
 * @throws IllegalArgumentException if [context] has an [CoroutineExceptionHandler] that is not an
 * [UncaughtExceptionCaptor].
 */
// @Suppress("FunctionName")
// TODO: Kotlin factory function; use constructor or static factory in C++
TestScope* test_scope(CoroutineContext context = empty_coroutine_context()) {
    CoroutineContext ctx_with_dispatcher = with_delay_skipping(context);
    TestScopeImpl* scope_ptr = nullptr;
    CoroutineExceptionHandler* exception_handler;

    auto* existing_handler = ctx_with_dispatcher[CoroutineExceptionHandler::Key];
    if (existing_handler == nullptr) {
        exception_handler = new CoroutineExceptionHandler([&](const CoroutineContext& ctx, std::exception_ptr exception) {
            scope_ptr->report_exception(exception);
        });
    } else {
        throw std::invalid_argument(
            "A CoroutineExceptionHandler was passed to TestScope. " +
            std::string("Please pass it as an argument to a `launch` or `async` block on an already-created scope ") +
            "if uncaught exceptions require special treatment."
        );
    }

    auto* scope = new TestScopeImpl(ctx_with_dispatcher + exception_handler);
    scope_ptr = scope;
    return scope;
}

/**
 * Adds a [TestDispatcher] and a [TestCoroutineScheduler] to the context if there aren't any already.
 *
 * @throws IllegalArgumentException if both a [TestCoroutineScheduler] and a [TestDispatcher] are passed.
 * @throws IllegalArgumentException if a [ContinuationInterceptor] is passed that is not a [TestDispatcher].
 */
CoroutineContext with_delay_skipping(const CoroutineContext& context) {
    auto* interceptor = context[ContinuationInterceptor::Key];
    TestDispatcher* dispatcher;

    if (auto* test_disp = dynamic_cast<TestDispatcher*>(interceptor)) {
        auto* ctx_scheduler = context[TestCoroutineScheduler::kKey];
        if (ctx_scheduler != nullptr) {
            if (&test_disp->scheduler() != ctx_scheduler) {
                throw std::invalid_argument(
                    "Both a TestCoroutineScheduler and TestDispatcher linked to " +
                    std::string("another scheduler were passed.")
                );
            }
        }
        dispatcher = test_disp;
    } else if (interceptor == nullptr) {
        dispatcher = standard_test_dispatcher(context[TestCoroutineScheduler::kKey], nullptr);
    } else {
        throw std::invalid_argument("Dispatcher must implement TestDispatcher");
    }

    return context + dispatcher + &dispatcher->scheduler();
}

class TestScopeImpl : public AbstractCoroutine<void>, public TestScope {
private:
    bool entered_;
    bool finished_;
    std::vector<std::exception_ptr> uncaught_exceptions_;
    std::mutex lock_;
    CoroutineScope* background_scope_;

public:
    TestScopeImpl(const CoroutineContext& context)
        : AbstractCoroutine<void>(context, true, true),
          entered_(false),
          finished_(false) {

        background_scope_ = new CoroutineScope(
            coroutine_context() + BackgroundWork::instance +
            reporting_supervisor_job([this](std::exception_ptr ex) {
                // TODO: Kotlin CancellationException check needs C++ equivalent
                // if (it !is CancellationException) reportException(it)
                report_exception(ex);
            })
        );
    }

    TestCoroutineScheduler& test_scheduler() override {
        return *context_[TestCoroutineScheduler::kKey];
    }

    CoroutineScope& background_scope() override {
        return *background_scope_;
    }

    /** Called upon entry to [runTest]. Will throw if called more than once. */
    void enter() {
        std::lock_guard<std::mutex> guard(lock_);
        if (entered_)
            throw std::logic_error("Only a single call to `runTest` can be performed during one test.");
        entered_ = true;
        if (finished_)
            throw std::logic_error("Unexpected state: finished before entering");

        /** the order is important: [reportException] is only guaranteed not to throw if [entered] is `true` but
         * [finished] is `false`.
         * However, we also want [uncaughtExceptions] to be queried after the callback is registered,
         * because the exception collector will be able to report the exceptions that arrived before this test but
         * after the previous one, and learning about such exceptions as soon is possible is nice. */
        // @Suppress("INVISIBLE_REFERENCE", "INVISIBLE_MEMBER") // do not remove the INVISIBLE_REFERENCE suppression: required in K2
        ensure_platform_exception_handler_loaded(exception_collector);
        if (catch_non_test_related_exceptions) {
            exception_collector.add_on_exception_callback(&lock_, [this](std::exception_ptr ex) {
                report_exception(ex);
            });
        }

        if (!uncaught_exceptions_.empty()) {
            exception_collector.remove_on_exception_callback(&lock_);
            auto* err = new UncaughtExceptionsBeforeTest();
            for (auto& e : uncaught_exceptions_) {
                err->add_suppressed(e);
            }
            throw *err;
        }
    }

    /** Called at the end of the test. May only be called once. Returns the list of caught unhandled exceptions. */
    std::vector<std::exception_ptr> leave() {
        std::lock_guard<std::mutex> guard(lock_);
        if (!entered_ || finished_)
            throw std::logic_error("Invalid state in leave()");

        /** After [finished] becomes `true`, it is no longer valid to have [reportException] as the callback. */
        exception_collector.remove_on_exception_callback(&lock_);
        finished_ = true;
        return uncaught_exceptions_;
    }

    /** Called at the end of the test. May only be called once. */
    std::vector<std::exception_ptr> legacy_leave() {
        std::vector<std::exception_ptr> exceptions;
        {
            std::lock_guard<std::mutex> guard(lock_);
            if (!entered_ || finished_)
                throw std::logic_error("Invalid state in legacy_leave()");

            /** After [finished] becomes `true`, it is no longer valid to have [reportException] as the callback. */
            exception_collector.remove_on_exception_callback(&lock_);
            finished_ = true;
            exceptions = uncaught_exceptions_;
        }

        auto active_jobs = children().filter([](Job* j) { return j->is_active(); }).to_list();
        if (exceptions.empty()) {
            if (!active_jobs.empty())
                throw UncompletedCoroutinesError(
                    "Active jobs found during the tear-down. " +
                    std::string("Ensure that all coroutines are completed or cancelled by your test. ") +
                    "The active jobs: " /* + activeJobs */
                );
            if (!test_scheduler().is_idle())
                throw UncompletedCoroutinesError(
                    "Unfinished coroutines found during the tear-down. " +
                    std::string("Ensure that all coroutines are completed or cancelled by your test.")
                );
        }
        return exceptions;
    }

    /** Stores an exception to report after [runTest], or rethrows it if not inside [runTest]. */
    void report_exception(std::exception_ptr throwable) {
        std::lock_guard<std::mutex> guard(lock_);
        if (finished_) {
            std::rethrow_exception(throwable);
        } else {
            // @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE") // do not remove the INVISIBLE_REFERENCE suppression: required in K2
            for (auto& existing : uncaught_exceptions_) {
                // avoid reporting exceptions that already were reported.
                // TODO: Kotlin unwrap() function needs C++ equivalent
                // if (unwrap(throwable) == unwrap(existingThrowable))
                //     return;
            }
            uncaught_exceptions_.push_back(throwable);
            if (!entered_) {
                auto* err = new UncaughtExceptionsBeforeTest();
                err->add_suppressed(throwable);
                throw *err;
            }
        }
    }

    /** Throws an exception if the coroutine is not completing. */
    std::exception_ptr try_get_completion_cause() {
        return completion_cause();
    }

    std::string to_string() const override {
        if (finished_) return "TestScope[test ended]";
        if (entered_) return "TestScope[test started]";
        return "TestScope[test not started]";
    }
};

/** Use the knowledge that any [TestScope] that we receive is necessarily a [TestScopeImpl]. */
TestScopeImpl& as_specific_implementation(TestScope& scope) {
    // TODO: Kotlin when expression with type checking; use dynamic_cast in C++
    if (auto* impl = dynamic_cast<TestScopeImpl*>(&scope)) {
        return *impl;
    }
    throw std::logic_error("TestScope must be a TestScopeImpl");
}

class UncaughtExceptionsBeforeTest : public std::logic_error {
public:
    UncaughtExceptionsBeforeTest()
        : std::logic_error(
            "There were uncaught exceptions before the test started. Please avoid this," +
            std::string(" as such exceptions are also reported in a platform-dependent manner so that they are not lost.")
        ) {}

    void add_suppressed(std::exception_ptr ex) {
        // TODO: Implement suppressed exception tracking
    }
};

/**
 * Thrown when a test has completed and there are tasks that are not completed or cancelled.
 */
// @ExperimentalCoroutinesApi
class UncompletedCoroutinesError : public std::runtime_error {
public:
    explicit UncompletedCoroutinesError(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * A flag that controls whether [TestScope] should attempt to catch arbitrary exceptions flying through the system.
 * If it is enabled, then any exception that is not caught by the user code will be reported as a test failure.
 * By default, it is enabled, but some tests may want to disable it to test the behavior of the system when they have
 * their own exception handling procedures.
 */
// @PublishedApi
bool catch_non_test_related_exceptions = true;

} // namespace test
} // namespace coroutines
} // namespace kotlinx
