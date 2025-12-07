// Transliterated from: test-utils/jvm/src/TestBase.kt
// TODO: #include <kotlinx/coroutines/scheduling/scheduling.hpp>
// TODO: #include <iostream>
// TODO: #include <vector>
// TODO: #include <thread>
// TODO: #include <atomic>
// TODO: #include <mutex>
// TODO: #include <test_framework.hpp>

namespace kotlinx {
namespace coroutines {
namespace testing {

const bool kVerbose = []() {
    try {
        // TODO: Implement System.getProperty equivalent
        // const char* prop = std::getenv("test.verbose");
        // return prop != nullptr && std::string(prop) == "true";
        return false;
    } catch (...) {
        return false;
    }
}();

// Is `true` when running in a nightly stress test mode.
const bool kIsStressTest = []() {
    // TODO: Implement System.getProperty equivalent
    // const char* prop = std::getenv("stressTest");
    // return prop != nullptr && std::string(prop) == "true";
    return false;
}();

const int kStressTestMultiplierSqrt = kIsStressTest ? 5 : 1;

constexpr long kShutdownTimeout = 1000L; // 1s at most to wait per thread

// Multiply various constants in stress tests by this factor, so that they run longer during nightly stress test.
const int kStressTestMultiplier = kStressTestMultiplierSqrt * kStressTestMultiplierSqrt;

// @Suppress("ACTUAL_WITHOUT_EXPECT")
using TestResult = void; // Unit type

void last_resort_report_exception(Throwable* error) {
    std::string cause_msg = ""; // TODO: error->cause() ? ": " + error->cause()->what() : "";
    std::cerr << error->what() << cause_msg << std::endl;
    // TODO: error->cause()->printStackTrace(std::cerr);
    std::cerr << "--- Detected at ---" << std::endl;
    // TODO: Throwable().printStackTrace(std::cerr);
}

// Base class for tests, so that tests for predictable scheduling of actions in multiple coroutines sharing a single
// thread can be written. Use it like this:
//
// ```
// class MyTest : TestBase() {
//     @Test
//     fun testSomething() = runBlocking { // run in the context of the main thread
//         expect(1) // initiate action counter
//         launch { // use the context of the main thread
//             expect(3) // the body of this coroutine in going to be executed in the 3rd step
//         }
//         expect(2) // launch just scheduled coroutine for execution later, so this line is executed second
//         yield() // yield main thread to the launched job
//         finish(4) // fourth step is the last one. `finish` must be invoked or test fails
//     }
// }
// ```
class TestBase : public OrderedExecutionTestBase, public ErrorCatching {
private:
    bool disable_out_check;
    ErrorCatching::Impl error_catching_impl;

    // Shutdown sequence
    std::unordered_set<std::thread::id> threads_before;
    std::vector<Throwable*> uncaught_exceptions;
    std::mutex uncaught_mutex;
    // TODO: Thread::UncaughtExceptionHandler* original_uncaught_exception_handler = nullptr;

public:
    TestBase() : TestBase(false) {}

    TestBase(bool disable_out_check)
        : disable_out_check(disable_out_check)
        , error_catching_impl() {}

    TestBase(bool disable_out_check, ErrorCatching::Impl error_catching)
        : disable_out_check(disable_out_check)
        , error_catching_impl(error_catching) {}

    void println(const std::string& message) {
        // TODO: PrintlnStrategy::actual_system_out.println(message);
        std::cout << message << std::endl;
    }

    // @BeforeTest
    void before() {
        init_pools_before_test();
        threads_before = current_threads();
        // TODO: original_uncaught_exception_handler = Thread::get_default_uncaught_exception_handler();
        // TODO: Thread::set_default_uncaught_exception_handler([&](std::thread& t, Throwable* e) {
        //     println("Exception in thread " + /* t.name() */ "" + ": " + e->what());
        //     e->printStackTrace();
        //     std::lock_guard<std::mutex> lock(uncaught_mutex);
        //     uncaught_exceptions.push_back(e);
        // });
        // TODO: PrintlnStrategy::configure(disable_out_check);
    }

