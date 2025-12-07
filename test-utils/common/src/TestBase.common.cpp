// Transliterated from: test-utils/common/src/TestBase.common.kt
// @file:Suppress("unused")
// TODO: #include <kotlinx/atomicfu/atomic.hpp>
// TODO: #include <kotlinx/coroutines/flow/flow.hpp>
// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <kotlinx/coroutines/internal/internal.hpp>
// TODO: #include <chrono>
// TODO: #include <test_framework.hpp>

namespace kotlinx {
namespace coroutines {
namespace testing {

// The number of milliseconds that is sure not to pass assertRunsFast.
constexpr long kSlow = 100000L;

// Asserts that a block completed within timeout.
template<typename T>
T assert_runs_fast(std::chrono::duration<double> timeout, std::function<T()> block) {
    T result;
    auto start = std::chrono::steady_clock::now();
    result = block();
    auto elapsed = std::chrono::steady_clock::now() - start;
    if (elapsed >= timeout) {
        throw std::runtime_error("Should complete in " + std::to_string(timeout.count()) +
                                 ", but took " + std::to_string(elapsed.count()));
    }
    return result;
}

// Asserts that a block completed within two seconds.
template<typename T>
T assert_runs_fast(std::function<T()> block) {
    return assert_runs_fast(std::chrono::seconds(2), block);
}

// Whether the tests should trace their calls to `expect` and `finish` with `println`.
// `false` by default. On the JVM, can be set to `true` by setting the `test.verbose` system property.
extern const bool kVerbose;

class OrderedExecution {
public:
    // Expect the next action to be index in order.
    virtual void expect(int index) = 0;

    // Expect this action to be final, with the given index.
    virtual void finish(int index) = 0;

    // Asserts that this line is never executed.
    virtual void expect_unreached() = 0;

    // Checks that finish was called.
    // By default, it is allowed to not call finish if expect was not called.
    // This is useful for tests that don't check the ordering of events.
    // When allow_not_using_expect is set to `false`, it is an error to not call finish in any case.
    virtual void check_finish_call(bool allow_not_using_expect = true) = 0;

    class Impl : public OrderedExecution {
    private:
        std::atomic<int> action_index{0};

    public:
        void expect(int index) override {
            int was_index = action_index.fetch_add(1) + 1;
            if (kVerbose) {
                std::cout << "expect(" << index << "), wasIndex=" << was_index << std::endl;
            }
            if (index != was_index) {
                if (was_index < 0) {
                    throw std::runtime_error("Expecting action index " + std::to_string(index) +
                                             " but it is actually finished");
                } else {
                    throw std::runtime_error("Expecting action index " + std::to_string(index) +
                                             " but it is actually " + std::to_string(was_index));
                }
            }
        }

        void finish(int index) override {
            int was_index = action_index.exchange(INT_MIN) + 1;
            if (kVerbose) {
                std::cout << "finish(" << index << "), wasIndex=" <<
                    (was_index < 0 ? "finished" : std::to_string(was_index)) << std::endl;
            }
            if (index != was_index) {
                if (was_index < 0) {
                    throw std::runtime_error("Finished more than once");
                } else {
                    throw std::runtime_error("Finishing with action index " + std::to_string(index) +
                                             " but it is actually " + std::to_string(was_index));
                }
            }
        }

        void expect_unreached() override {
            int value = action_index.load();
            std::string message;
            if (value < 0) {
                message = "already finished";
            } else if (value == 0) {
                message = "'expect' was not called yet";
            } else {
                message = "the last executed action was " + std::to_string(value);
            }
            throw std::runtime_error("Should not be reached, " + message);
        }

        void check_finish_call(bool allow_not_using_expect = true) override {
            int value = action_index.load();
            if (!(value < 0 || (allow_not_using_expect && value == 0))) {
                throw std::runtime_error("Expected `finish(" + std::to_string(value + 1) +
                                         ")` to be called, but the test finished");
            }
        }
    };
};

class ErrorCatching {
public:
    // Returns `true` if errors were logged in the test.
    virtual bool has_error() = 0;

    // Directly reports an error to the test catching facilities.
    virtual void report_error(Throwable* error) = 0;

    class Impl : public ErrorCatching {
    private:
        std::vector<Throwable*> errors;
        std::mutex lock;
        bool closed = false;

    public:
        bool has_error() override {
            std::lock_guard<std::mutex> guard(lock);
            return !errors.empty();
        }

