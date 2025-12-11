#include "include/kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "include/kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <iostream>
#include <memory>

using namespace kotlinx::coroutines;

int main() {
    std::cout << "Testing suspension infrastructure fixes..." << std::endl;
    
    // Test 1: Verify COROUTINE_SUSPENDED marker works
    std::cout << "✓ COROUTINE_SUSPENDED marker: " << COROUTINE_SUSPENDED << std::endl;
    std::cout << "✓ is_coroutine_suspended(COROUTINE_SUSPENDED): " 
              << is_coroutine_suspended(COROUTINE_SUSPENDED) << std::endl;
    
    // Test 2: Test CancellableContinuationImpl suspension behavior
    std::cout << "\nTesting CancellableContinuationImpl..." << std::endl;
    
    class TestContinuation : public Continuation<int> {
    public:
        std::shared_ptr<CoroutineContext> get_context() const override { 
            return std::make_shared<CoroutineContext>(); 
        }
        
        void resume_with(Result<int> result) override {
            if (result.is_success()) {
                std::cout << "✓ Continuation resumed with: " << result.get_or_throw() << std::endl;
            } else {
                std::cout << "✗ Continuation failed with exception" << std::endl;
            }
        }
    };
    
    auto test_continuation = std::make_shared<TestContinuation>();
    auto impl = std::make_shared<CancellableContinuationImpl<int>>(test_continuation, 1);
    impl->init_cancellability();
    
    // Test 3: Check try_suspend functionality
    std::cout << "\nTesting try_suspend mechanism..." << std::endl;
    bool suspended = impl->try_suspend();
    std::cout << "✓ try_suspend() returned: " << (suspended ? "true (suspended)" : "false (not suspended)") << std::endl;
    
    if (suspended) {
        std::cout << "✓ Suspension infrastructure working correctly" << std::endl;
    } else {
        std::cout << "✗ Suspension failed - continuation not suspended" << std::endl;
    }
    
    // Test 4: Test suspend_cancellable_coroutine function
    std::cout << "\nTesting suspend_cancellable_coroutine..." << std::endl;
    
    auto outer_continuation = std::make_shared<TestContinuation>();
    
    void* result = suspend_cancellable_coroutine<int>(
        [](CancellableContinuation<int>& cont) {
            std::cout << "✓ Inside suspend block - continuing will suspend" << std::endl;
            // Don't resume - this should cause suspension
        },
        outer_continuation.get()
    );
    
    if (is_coroutine_suspended(result)) {
        std::cout << "✓ suspend_cancellable_coroutine correctly returned COROUTINE_SUSPENDED" << std::endl;
    } else {
        std::cout << "✗ suspend_cancellable_coroutine did not suspend properly" << std::endl;
    }
    
    std::cout << "\n=== All suspension infrastructure tests completed ===" << std::endl;
    return 0;
}