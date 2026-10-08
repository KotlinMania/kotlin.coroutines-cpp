/**
 * @file test_suspension_core.cpp
 * @brief Core tests for the __LINE__-based coroutine state machine macros.
 *
 * Tests coroutine_begin/coroutine_yield/coroutine_end macros that provide
 * resume regions for mandatory Kotlin/Native LLVM dispatch injection.
 */

#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/testing/TestBase.hpp"
#include <iostream>
#include <vector>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::intrinsics;
using namespace kotlinx::coroutines::testing;

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
    bool suspend_call = true;
    void* immediate_value = nullptr;
    int before_count = 0;
    int after_count = 0;

    explicit YieldValueCoroutine(std::shared_ptr<Continuation<void*>> completion,
                                 bool suspend = true, void* immediate = nullptr)
        : ContinuationImpl(std::move(completion)), suspend_call(suspend),
          immediate_value(immediate) {}

    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)

        ++before_count;
        coroutine_yield_value(this, result,
                              suspend_call ? COROUTINE_SUSPENDED : immediate_value, value);
        ++after_count;
        return value;
    }
};

// Completion continuation that captures the result
class TestCompletion : public Continuation<void*> {
public:
    bool completed = false;
    int completion_count = 0;
    void* result_value = nullptr;
    std::exception_ptr exception;

    std::shared_ptr<CoroutineContext> get_context() const override {
        return EmptyCoroutineContext::instance();
    }