        void report_error(Throwable* error) override {
            std::lock_guard<std::mutex> guard(lock);
            if (closed) {
                last_resort_report_exception(error);
            } else {
                errors.push_back(error);
            }
        }

        void close() {
            std::lock_guard<std::mutex> guard(lock);
            if (closed) {
                auto error = new std::runtime_error("ErrorCatching closed more than once");
                last_resort_report_exception(error);
                errors.push_back(reinterpret_cast<Throwable*>(error));
            }
            closed = true;
            if (!errors.empty()) {
                Throwable* first = errors[0];
                for (size_t i = 1; i < errors.size(); ++i) {
                    // TODO: Implement add_suppressed equivalent
                    // first->add_suppressed(errors[i]);
                }
                throw *first;
            }
        }
    };
};

// Reports an error *somehow* so that it doesn't get completely forgotten.
void last_resort_report_exception(Throwable* error); // Platform-specific implementation

// Throws std::runtime_error when `value` is false, like `check` in stdlib, but also ensures that the
// test will not complete successfully even if this exception is consumed somewhere in the test.
inline void check(ErrorCatching& error_catching, bool value, std::function<std::string()> lazy_message) {
    if (!value) {
        throw std::runtime_error(lazy_message());
    }
}

// Throws std::runtime_error, like `error` in stdlib, but also ensures that the test will not
// complete successfully even if this exception is consumed somewhere in the test.
[[noreturn]] inline void error(ErrorCatching& error_catching, const std::string& message, Throwable* cause = nullptr) {
    auto exception = new std::runtime_error(message);
    error_catching.report_error(reinterpret_cast<Throwable*>(exception));
    throw *exception;
}

// A class inheriting from which allows to check the execution order inside tests.
// @see TestBase
class OrderedExecutionTestBase : public OrderedExecution {
private:
    // TODO: move to by-delegation when reset is no longer needed.
    OrderedExecution::Impl ordered_execution_delegate;

public:
    // @AfterTest
    void check_finished() {
        ordered_execution_delegate.check_finish_call();
    }

    // Resets counter and finish flag. Workaround for parametrized tests absence in common
    void reset() {
        ordered_execution_delegate.check_finish_call();
        ordered_execution_delegate = OrderedExecution::Impl();
    }

    void expect(int index) override {
        ordered_execution_delegate.expect(index);
    }

    void finish(int index) override {
        ordered_execution_delegate.finish(index);
    }

    void expect_unreached() override {
        ordered_execution_delegate.expect_unreached();
    }

    void check_finish_call(bool allow_not_using_expect = true) override {
        ordered_execution_delegate.check_finish_call(allow_not_using_expect);
    }
};

template<typename T>
void void_func(T&) {}

// @OptionalExpectation
// actual annotation class NoJs
// Platform-specific annotation

// @OptionalExpectation
// actual annotation class NoNative
// Platform-specific annotation

// @OptionalExpectation
// actual annotation class NoWasmJs
// Platform-specific annotation

// @OptionalExpectation
// actual annotation class NoWasmWasi
// Platform-specific annotation

extern const bool kIsStressTest;
extern const int kStressTestMultiplier;
extern const int kStressTestMultiplierSqrt;

// The result of a multiplatform asynchronous test.
// Aliases into Unit on K/JVM and K/N, and into Promise on K/JS.
// @Suppress("NO_ACTUAL_FOR_EXPECT")
// Platform-specific: class TestResult

// Platform-specific: class TestBase : public OrderedExecutionTestBase, public ErrorCatching
// Methods:
// - void println(const std::string& message)
// - TestResult run_test(
//     std::function<bool(Throwable*)> expected = nullptr,
//     std::vector<std::function<bool(Throwable*)>> unhandled = {},
//     std::function<void(CoroutineScope&)> block
//   )
// - bool has_error()
// - void report_error(Throwable* error)

void hang(std::function<void()> on_cancellation) { // TODO: implement coroutine suspension
    try {
        // TODO: implement suspendCancellableCoroutine<void>
        // suspend_cancellable_coroutine<void>([](auto continuation) {});
    } catch (...) {
        on_cancellation();
        throw;
    }
}

template<typename T>
void assert_fails_with(Flow<T>& flow) { // TODO: implement coroutine suspension
    // TODO: implement assertFailsWith<T>
    // assert_fails_with<T>([&]() {
    //     flow.collect();
    // });
}

int sum(Flow<int>& flow) { // TODO: implement coroutine suspension
    return flow.fold(0, [](int acc, int value) { return acc + value; });
}

long long long_sum(Flow<long long>& flow) { // TODO: implement coroutine suspension
    return flow.fold(0LL, [](long long acc, long long value) { return acc + value; });
}

// data is added to avoid stacktrace recovery because CopyableThrowable is not accessible from common modules
class TestException : public std::exception {
private:
    std::string message_;
    void* data_;

public:
    TestException(const std::string& message = "", void* data = nullptr)
        : message_(message), data_(data) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }
};

class TestException1 : public std::exception {
private:
    std::string message_;
    void* data_;

public:
    TestException1(const std::string& message = "", void* data = nullptr)
        : message_(message), data_(data) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }
};

class TestException2 : public std::exception {
private:
    std::string message_;
    void* data_;

public:
    TestException2(const std::string& message = "", void* data = nullptr)
        : message_(message), data_(data) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }
};

