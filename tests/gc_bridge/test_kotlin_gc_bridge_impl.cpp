// test_kotlin_gc_bridge_impl.cpp
// C++ implementation that Kotlin will call
// Compile as shared library for Kotlin Native cinterop

#include "kotlinx/coroutines/KotlinGCBridge.hpp"
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>

#ifdef __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

using namespace kotlinx::coroutines;
using namespace std::chrono;

// Helper: do some heavy C++ work
void do_cpp_work(int iterations) {
    std::vector<std::vector<int>> data;
    
    for (int i = 0; i < iterations; i++) {
        data.push_back(std::vector<int>(1000, i));
        
        int sum = 0;
        for (const auto& vec : data) {
            for (int val : vec) {
                sum += val;
            }
        }
        
        // Keep memory bounded
        if (data.size() > 100) {
            data.erase(data.begin(), data.begin() + 50);
        }
    }
}

// Test 1: C++ work WITHOUT GC guard
// cinterop will insert automatic thread state switches
extern "C" void test_cpp_without_guard() {
    std::cout << "[C++] Starting work without GC guard\n";
    std::cout << "[C++] (cinterop will auto-switch to Native state)\n";
    
    auto start = high_resolution_clock::now();
    do_cpp_work(500);
    auto end = high_resolution_clock::now();
    
    auto duration_ms = duration_cast<milliseconds>(end - start).count();
    std::cout << "[C++] Completed in " << duration_ms << " ms\n";
}

// Test 2: C++ work WITH GC guard (manual state control)
// Using @GCUnsafeCall, so we control thread state manually
extern "C" void test_cpp_with_guard() {
    std::cout << "[C++] Starting work with GC guard\n";
    
    // We enter in Runnable state (caller's state)
    std::cout << "[C++] Switching to Native state...\n";
    
    {
        KotlinNativeStateGuard guard;
        
        std::cout << "[C++] In Native state, doing work...\n";
        auto start = high_resolution_clock::now();
        do_cpp_work(500);
        auto end = high_resolution_clock::now();
        
        auto duration_ms = duration_cast<milliseconds>(end - start).count();
        std::cout << "[C++] Work completed in " << duration_ms << " ms\n";
        std::cout << "[C++] Switching back to Runnable state...\n";
    }
    
    std::cout << "[C++] Back in Runnable state\n";
}

// Test 3: C++ work with periodic safepoint checks
extern "C" void test_cpp_with_safepoints() {
    std::cout << "[C++] Starting work with safepoint checks\n";
    
    {
        KotlinNativeStateGuard guard;
        
        for (int batch = 0; batch < 5; batch++) {
            std::cout << "[C++] Batch " << (batch + 1) << "/5...\n";
            do_cpp_work(100);
            
            // Periodically check if GC wants to pause us
            std::cout << "[C++] Checking safepoint...\n";
            check_safepoint();
        }
    }
    
    std::cout << "[C++] All batches completed\n";
}

// Test 4: Get C++ memory info
extern "C" int64_t test_cpp_get_memory_info() {
#ifdef __APPLE__
    malloc_statistics_t stats;
    malloc_zone_statistics(nullptr, &stats);
    return static_cast<int64_t>(stats.size_in_use);
#else
    return 0;
#endif
}

// Test 5: Long-running operation that simulates real work
extern "C" void test_cpp_long_running(int duration_seconds) {
    std::cout << "[C++] Starting long-running operation (" 
              << duration_seconds << " seconds)\n";
    
    {
        KotlinNativeStateGuard guard;
        
        auto end_time = steady_clock::now() + seconds(duration_seconds);
        int iterations = 0;
        
        while (steady_clock::now() < end_time) {
            do_cpp_work(10);
            iterations++;
            
            // Check safepoint every 100 iterations
            if (iterations % 100 == 0) {
                check_safepoint();
                std::cout << "[C++] Still working... (iteration " 
                          << iterations << ")\n";
            }
        }
        
        std::cout << "[C++] Completed " << iterations << " iterations\n";
    }
}

// Test 6: Callback to Kotlin (requires state switching)
extern "C" void test_cpp_with_callback(void (*kotlin_callback)(int)) {
    std::cout << "[C++] Starting work with callbacks\n";
    
    {
        KotlinNativeStateGuard guard;
        
        for (int i = 0; i < 5; i++) {
            std::cout << "[C++] Doing C++ work (iteration " << i << ")...\n";
            do_cpp_work(50);
            
            // Need to call Kotlin? Switch back to Runnable
            std::cout << "[C++] Switching to Runnable for callback...\n";
            Kotlin_mm_switchThreadStateRunnable();
            
            kotlin_callback(i);
            
            // Back to Native
            std::cout << "[C++] Switching back to Native...\n";
            Kotlin_mm_switchThreadStateNative();
        }
    }
    
    std::cout << "[C++] All callbacks completed\n";
}

// Export info about GC bridge status
extern "C" bool test_cpp_is_gc_bridge_available() {
    return is_kotlin_native_runtime_available();
}

// Stress test: allocate and free in a loop
extern "C" void test_cpp_memory_stress(int iterations) {
    std::cout << "[C++] Starting memory stress test (" 
              << iterations << " iterations)\n";
    
    {
        KotlinNativeStateGuard guard;
        
        for (int i = 0; i < iterations; i++) {
            // Allocate 10MB
            std::vector<char> buffer(10 * 1024 * 1024);
            
            // Do something with it
            for (size_t j = 0; j < buffer.size(); j += 1024) {
                buffer[j] = static_cast<char>(i % 256);
            }
            
            // Check safepoint periodically
            if (i % 10 == 0) {
                check_safepoint();
            }
            
            // buffer is freed here
        }
        
        std::cout << "[C++] Stress test completed\n";
    }
}