    void resume_with(Result<void*> result) override {
        completed = true;
        ++completion_count;
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
    assert_true(is_coroutine_suspended(r1));
    assert_equals(10, coro->counter);
    assert_equals(static_cast<size_t>(1), execution_log.size());
    assert_equals(1, execution_log[0]);

    // Resume - runs until second yield
    void* r2 = coro->invoke_suspend(Result<void*>::success(nullptr));
    assert_true(is_coroutine_suspended(r2));
    assert_equals(20, coro->counter);
    assert_equals(static_cast<size_t>(2), execution_log.size());
    assert_equals(2, execution_log[1]);

    // Resume - runs to completion
    void* r3 = coro->invoke_suspend(Result<void*>::success(nullptr));
    assert_false(is_coroutine_suspended(r3));
    assert_equals(30, coro->counter);
    assert_equals(static_cast<size_t>(3), execution_log.size());
    assert_equals(3, execution_log[2]);

    std::cout << "PASSED" << std::endl;
}

void test_conditional_suspend() {
    std::cout << "test_conditional_suspend... ";

    // Test with suspension
    {
        auto completion = std::make_shared<TestCompletion>();
        auto coro = std::make_shared<ConditionalSuspendCoroutine>(true, completion);

        void* r1 = coro->invoke_suspend(Result<void*>::success(nullptr));
        assert_true(is_coroutine_suspended(r1));
        assert_equals(1, coro->value);

        void* r2 = coro->invoke_suspend(Result<void*>::success(nullptr));
        assert_false(is_coroutine_suspended(r2));
        assert_equals(2, coro->value);
    }

    // Test without suspension
    {
        auto completion = std::make_shared<TestCompletion>();
        auto coro = std::make_shared<ConditionalSuspendCoroutine>(false, completion);

        void* r1 = coro->invoke_suspend(Result<void*>::success(nullptr));
        assert_false(is_coroutine_suspended(r1));
        assert_equals(2, coro->value);
    }

    std::cout << "PASSED" << std::endl;
}

void test_loop_suspend() {
    std::cout << "test_loop_suspend... ";

    auto completion = std::make_shared<TestCompletion>();
    auto coro = std::make_shared<LoopCoroutine>(completion);

    // iteration 0: sum = 0, iteration = 1
    void* r = coro->invoke_suspend(Result<void*>::success(nullptr));
    assert_true(is_coroutine_suspended(r));
    assert_equals(1, coro->iteration);
    assert_equals(0, coro->sum);

    // iteration 1: sum = 1, iteration = 2
    r = coro->invoke_suspend(Result<void*>::success(nullptr));
    assert_true(is_coroutine_suspended(r));
    assert_equals(2, coro->iteration);
    assert_equals(1, coro->sum);

    // iteration 2: sum = 3, iteration = 3
    r = coro->invoke_suspend(Result<void*>::success(nullptr));
    assert_true(is_coroutine_suspended(r));
    assert_equals(3, coro->iteration);
    assert_equals(3, coro->sum);

    // Loop done, completion
    r = coro->invoke_suspend(Result<void*>::success(nullptr));
    assert_false(is_coroutine_suspended(r));

    std::cout << "PASSED" << std::endl;
}

void test_yield_value_resume_result() {
    std::cout << "test_yield_value_resume_result... ";

    static int k_marker = 42;
    void* expected = static_cast<void*>(&k_marker);

    auto completion = std::make_shared<TestCompletion>();
    auto coro = std::make_shared<YieldValueCoroutine>(completion);

    void* r1 = coro->invoke_suspend(Result<void*>::success(nullptr));
    assert_true(is_coroutine_suspended(r1));

    void* r2 = coro->invoke_suspend(Result<void*>::success(expected));
    assert_false(is_coroutine_suspended(r2));
    assert_equals(expected, r2);
    assert_equals(expected, coro->value);

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
    assert_equals(10, coro->counter);

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

    assert_true(threw);
    assert_true(execution_log.empty());
    std::cout << "PASSED" << std::endl;
}


// Regression contracts: NativeSuspendFunctionLowering.kt:253-335 and
// Kotlin/Native runtime ContinuationImpl.kt:21-45. These exercise observable
// handoffs rather than testing the textual form of the macro expansion.
void test_yield_value_immediate_result() {
    std::cout << "test_yield_value_immediate_result... ";
    int marker = 71;
    auto completion = std::make_shared<TestCompletion>();
    auto coro = std::make_shared<YieldValueCoroutine>(completion, false, &marker);
    coro->resume_with(Result<void*>::success(nullptr));
    assert_equals(1, completion->completion_count);
    assert_equals(static_cast<void*>(&marker), completion->result_value);
    assert_equals(1, coro->before_count);
    assert_equals(1, coro->after_count);
    std::cout << "PASSED" << std::endl;
}

void test_resumed_exception_stops_continuation() {
    std::cout << "test_resumed_exception_stops_continuation... ";
    auto completion = std::make_shared<TestCompletion>();
    auto coro = std::make_shared<YieldValueCoroutine>(completion);
    coro->resume_with(Result<void*>::success(nullptr));
    assert_equals(0, completion->completion_count);
    auto failure = std::make_exception_ptr(std::runtime_error("resumed failure"));
    coro->resume_with(Result<void*>::failure(failure));
    assert_equals(1, completion->completion_count);
    assert_true(completion->exception == failure);
    assert_equals(1, coro->before_count);
    assert_equals(0, coro->after_count);
    std::cout << "PASSED" << std::endl;
}

void test_resume_with_all_steps() {
    std::cout << "test_resume_with_all_steps... ";
    execution_log.clear();
    auto completion = std::make_shared<TestCompletion>();
    auto coro = std::make_shared<SimpleYieldCoroutine>(completion);
    coro->resume_with(Result<void*>::success(nullptr));
    assert_equals(0, completion->completion_count);
    assert_equals(10, coro->counter);
    coro->resume_with(Result<void*>::success(nullptr));
    assert_equals(0, completion->completion_count);
    assert_equals(20, coro->counter);
    coro->resume_with(Result<void*>::success(nullptr));
    assert_equals(1, completion->completion_count);
    assert_equals(30, coro->counter);
    assert_equals(std::vector<int>{1, 2, 3}, execution_log);
    std::cout << "PASSED" << std::endl;
}

void test_independent_frames() {
    std::cout << "test_independent_frames... ";
    auto loop_completion = std::make_shared<TestCompletion>();
    auto value_completion = std::make_shared<TestCompletion>();
    auto loop = std::make_shared<LoopCoroutine>(loop_completion);
    auto value = std::make_shared<YieldValueCoroutine>(value_completion);
    loop->resume_with(Result<void*>::success(nullptr));
    value->resume_with(Result<void*>::success(nullptr));
    assert_equals(0, loop_completion->completion_count);
    assert_equals(0, value_completion->completion_count);
    loop->resume_with(Result<void*>::success(nullptr));
    int payload = 93;
    value->resume_with(Result<void*>::success(&payload));
    assert_equals(1, value_completion->completion_count);
    assert_equals(static_cast<void*>(&payload), value_completion->result_value);
    assert_equals(1, value->before_count);
    assert_equals(1, value->after_count);
    assert_equals(2, loop->iteration);
    assert_equals(1, loop->sum);
    loop->resume_with(Result<void*>::success(nullptr));
    loop->resume_with(Result<void*>::success(nullptr));
    assert_equals(1, loop_completion->completion_count);
    assert_equals(3, loop->sum);
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
    test_yield_value_immediate_result();
    test_resumed_exception_stops_continuation();
    test_resume_with_all_steps();
    test_independent_frames();

    std::cout << "=== All tests passed ===" << std::endl;
    return 0;
}