class TestException3 : public std::exception {
private:
    std::string message_;
    void* data_;

public:
    TestException3(const std::string& message = "", void* data = nullptr)
        : message_(message), data_(data) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }
};

class TestCancellationException : public CancellationException {
private:
    void* data_;

public:
    TestCancellationException(const std::string& message = "", void* data = nullptr)
        : CancellationException(message), data_(data) {}
};

class TestRuntimeException : public std::runtime_error {
private:
    void* data_;

public:
    TestRuntimeException(const std::string& message = "", void* data = nullptr)
        : std::runtime_error(message), data_(data) {}
};

class RecoverableTestException : public std::runtime_error {
public:
    RecoverableTestException(const std::string& message = "")
        : std::runtime_error(message) {}
};

class RecoverableTestCancellationException : public CancellationException {
public:
    RecoverableTestCancellationException(const std::string& message = "")
        : CancellationException(message) {}
};

// Erases identity and equality checks for tests
CoroutineContext wrapper_dispatcher(CoroutineContext& context) {
    CoroutineDispatcher* dispatcher = dynamic_cast<CoroutineDispatcher*>(
        context[ContinuationInterceptor::kKey]
    );

    class WrapperDispatcher : public CoroutineDispatcher {
    private:
        CoroutineDispatcher* wrapped;

    public:
        WrapperDispatcher(CoroutineDispatcher* d) : wrapped(d) {}

        bool is_dispatch_needed(CoroutineContext& context) override {
            return wrapped->is_dispatch_needed(context);
        }

        void dispatch(CoroutineContext& context, Runnable& block) override {
            wrapped->dispatch(context, block);
        }
    };

    return CoroutineContext(new WrapperDispatcher(dispatcher));
}

CoroutineContext wrapper_dispatcher() { // TODO: implement coroutine suspension
    // TODO: Get current coroutineContext
    // return wrapper_dispatcher(coroutine_context());
    return CoroutineContext();
}

class BadClass {
public:
    bool operator==(const BadClass& other) const {
        throw std::runtime_error("equals");
    }

    size_t hash_code() const {
        throw std::runtime_error("hashCode");
    }

    std::string to_string() const {
        throw std::runtime_error("toString");
    }
};

extern const bool kIsJavaAndWindows;
extern const bool kIsNative;

// In common tests we emulate parameterized tests
// by iterating over parameters space in the single @Test method.
// This kind of tests is too slow for JS and does not fit into
// the default Mocha timeout, so we're using this flag to bail-out
// and run such tests only on JVM and K/N.
extern const bool kIsBoundByJsTestTimeout;

// `true` if this platform has the same event loop for `DefaultExecutor` and Dispatchers.Unconfined
extern const bool kUsesSharedEventLoop;

} // namespace testing
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement suspend function mechanics (coroutine suspension)
// 2. Implement atomic operations properly (replace std::atomic with kotlinx::atomicfu)
// 3. Implement Flow<T> with collect, fold methods
// 4. Implement CoroutineContext, CoroutineDispatcher, ContinuationInterceptor
// 5. Implement CancellationException base class
// 6. Implement suspendCancellableCoroutine
// 7. Implement Throwable base class with suppressed exceptions
// 8. Implement platform-specific VERBOSE, stress test flags
// 9. Implement platform-specific TestBase class
// 10. Implement platform-specific TestResult type
// 11. Implement platform-specific last_resort_report_exception
// 12. Implement Runnable interface
// 13. Add proper includes for all dependencies
// 14. Implement MainScope, Job, and coroutine_context() function
// 15. Handle memory management for exception objects