    // @AfterTest
    void on_completion() {
        // onCompletion should not throw exceptions before it finishes all cleanup, so that other tests always
        // start in a clear, restored state, so we postpone throwing the observed errors.
        auto cleanup_step = [&](std::function<void()> block) {
            try {
                block();
            } catch (Throwable* e) {
                report_error(e);
            }
        };

        cleanup_step([&]() { check_finish_call(); });
        // Reset the output stream first
        cleanup_step([&]() { /* PrintlnStrategy::reset(); */ });
        // Shutdown all thread pools
        cleanup_step([&]() { shutdown_pools_after_test(); });
        // Check that are now leftover threads
        cleanup_step([&]() { check_test_threads(threads_before); });
        // Restore original uncaught exception handler after the main shutdown sequence
        // TODO: Thread::set_default_uncaught_exception_handler(original_uncaught_exception_handler);

        if (!uncaught_exceptions.empty()) {
            report_error(new std::runtime_error("Expected no uncaught exceptions, but got " +
                                                 std::to_string(uncaught_exceptions.size())));
        }

        // The very last action -- throw all the detected errors
        error_catching_impl.close();
    }

    TestResult run_test(
        std::function<bool(Throwable*)> expected = nullptr,
        std::vector<std::function<bool(Throwable*)>> unhandled = {},
        std::function<void(CoroutineScope&)> block = nullptr // TODO: implement coroutine suspension
    ) {
        int ex_count = 0;
        Throwable* ex = nullptr;

        try {
            // TODO: Implement runBlocking
            // run_blocking(
            //     CoroutineExceptionHandler([&](CoroutineContext& ctx, Throwable* e) {
            //         if (dynamic_cast<CancellationException*>(e)) return; // are ignored
            //         ex_count++;
            //         if (ex_count > unhandled.size()) {
            //             error(error_catching_impl, "Too many unhandled exceptions " + std::to_string(ex_count) +
            //                   ", expected " + std::to_string(unhandled.size()) + ", got: " + e->what(), e);
            //         }
            //         if (!unhandled[ex_count - 1](e)) {
            //             error(error_catching_impl, "Unhandled exception was unexpected: " + std::string(e->what()), e);
            //         }
            //     }),
            //     block
            // );
        } catch (Throwable* e) {
            ex = e;
            if (expected != nullptr) {
                if (!expected(e)) {
                    error(error_catching_impl, "Unexpected exception: " + std::string(e->what()), e);
                }
            } else {
                throw e;
            }
        }

        if (ex == nullptr && expected != nullptr) {
            throw std::runtime_error("Exception was expected but none produced");
        }
        if (ex_count < unhandled.size()) {
            error(error_catching_impl, "Too few unhandled exceptions " + std::to_string(ex_count) +
                  ", expected " + std::to_string(unhandled.size()));
        }
    }

    bool has_error() override {
        return error_catching_impl.has_error();
    }

    void report_error(Throwable* error) override {
        error_catching_impl.report_error(error);
    }

protected:
    CoroutineContext* current_dispatcher() { // TODO: implement coroutine suspension
        // TODO: return coroutine_context()[ContinuationInterceptor::kKey];
        return nullptr;
    }
};

// TODO: Implement PrintlnStrategy class (lines 153-225 in original)
// TODO: Implement init_pools_before_test function
// TODO: Implement shutdown_pools_after_test function

void init_pools_before_test() {
    // @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
    // TODO: DefaultScheduler::use_private_scheduler();
}

void shutdown_pools_after_test() {
    // @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
    // TODO: DefaultScheduler::shutdown(kShutdownTimeout);
    // TODO: DefaultExecutor::shutdown_for_tests(kShutdownTimeout);
    // TODO: DefaultScheduler::restore();
}

const bool kIsNative = false;
const bool kIsBoundByJsTestTimeout = false;

// We ignore tests that test **real** non-virtualized tests with time on Windows, because
// our CI Windows is virtualized itself (oh, the irony) and its clock resolution is dozens of ms,
// which makes such tests flaky.
const bool kIsJavaAndWindows = []() {
    // TODO: return std::getenv("os.name") contains "Windows";
    return false;
}();

const bool kUsesSharedEventLoop = false;

} // namespace testing
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement suspend function mechanics (coroutine suspension)
// 2. Implement System.getProperty / std::getenv properly
// 3. Implement Throwable with cause() and printStackTrace()
// 4. Implement runBlocking function
// 5. Implement CoroutineExceptionHandler
// 6. Implement CancellationException detection
// 7. Implement current_threads() function
// 8. Implement check_test_threads() function
// 9. Implement Thread::UncaughtExceptionHandler equivalent
// 10. Implement PrintlnStrategy for output capture
// 11. Implement init_pools_before_test and shutdown_pools_after_test
// 12. Implement DefaultScheduler and DefaultExecutor
// 13. Implement ContinuationInterceptor
// 14. Add proper includes for all dependencies
// 15. Handle memory management for exception objects
