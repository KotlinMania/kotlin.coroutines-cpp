/**
 * @file TestBase.hpp
 * @brief Test infrastructure for kotlinx.coroutines tests
 *
 * Transliterated from: test-utils/common/src/TestBase.common.kt
 *                      test-utils/native/src/TestBase.kt
 *
 * Provides:
 * - OrderedExecution: expect(n), finish(n), expect_unreached()
 * - ErrorCatching: has_error(), report_error()
 * - TestBase: combines both, provides run_test()
 * - Test exceptions: TestException, TestCancellationException
 */

#pragma once

#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include <atomic>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <mutex>
#include <cassert>

namespace kotlinx {
namespace coroutines {
namespace testing {

// =============================================================================
// Constants
// =============================================================================

constexpr bool VERBOSE = false;
constexpr long SLOW = 100000L;

// =============================================================================
// Test Exceptions
// =============================================================================

class TestException : public std::runtime_error {
public:
    explicit TestException(const std::string& message = "TestException")
        : std::runtime_error(message) {}
};

class TestException1 : public std::runtime_error {
public:
    explicit TestException1(const std::string& message = "TestException1")
        : std::runtime_error(message) {}
};

class TestException2 : public std::runtime_error {
public:
    explicit TestException2(const std::string& message = "TestException2")
        : std::runtime_error(message) {}
};

class TestException3 : public std::runtime_error {
public:
    explicit TestException3(const std::string& message = "TestException3")
        : std::runtime_error(message) {}
};

class TestCancellationException : public CancellationException {
public:
    explicit TestCancellationException(const std::string& message = "TestCancellationException")
        : CancellationException(message) {}
};

class TestRuntimeException : public std::runtime_error {
public:
    explicit TestRuntimeException(const std::string& message = "TestRuntimeException")
        : std::runtime_error(message) {}
};

// =============================================================================
// OrderedExecution Interface
// =============================================================================

class OrderedExecution {
public:
    virtual ~OrderedExecution() = default;

    /** Expect the next action to be [index] in order. */
    virtual void expect(int index) = 0;

    /** Expect this action to be final, with the given [index]. */
    virtual void finish(int index) = 0;

    /** Asserts that this line is never executed. */
    virtual void expect_unreached() = 0;

    /**
     * Checks that finish() was called.
     * @param allow_not_using_expect If true, it's OK if expect() was never called
     */
    virtual void check_finish_call(bool allow_not_using_expect = true) = 0;
};

/**
 * Implementation of OrderedExecution using atomic counter
 */
class OrderedExecutionImpl : public OrderedExecution {
private:
    std::atomic<int> action_index_{0};

public:
    void expect(int index) override {
        int was_index = ++action_index_;
        if (VERBOSE) {
            std::cout << "expect(" << index << "), wasIndex=" << was_index << std::endl;
        }
        if (index != was_index) {
            if (was_index < 0) {
                throw std::logic_error("Expecting action index " + std::to_string(index) +
                                       " but it is actually finished");
            } else {
                throw std::logic_error("Expecting action index " + std::to_string(index) +
                                       " but it is actually " + std::to_string(was_index));
            }
        }
    }

    void finish(int index) override {
        int was_index = action_index_.exchange(INT_MIN) + 1;
        if (VERBOSE) {
            std::cout << "finish(" << index << "), wasIndex="
                      << (was_index < 0 ? "finished" : std::to_string(was_index)) << std::endl;
        }
        if (index != was_index) {
            if (was_index < 0) {
                throw std::logic_error("Finished more than once");
            } else {
                throw std::logic_error("Finishing with action index " + std::to_string(index) +
                                       " but it is actually " + std::to_string(was_index));
            }
        }
    }

    void expect_unreached() override {
        int val = action_index_.load();
        std::string msg = "Should not be reached, ";
        if (val < 0) {
            msg += "already finished";
        } else if (val == 0) {
            msg += "'expect' was not called yet";
        } else {
            msg += "the last executed action was " + std::to_string(val);
        }
        throw std::logic_error(msg);
    }

    void check_finish_call(bool allow_not_using_expect = true) override {
        int val = action_index_.load();
        bool ok = (val < 0) || (allow_not_using_expect && val == 0);
        if (!ok) {
            throw std::logic_error("Expected `finish(" + std::to_string(val + 1) +
                                   ")` to be called, but the test finished");
        }
    }

    void reset() {
        check_finish_call();
        action_index_.store(0);
    }
};

// =============================================================================
// ErrorCatching Interface
// =============================================================================

class ErrorCatching {
public:
    virtual ~ErrorCatching() = default;

    /** Returns true if errors were logged in the test. */
    virtual bool has_error() const = 0;

    /** Directly reports an error to the test catching facilities. */
    virtual void report_error(std::exception_ptr error) = 0;
};

class ErrorCatchingImpl : public ErrorCatching {
private:
    std::vector<std::exception_ptr> errors_;
    mutable std::mutex mutex_;
    bool closed_ = false;

public:
    bool has_error() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return !errors_.empty();
    }

