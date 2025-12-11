/**
 * Test suite for CancellableContinuationImpl
 *
 * Tests the cancellation infrastructure WITHOUT using C++ coroutines.
 * Uses direct API calls to test the state machine behavior.
 */

#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <atomic>

using namespace kotlinx::coroutines;

// Mock continuation for testing - minimal implementation
template<typename T>
struct MockContinuation : public Continuation<T> {
    std::atomic<bool> resumed{false};
    Result<T> last_result;

    std::shared_ptr<CoroutineContext> get_context() const override {
        return nullptr;  // No context needed for basic tests
    }

    void resume_with(Result<T> result) override {
        last_result = result;
        resumed = true;
    }
};

// Test 1: Successful Resume (with suspension - the normal coroutine path)
void test_successful_resume() {
    std::cout << "Test 1: Successful Resume (suspended first)..." << std::endl;

    auto mock = std::make_shared<MockContinuation<int>>();
    auto impl = std::make_shared<CancellableContinuationImpl<int>>(mock, 1);

    std::cout << "  Created impl, calling get_result() to suspend..." << std::endl;

    // Simulate what happens in a real coroutine:
    // First, get_result() is called which tries to suspend
    void* result = impl->get_result();

    if (result == intrinsics::get_COROUTINE_SUSPENDED()) {
        std::cout << "  Suspended as expected, now resuming..." << std::endl;
    } else {
        std::cout << "  UNEXPECTED: Did not suspend" << std::endl;
        assert(false);
    }

    // Now resume - this should dispatch to the delegate
    impl->resume(42, nullptr);

    std::cout << "  Resume called. mock->resumed=" << mock->resumed.load() << std::endl;
    std::cout << "  last_result.is_success()=" << mock->last_result.is_success() << std::endl;
    if (mock->last_result.is_success()) {
        std::cout << "  last_result.get_or_throw()=" << mock->last_result.get_or_throw() << std::endl;
    }

    // Check that continuation was resumed
    if (mock->resumed && mock->last_result.is_success() && mock->last_result.get_or_throw() == 42) {
        std::cout << "SUCCESS: Got 42" << std::endl;
    } else {
        std::cout << "FAILED: Resume did not work correctly" << std::endl;
        assert(false);
    }
}

// Test 1b: Resume before suspension (fast path - no dispatch needed)
void test_resume_before_suspend() {
    std::cout << "Test 1b: Resume before suspension (fast path)..." << std::endl;

    auto mock = std::make_shared<MockContinuation<int>>();
    auto impl = std::make_shared<CancellableContinuationImpl<int>>(mock, 1);

    // Resume BEFORE calling get_result - this is the fast path
    // State transitions: Active -> CompletedContinuation
    // Decision: UNDECIDED -> RESUMED (via dispatch_resume -> try_resume)
    impl->resume(42, nullptr);

    // Delegate should NOT have been called (no dispatch because not suspended)
    if (mock->resumed) {
        std::cout << "WARNING: Delegate was called even though we didn't suspend (this is optional behavior)" << std::endl;
    }

    // State should be completed
    if (!impl->is_completed()) {
        std::cout << "FAILED: Should be completed after resume" << std::endl;
        assert(false);
    }

    // Now call get_result - should return the value immediately, not suspend
    // Because decision is already RESUMED
    void* result = impl->get_result();

    if (result != intrinsics::get_COROUTINE_SUSPENDED()) {
        std::cout << "SUCCESS: Fast path worked, did not suspend" << std::endl;
    } else {
        std::cout << "FAILED: Should not have suspended after resume" << std::endl;
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

    // Resume first (fast path - no dispatch because not suspended yet)
    impl->resume(100, nullptr);

    // Then try to cancel - should fail because state is now CompletedContinuation
    bool cancelled = impl->cancel(std::make_exception_ptr(std::runtime_error("Too late")));

    // In the fast path case, delegate is NOT called (coroutine never suspended).
    // The value is stored in state and retrieved when get_result() is called.
    // What we verify:
    // 1. Cancel should fail (return false) because already completed
    // 2. State should be completed
    // 3. State should NOT be cancelled
    if (!cancelled && impl->is_completed() && !impl->is_cancelled()) {
        std::cout << "SUCCESS: Resume won the race" << std::endl;
    } else {
        std::cout << "FAILED: cancelled=" << cancelled
                  << ", is_completed=" << impl->is_completed()
                  << ", is_cancelled=" << impl->is_cancelled() << std::endl;
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

// Test 5: is_active / is_cancelled / is_completed states
void test_states() {
    std::cout << "Test 5: State Transitions..." << std::endl;

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

// Test 6: try_resume API
void test_try_resume() {
    std::cout << "Test 6: try_resume API..." << std::endl;

    auto mock = std::make_shared<MockContinuation<int>>();
    auto impl = std::make_shared<CancellableContinuationImpl<int>>(mock, 1);

    // First try_resume should succeed
    void* token = impl->try_resume(42);
    if (token != nullptr) {
        impl->complete_resume(token);
        std::cout << "SUCCESS: try_resume returned valid token" << std::endl;
    } else {
        std::cout << "FAILED: try_resume returned nullptr" << std::endl;
        assert(false);
    }

    // Second try_resume should fail
    void* token2 = impl->try_resume(100);
    if (token2 == nullptr) {
        std::cout << "SUCCESS: Second try_resume correctly returned nullptr" << std::endl;
    } else {
        std::cout << "FAILED: Second try_resume should have returned nullptr" << std::endl;
        assert(false);
    }
}

int main() {
    std::cout << "=== CancellableContinuationImpl Test Suite ===" << std::endl;
    std::cout << "(No C++ coroutines - direct API testing)" << std::endl;
    std::cout << std::endl;

    test_successful_resume();
    test_resume_before_suspend();
    test_cancellation();
    test_race_resume_wins();
    test_race_cancel_wins();
    test_states();
    test_try_resume();

    std::cout << std::endl;
    std::cout << "=== All tests passed ===" << std::endl;
    return 0;
}
