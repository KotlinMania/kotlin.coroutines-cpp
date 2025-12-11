/**
 * Test suite for CancellableContinuationImpl
 *
 * Tests the cancellation infrastructure WITHOUT using C++ coroutines.
 * Uses direct API calls to test the state machine behavior.
 */

#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <atomic>

using namespace kotlinx::coroutines;

// Mock continuation for testing
template<typename T>
struct MockContinuation : public Continuation<T> {
    std::shared_ptr<Job> job;
    std::atomic<bool> resumed{false};
    Result<T> last_result;

    MockContinuation(std::shared_ptr<Job> j = nullptr) : job(j) {}

    std::shared_ptr<CoroutineContext> get_context() const override {
        return job;
    }

    void resume_with(Result<T> result) override {
        last_result = result;
        resumed = true;
    }
};

// Test 1: Successful Resume
void test_successful_resume() {
    std::cout << "Test 1: Successful Resume..." << std::endl;

    auto mock = std::make_shared<MockContinuation<int>>();
    auto impl = std::make_shared<CancellableContinuationImpl<int>>(mock, 1);

    // Resume with value
    impl->resume(42, nullptr);

    // Check that continuation was resumed
    if (mock->resumed && mock->last_result.is_success() && mock->last_result.get_or_throw() == 42) {
        std::cout << "SUCCESS: Got 42" << std::endl;
    } else {
        std::cout << "FAILED: Resume did not work correctly" << std::endl;
        assert(false);
    }
}

// Test 2: Cancellation
void test_cancellation() {
    std::cout << "Test 2: Cancellation..." << std::endl;

    auto mock = std::make_shared<MockContinuation<int>>();
    auto impl = std::make_shared<CancellableContinuationImpl<int>>(mock, 1);

    bool handler_invoked = false;
    impl->invoke_on_cancellation([&](std::exception_ptr) {
        handler_invoked = true;
    });

    // Cancel the continuation
    bool cancelled = impl->cancel(std::make_exception_ptr(std::runtime_error("Cancelled")));

    if (cancelled && handler_invoked) {
        std::cout << "SUCCESS: Cancellation worked and handler invoked" << std::endl;
    } else {
        std::cout << "FAILED: cancelled=" << cancelled << ", handler_invoked=" << handler_invoked << std::endl;
        assert(false);
    }
}

// Test 3: Resume vs Cancel Race (Resume first)
void test_race_resume_wins() {
    std::cout << "Test 3: Race (Resume wins)..." << std::endl;

    auto mock = std::make_shared<MockContinuation<int>>();
    auto impl = std::make_shared<CancellableContinuationImpl<int>>(mock, 1);

    // Resume first
    impl->resume(100, nullptr);

    // Then try to cancel - should fail
    bool cancelled = impl->cancel(std::make_exception_ptr(std::runtime_error("Too late")));

    if (!cancelled && mock->resumed && mock->last_result.get_or_throw() == 100) {
        std::cout << "SUCCESS: Resume won the race" << std::endl;
    } else {
        std::cout << "FAILED: Race condition not handled correctly" << std::endl;
        assert(false);
    }
}

// Test 4: Cancel vs Resume Race (Cancel first)
void test_race_cancel_wins() {
    std::cout << "Test 4: Race (Cancel wins)..." << std::endl;

    auto mock = std::make_shared<MockContinuation<int>>();
    auto impl = std::make_shared<CancellableContinuationImpl<int>>(mock, 1);

    // Cancel first
    bool cancelled = impl->cancel(std::make_exception_ptr(std::runtime_error("Cancelled first")));

    // Then try to resume - should fail (or be ignored)
    impl->resume(100, nullptr);

    if (cancelled && impl->is_cancelled()) {
        std::cout << "SUCCESS: Cancel won the race" << std::endl;
    } else {
        std::cout << "FAILED: Race condition not handled correctly" << std::endl;
        assert(false);
    }
}

// Test 5: Parent Job Cancellation Propagation
void test_parent_cancellation() {
    std::cout << "Test 5: Parent Cancellation Propagation..." << std::endl;

    auto parent = JobImpl::create(nullptr);
    auto mock = std::make_shared<MockContinuation<int>>(parent);
    auto impl = std::make_shared<CancellableContinuationImpl<int>>(mock, 1);

    impl->init_cancellability();

    if (!impl->is_active()) {
        std::cout << "FAILED: Impl should be active initially" << std::endl;
        assert(false);
    }

    // Cancel parent
    parent->cancel(std::make_exception_ptr(std::runtime_error("Parent Cancelled")));

    if (impl->is_cancelled()) {
        std::cout << "SUCCESS: Impl cancelled by parent" << std::endl;
    } else {
        std::cout << "FAILED: Impl NOT cancelled by parent" << std::endl;
        assert(false);
    }
}

// Test 6: Multiple cancellation handlers
void test_multiple_handlers() {
    std::cout << "Test 6: Multiple Cancellation Handlers..." << std::endl;

    auto mock = std::make_shared<MockContinuation<int>>();
    auto impl = std::make_shared<CancellableContinuationImpl<int>>(mock, 1);

    int handler_count = 0;

    impl->invoke_on_cancellation([&](std::exception_ptr) {
        handler_count++;
    });

    impl->invoke_on_cancellation([&](std::exception_ptr) {
        handler_count++;
    });

    impl->cancel(std::make_exception_ptr(std::runtime_error("Cancelled")));

    // Depending on implementation, either last handler wins or all are called
    if (handler_count >= 1) {
        std::cout << "SUCCESS: Handler(s) invoked (count=" << handler_count << ")" << std::endl;
    } else {
        std::cout << "FAILED: No handlers invoked" << std::endl;
        assert(false);
    }
}

// Test 7: is_active / is_cancelled / is_completed states
void test_states() {
    std::cout << "Test 7: State Transitions..." << std::endl;

    auto mock = std::make_shared<MockContinuation<int>>();
    auto impl = std::make_shared<CancellableContinuationImpl<int>>(mock, 1);

    // Initially active
    if (!impl->is_active()) {
        std::cout << "FAILED: Should be active initially" << std::endl;
        assert(false);
    }

    if (impl->is_cancelled()) {
        std::cout << "FAILED: Should not be cancelled initially" << std::endl;
        assert(false);
    }

    if (impl->is_completed()) {
        std::cout << "FAILED: Should not be completed initially" << std::endl;
        assert(false);
    }

    // Resume completes it
    impl->resume(42, nullptr);

    if (impl->is_active()) {
        std::cout << "FAILED: Should not be active after resume" << std::endl;
        assert(false);
    }

    if (impl->is_completed()) {
        std::cout << "SUCCESS: State transitions correct" << std::endl;
    } else {
        std::cout << "FAILED: Should be completed after resume" << std::endl;
        assert(false);
    }
}

int main() {
    std::cout << "=== CancellableContinuationImpl Test Suite ===" << std::endl;
    std::cout << "(No C++ coroutines - direct API testing)" << std::endl;
    std::cout << std::endl;

    test_successful_resume();
    test_cancellation();
    test_race_resume_wins();
    test_race_cancel_wins();
    test_parent_cancellation();
    test_multiple_handlers();
    test_states();

    std::cout << std::endl;
    std::cout << "=== All tests passed ===" << std::endl;
    return 0;
}
