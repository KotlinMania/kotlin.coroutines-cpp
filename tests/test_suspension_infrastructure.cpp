#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <iostream>
#include <cassert>

using namespace kotlinx::coroutines::intrinsics;

// Test the basic suspension infrastructure
int main() {
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
    return 0;
}