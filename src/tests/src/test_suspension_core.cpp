/**
 * @file test_suspension_core.cpp
 * @brief Core tests for the __LINE__-based coroutine state machine macros.
 *
 * Tests coroutine_begin/coroutine_yield/coroutine_end macros that provide
 * stackless coroutines via Duff's device pattern.
 */

#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include <iostream>
#include <cassert>
#include <vector>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::intrinsics;

// Track execution order
static std::vector<int> execution_log;

/**
 * Simple coroutine that yields twice.
 * Demonstrates the __LINE__ state machine pattern.
 */
class SimpleYieldCoroutine : public ContinuationImpl {
public:
    void* _label = nullptr;  // blockaddress storage (NativePtr)
    int counter = 0;

    explicit SimpleYieldCoroutine(std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)) {}

    void* invoke_suspend(Result<void*> result) override {
        (void)result;

        coroutine_begin(this)

        execution_log.push_back(1);
        counter = 10;

        // First yield - returns COROUTINE_SUSPENDED
        coroutine_yield(this, COROUTINE_SUSPENDED);

        execution_log.push_back(2);
        counter = 20;

        // Second yield
        coroutine_yield(this, COROUTINE_SUSPENDED);

        execution_log.push_back(3);
        counter = 30;

        coroutine_end(this)
    }
};

/**
 * Coroutine that conditionally suspends based on a value.
 */
class ConditionalSuspendCoroutine : public ContinuationImpl {
public:
    void* _label = nullptr;  // blockaddress storage (NativePtr)
    bool should_suspend;
    int value = 0;

    ConditionalSuspendCoroutine(bool suspend, std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)), should_suspend(suspend) {}

    void* invoke_suspend(Result<void*> result) override {
        (void)result;

        coroutine_begin(this)

        value = 1;

        // Only suspends if should_suspend is true
        if (should_suspend) {
            coroutine_yield(this, COROUTINE_SUSPENDED);
        }

        value = 2;

        coroutine_end(this)
    }
};

/**
 * Coroutine with a loop containing suspend points.
 */
class LoopCoroutine : public ContinuationImpl {
public:
    void* _label = nullptr;  // blockaddress storage (NativePtr)
    int iteration = 0;
    int sum = 0;

    explicit LoopCoroutine(std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)) {}

    void* invoke_suspend(Result<void*> result) override {
        (void)result;

        coroutine_begin(this)

        while (iteration < 3) {
            sum += iteration;
            iteration++;
            coroutine_yield(this, COROUTINE_SUSPENDED);
        }

        coroutine_end(this)
    }
};

/**
 * Coroutine that resumes with a value via the invoke_suspend(Result<...>) parameter.
 *
 * This exercises Kotlin/Native-style suspension point semantics:
 * - initial call returns COROUTINE_SUSPENDED
 * - resumed call receives the value via `result`
 */
class YieldValueCoroutine : public ContinuationImpl {
public:
    void* _label = nullptr;  // blockaddress storage (NativePtr)
    void* value = nullptr;

    explicit YieldValueCoroutine(std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)) {}

    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)

        coroutine_yield_value(this, result, COROUTINE_SUSPENDED, value);

        return value;
    }
};

// Completion continuation that captures the result
class TestCompletion : public Continuation<void*> {
public:
    bool completed = false;
    void* result_value = nullptr;
    std::exception_ptr exception;

    std::shared_ptr<CoroutineContext> get_context() const override {
        return EmptyCoroutineContext::instance();
    }

    void resume_with(Result<void*> result) override {
        completed = true;
        if (result.is_success()) {
            result_value = result.get_or_throw();
        } else {
            exception = result.exception_or_null();
        }
    }
};

void test_simple_yield() {
    std::cout << "test_simple_yield... ";
    execution_log.clear();

    auto completion = std::make_shared<TestCompletion>();
    auto coro = std::make_shared<SimpleYieldCoroutine>(completion);

    // First call - runs until first yield
    void* r1 = coro->invoke_suspend(Result<void*>::success(nullptr));
    assert(is_coroutine_suspended(r1));
    assert(coro->counter == 10);
    assert(execution_log.size() == 1);
    assert(execution_log[0] == 1);

    // Resume - runs until second yield
    void* r2 = coro->invoke_suspend(Result<void*>::success(nullptr));
    assert(is_coroutine_suspended(r2));
    assert(coro->counter == 20);
    assert(execution_log.size() == 2);
    assert(execution_log[1] == 2);

    // Resume - runs to completion
    void* r3 = coro->invoke_suspend(Result<void*>::success(nullptr));
    assert(!is_coroutine_suspended(r3));
    assert(coro->counter == 30);
    assert(execution_log.size() == 3);
    assert(execution_log[2] == 3);

    std::cout << "PASSED" << std::endl;
}