    void report_error(std::exception_ptr error) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) {
            try {
                std::rethrow_exception(error);
            } catch (const std::exception& e) {
                std::cerr << "Late error: " << e.what() << std::endl;
            }
        } else {
            errors_.push_back(error);
        }
    }

    void close() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) {
            errors_.push_back(std::make_exception_ptr(
                std::logic_error("ErrorCatching closed more than once")));
        }
        closed_ = true;
        if (!errors_.empty()) {
            std::rethrow_exception(errors_.front());
        }
    }
};

// =============================================================================
// TestBase - Main test class
// =============================================================================

/**
 * Base class for coroutine tests.
 *
 * Provides:
 * - expect(n), finish(n), expect_unreached() for ordering verification
 * - has_error(), report_error() for error catching
 * - run_test() to execute test blocks in a coroutine context
 */
class TestBase : public OrderedExecution, public ErrorCatching {
private:
    OrderedExecutionImpl ordered_execution_;
    ErrorCatchingImpl error_catching_;

public:
    virtual ~TestBase() = default;

    // OrderedExecution delegation
    void expect(int index) override { ordered_execution_.expect(index); }
    void finish(int index) override { ordered_execution_.finish(index); }
    void expect_unreached() override { ordered_execution_.expect_unreached(); }
    void check_finish_call(bool allow_not_using_expect = true) override {
        ordered_execution_.check_finish_call(allow_not_using_expect);
    }

    // ErrorCatching delegation
    bool has_error() const override { return error_catching_.has_error(); }
    void report_error(std::exception_ptr error) override { error_catching_.report_error(error); }

    // Reset for parameterized tests
    void reset() {
        ordered_execution_.reset();
    }

    // Print helper
    template<typename T>
    void println(const T& message) {
        std::cout << message << std::endl;
    }

    /**
     * Run a test in a blocking coroutine context.
     *
     * @param block The test body - receives CoroutineScope*
     */
    void run_test(std::function<void(CoroutineScope*)> block) {
        run_test(nullptr, {}, block);
    }

    /**
     * Run a test with expected exception.
     *
     * @param expected Predicate to check if thrown exception is expected
     * @param block The test body
     */
    void run_test(
        std::function<bool(std::exception_ptr)> expected,
        std::function<void(CoroutineScope*)> block
    ) {
        run_test(expected, {}, block);
    }

    /**
     * Full run_test with expected exception and unhandled exception handlers.
     */
    void run_test(
        std::function<bool(std::exception_ptr)> expected,
        std::vector<std::function<bool(std::exception_ptr)>> unhandled,
        std::function<void(CoroutineScope*)> block
    ) {
        std::exception_ptr ex = nullptr;
        size_t ex_count = 0;

        try {
            // Create context with exception handler
            auto context = EmptyCoroutineContext::instance();
            // TODO: Add CoroutineExceptionHandler to context

            // Run blocking (use Unit, not void, as template parameter)
            run_blocking<Unit>(context, [&](CoroutineScope* scope) -> Unit {
                block(scope);
                return Unit{};
            });

        } catch (...) {
            ex = std::current_exception();
            if (expected) {
                if (!expected(ex)) {
                    throw std::logic_error("Unexpected exception type");
                }
            } else {
                throw;
            }
        }

        if (!ex && expected) {
            throw std::logic_error("Exception was expected but none produced");
        }

        if (ex_count < unhandled.size()) {
            throw std::logic_error("Too few unhandled exceptions " + std::to_string(ex_count) +
                                   ", expected " + std::to_string(unhandled.size()));
        }
    }
};

// =============================================================================
// Assertion Helpers
// =============================================================================

inline void assert_true(bool condition, const std::string& message = "Assertion failed") {
    if (!condition) {
        throw std::logic_error(message);
    }
}

inline void assert_false(bool condition, const std::string& message = "Assertion failed") {
    assert_true(!condition, message);
}

template<typename T>
void assert_equals(const T& expected, const T& actual, const std::string& message = "") {
    if (expected != actual) {
        std::string msg = message.empty() ? "Expected equal values" : message;
        throw std::logic_error(msg);
    }
}

template<typename T>
void assert_same(const T& expected, const T& actual) {
    if (&expected != &actual) {
        throw std::logic_error("Expected same object reference");
    }
}

inline void assert_null(void* ptr) {
    if (ptr != nullptr) {
        throw std::logic_error("Expected null");
    }
}

inline void assert_null(std::exception_ptr ptr) {
    if (ptr != nullptr) {
        throw std::logic_error("Expected null exception_ptr");
    }
}

template<typename T, typename U>
void assert_is(const U& value) {
    if (dynamic_cast<const T*>(&value) == nullptr) {
        throw std::logic_error("Type assertion failed");
    }
}

// =============================================================================
// BadClass for testing equality edge cases
// =============================================================================

class BadClass {
public:
    bool operator==(const BadClass&) const { throw std::logic_error("equals"); }
    size_t hash() const { throw std::logic_error("hashCode"); }
    std::string to_string() const { throw std::logic_error("toString"); }
};

} // namespace testing
} // namespace coroutines
} // namespace kotlinx
