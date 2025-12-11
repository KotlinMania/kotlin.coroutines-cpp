#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

using namespace kotlinx::coroutines::intrinsics;

void test_suspension_marker_consistency() {
    std::cout << "Testing suspension marker consistency..." << std::endl;
    
    // Test that the marker is consistent across multiple calls
    void* marker1 = get_COROUTINE_SUSPENDED();
    void* marker2 = get_COROUTINE_SUSPENDED();
    void* marker3 = get_COROUTINE_SUSPENDED();
    
    assert(marker1 == marker2);
    assert(marker2 == marker3);
    assert(marker1 != nullptr);
    
    std::cout << "✓ COROUTINE_SUSPENDED marker is consistent: " << marker1 << std::endl;
}

void test_suspension_detection() {
    std::cout << "\nTesting suspension detection..." << std::endl;
    
    void* suspended = get_COROUTINE_SUSPENDED();
    
    // Test detection of suspended marker
    assert(is_coroutine_suspended(suspended) == true);
    std::cout << "✓ Correctly identifies COROUTINE_SUSPENDED marker" << std::endl;
    
    // Test detection of regular pointers
    int value = 42;
    assert(is_coroutine_suspended(&value) == false);
    std::cout << "✓ Correctly identifies regular pointers as not suspended" << std::endl;
    
    // Test detection of null pointer
    assert(is_coroutine_suspended(nullptr) == false);
    std::cout << "✓ Correctly identifies nullptr as not suspended" << std::endl;
    
    // Test detection of heap allocated values
    int* heap_value = new int(123);
    assert(is_coroutine_suspended(heap_value) == false);
    std::cout << "✓ Correctly identifies heap pointers as not suspended" << std::endl;
    delete heap_value;
}

void test_thread_safety() {
    std::cout << "\nTesting thread safety..." << std::endl;
    
    // Test that the marker is the same across threads
    void* main_thread_marker = get_COROUTINE_SUSPENDED();
    void* other_thread_marker = nullptr;
    
    std::thread t([&other_thread_marker]() {
        other_thread_marker = get_COROUTINE_SUSPENDED();
        
        // Test detection in other thread
        assert(is_coroutine_suspended(other_thread_marker) == true);
    });
    
    t.join();
    
    assert(main_thread_marker == other_thread_marker);
    std::cout << "✓ COROUTINE_SUSPENDED marker is consistent across threads" << std::endl;
}

void test_performance_characteristics() {
    std::cout << "\nTesting performance characteristics..." << std::endl;
    
    void* suspended = get_COROUTINE_SUSPENDED();
    const int iterations = 1000000;
    
    // Test speed of suspension detection
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        volatile bool result = is_coroutine_suspended(suspended);
        (void)result; // Prevent optimization
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double ops_per_second = (double)iterations / duration.count() * 1000000;
    
    std::cout << "✓ Suspension detection performance: " << ops_per_second << " ops/sec" << std::endl;
    std::cout << "  (Should be very fast - just a pointer comparison)" << std::endl;
}

int main() {
    std::cout << "=== kotlinx.coroutines-cpp Suspension Infrastructure Test ===" << std::endl;
    
    try {
        test_suspension_marker_consistency();
        test_suspension_detection();
        test_thread_safety();
        test_performance_characteristics();
        
        std::cout << "\n🎉 All suspension infrastructure tests passed!" << std::endl;
        std::cout << "\nSummary:" << std::endl;
        std::cout << "- COROUTINE_SUSPENDED marker is consistent and thread-safe" << std::endl;
        std::cout << "- Suspension detection works correctly" << std::endl;
        std::cout << "- Performance is optimal (pointer comparison)" << std::endl;
        std::cout << "- Infrastructure is ready for coroutine suspension" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}