void test_conditional_suspend() {
    std::cout << "test_conditional_suspend... ";

    // Test with suspension
    {
        auto completion = std::make_shared<TestCompletion>();
        auto coro = std::make_shared<ConditionalSuspendCoroutine>(true, completion);

        void* r1 = coro->invoke_suspend(Result<void*>::success(nullptr));
        assert(is_coroutine_suspended(r1));
        assert(coro->value == 1);

        void* r2 = coro->invoke_suspend(Result<void*>::success(nullptr));
        assert(!is_coroutine_suspended(r2));
        assert(coro->value == 2);
    }

    // Test without suspension
    {
        auto completion = std::make_shared<TestCompletion>();
        auto coro = std::make_shared<ConditionalSuspendCoroutine>(false, completion);

        void* r1 = coro->invoke_suspend(Result<void*>::success(nullptr));
        assert(!is_coroutine_suspended(r1));
        assert(coro->value == 2);
    }

    std::cout << "PASSED" << std::endl;
}

void test_loop_suspend() {
    std::cout << "test_loop_suspend... ";

    auto completion = std::make_shared<TestCompletion>();
    auto coro = std::make_shared<LoopCoroutine>(completion);

    // iteration 0: sum = 0, iteration = 1
    void* r = coro->invoke_suspend(Result<void*>::success(nullptr));
    assert(is_coroutine_suspended(r));
    assert(coro->iteration == 1);
    assert(coro->sum == 0);

    // iteration 1: sum = 1, iteration = 2
    r = coro->invoke_suspend(Result<void*>::success(nullptr));
    assert(is_coroutine_suspended(r));
    assert(coro->iteration == 2);
    assert(coro->sum == 1);

    // iteration 2: sum = 3, iteration = 3
    r = coro->invoke_suspend(Result<void*>::success(nullptr));
    assert(is_coroutine_suspended(r));
    assert(coro->iteration == 3);
    assert(coro->sum == 3);

    // Loop done, completion
    r = coro->invoke_suspend(Result<void*>::success(nullptr));
    assert(!is_coroutine_suspended(r));

    std::cout << "PASSED" << std::endl;
}

void test_yield_value_resume_result() {
    std::cout << "test_yield_value_resume_result... ";

    static int k_marker = 42;
    void* expected = static_cast<void*>(&k_marker);

    auto completion = std::make_shared<TestCompletion>();
    auto coro = std::make_shared<YieldValueCoroutine>(completion);

    void* r1 = coro->invoke_suspend(Result<void*>::success(nullptr));
    assert(is_coroutine_suspended(r1));

    void* r2 = coro->invoke_suspend(Result<void*>::success(expected));
    assert(!is_coroutine_suspended(r2));
    assert(r2 == expected);
    assert(coro->value == expected);

    std::cout << "PASSED" << std::endl;
}

void test_resume_with_value() {
    std::cout << "test_resume_with_value... ";

    // Test that BaseContinuationImpl::resume_with drives the state machine
    auto completion = std::make_shared<TestCompletion>();
    auto coro = std::make_shared<SimpleYieldCoroutine>(completion);

    // Start via resume_with - this drives the loop in BaseContinuationImpl
    coro->resume_with(Result<void*>::success(nullptr));

    // After first invoke_suspend returns COROUTINE_SUSPENDED,
    // resume_with returns (doesn't keep looping)
    assert(coro->counter == 10);

    std::cout << "PASSED" << std::endl;
}

void test_start_with_exception_throws() {
    std::cout << "test_start_with_exception_throws... ";
    execution_log.clear();

    auto completion = std::make_shared<TestCompletion>();
    auto coro = std::make_shared<SimpleYieldCoroutine>(completion);

    bool threw = false;
    try {
        (void)coro->invoke_suspend(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("boom"))));
    } catch (const std::runtime_error&) {
        threw = true;
    }

    assert(threw);
    assert(execution_log.empty());
    std::cout << "PASSED" << std::endl;
}

int main() {
    std::cout << "=== test_suspension_core ===" << std::endl;

    test_simple_yield();
    test_conditional_suspend();
    test_loop_suspend();
    test_yield_value_resume_result();
    test_resume_with_value();
    test_start_with_exception_throws();

    std::cout << "=== All tests passed ===" << std::endl;
    return 0;
}
