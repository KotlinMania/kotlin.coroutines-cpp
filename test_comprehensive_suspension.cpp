#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::intrinsics;

// Mock continuation for testing
class MockContinuation : public Continuation<int> {
public:
    int result_value = 0;
    std::exception_ptr exception_ptr;
    bool resumed = false;
    
    void resume_with(Result<int> result) override {
        if (result.is_success()) {
            result_value = result.get_or_throw();
        } else {
            exception_ptr = result.exception_or_null();
        }
        resumed = true;
    }
    
    std::shared_ptr<CoroutineContext> get_context() const override {
        return nullptr; // Empty context for testing
    }
};

void test_suspension_infrastructure() {
    std::cout << "Testing suspension infrastructure..." << std::endl;
    
    // Test 1: COROUTINE_SUSPENDED marker
    void* suspended = get_COROUTINE_SUSPENDED();
    std::cout << "COROUTINE_SUSPENDED marker address: " << suspended << std::endl;
    
    // Test 2: is_coroutine_suspended function
    bool is_suspended = is_coroutine_suspended(suspended);
    assert(is_suspended == true);
    std::cout << "✓ is_coroutine_suspended correctly identifies suspended marker" << std::endl;
    
    // Test 3: Non-suspended values
    int some_value = 42;
    bool not_suspended = is_coroutine_suspended(&some_value);
    assert(not_suspended == false);
    std::cout << "✓ is_coroutine_suspended correctly identifies non-suspended values" << std::endl;
    
    // Test 4: Null pointer
    bool null_not_suspended = is_coroutine_suspended(nullptr);
    assert(null_not_suspended == false);
    std::cout << "✓ is_coroutine_suspended correctly identifies nullptr" << std::endl;
    
    std::cout << "All suspension infrastructure tests passed!" << std::endl;
}

void test_cancellable_continuation_suspension() {
    std::cout << "\nTesting CancellableContinuation suspension..." << std::endl;
    
    auto mock_continuation = std::make_shared<MockContinuation>();
    
    // Test synchronous completion (no suspension)
    void* result = suspend_cancellable_coroutine<int>(
        [](CancellableContinuation<int>& cont) {
            // Immediately resume with a value
            cont.resume(42, nullptr);
        },
        mock_continuation.get()
    );
    
    // Should not be suspended since we resumed immediately
    assert(result != COROUTINE_SUSPENDED);
    assert(mock_continuation->resumed == true);
    assert(mock_continuation->result_value == 42);
    std::cout << "✓ Synchronous completion works correctly" << std::endl;
    
    // Test asynchronous completion (suspension)
    mock_continuation->resumed = false;
    mock_continuation->result_value = 0;
    
    std::shared_ptr<CancellableContinuation<int>> captured_cont = nullptr;
    result = suspend_cancellable_coroutine<int>(
        [&captured_cont](CancellableContinuation<int>& cont) {
            // Store continuation for later resumption
            captured_cont = std::static_pointer_cast<CancellableContinuation<int>>(
                static_cast<CancellableContinuationImpl<int>*>(&cont)->shared_from_this()
            );
            // Don't resume immediately - should suspend
        },
        mock_continuation.get()
    );
    
    // Should be suspended
    assert(result == COROUTINE_SUSPENDED);
    assert(mock_continuation->resumed == false);
    std::cout << "✓ Asynchronous suspension works correctly" << std::endl;
    
    // Resume after delay
    std::thread([captured_cont, mock_continuation]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        captured_cont->resume(123, nullptr);
    }).detach();
    
    // Wait for completion
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    assert(mock_continuation->resumed == true);
    assert(mock_continuation->result_value == 123);
    std::cout << "✓ Asynchronous resumption works correctly" << std::endl;
}

void test_proper_suspension_marker_usage() {
    std::cout << "\nTesting proper suspension marker usage..." << std::endl;
    
    // Test that the marker is consistent
    void* marker1 = get_COROUTINE_SUSPENDED();
    void* marker2 = get_COROUTINE_SUSPENDED();
    assert(marker1 == marker2);
    std::cout << "✓ COROUTINE_SUSPENDED marker is consistent" << std::endl;
    
    // Test that the marker is not null
    assert(marker1 != nullptr);
    std::cout << "✓ COROUTINE_SUSPENDED marker is not null" << std::endl;
    
    // Test that the marker is a valid pointer
    assert(is_coroutine_suspended(marker1));
    std::cout << "✓ COROUTINE_SUSPENDED marker passes validation" << std::endl;
}

int main() {
    try {
        test_suspension_infrastructure();
        test_cancellable_continuation_suspension();
        test_proper_suspension_marker_usage();
        
        std::cout << "\n🎉 All suspension tests passed!" << std::endl;
        std::cout << "The suspension infrastructure is working correctly." << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}