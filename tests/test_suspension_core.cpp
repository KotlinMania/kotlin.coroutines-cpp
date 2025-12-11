#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <iostream>

int main() {
    std::cout << "Testing suspension infrastructure fixes..." << std::endl;
    
    // Test 1: Verify COROUTINE_SUSPENDED marker works
    void* suspended_marker = kotlinx::coroutines::intrinsics::get_COROUTINE_SUSPENDED();
    std::cout << "✓ COROUTINE_SUSPENDED marker: " << suspended_marker << std::endl;
    
    // Test 2: Test is_coroutine_suspended function
    bool suspended_check = kotlinx::coroutines::intrinsics::is_coroutine_suspended(suspended_marker);
    std::cout << "✓ is_coroutine_suspended(COROUTINE_SUSPENDED): " 
              << (suspended_check ? "true" : "false") << std::endl;
    
    // Test 3: Test that non-suspended values return false
    void* fake_value = (void*)0x12345678;
    bool not_suspended = kotlinx::coroutines::intrinsics::is_coroutine_suspended(fake_value);
    std::cout << "✓ is_coroutine_suspended(fake_value): " 
              << (not_suspended ? "true" : "false") << std::endl;
    
    // Test 4: Test consistency - multiple calls should return same result
    void* marker1 = kotlinx::coroutines::intrinsics::get_COROUTINE_SUSPENDED();
    void* marker2 = kotlinx::coroutines::intrinsics::get_COROUTINE_SUSPENDED();
    std::cout << "✓ Marker consistency: " << (marker1 == marker2 ? "true" : "false") << std::endl;
    
    // Test 5: Test thread safety (basic check)
    bool suspended_check2 = kotlinx::coroutines::intrinsics::is_coroutine_suspended(marker1);
    std::cout << "✓ Multiple calls consistent: " 
              << (suspended_check == suspended_check2 ? "true" : "false") << std::endl;
    
    std::cout << "\n=== Core suspension infrastructure tests completed ===" << std::endl;
    
    if (suspended_check && !not_suspended && (marker1 == marker2) && (suspended_check == suspended_check2)) {
        std::cout << "✅ All core suspension tests PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some tests FAILED!" << std::endl;
        return 1;
    }
}