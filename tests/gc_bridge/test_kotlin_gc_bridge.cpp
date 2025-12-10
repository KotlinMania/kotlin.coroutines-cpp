// test_kotlin_gc_bridge.cpp
// Test the Kotlin GC bridge integration
// Compile: clang++ -std=c++17 -I include test_kotlin_gc_bridge.cpp -o test_gc_bridge

#include "kotlinx/coroutines/KotlinGCBridge.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#ifdef __APPLE__
#include <malloc/malloc.h>  // For malloc_zone_statistics on macOS
#else
#include <malloc.h>
#endif

using namespace std::chrono;
using namespace kotlinx::coroutines;

// Track memory allocations
struct MemoryStats {
    size_t allocated_bytes = 0;
    size_t allocation_count = 0;
    
    void print() const {
        std::cout << "Memory: " << (allocated_bytes / 1024.0 / 1024.0) 
                  << " MB in " << allocation_count << " allocations\n";
    }
};

MemoryStats get_memory_stats() {
    MemoryStats stats;
    
#ifdef __APPLE__
    // macOS - use malloc zone statistics
    malloc_statistics_t zone_stats;
    malloc_zone_statistics(nullptr, &zone_stats);
    stats.allocated_bytes = zone_stats.size_in_use;
    stats.allocation_count = zone_stats.blocks_in_use;
#else
    // Other platforms - would need different approach
    stats.allocated_bytes = 0;
    stats.allocation_count = 0;
#endif
    
    return stats;
}

// Simulate heavy work with allocations
void do_heavy_work_with_allocations(int iterations, bool print_progress = false) {
    std::vector<std::vector<int>> data;
    
    for (int i = 0; i < iterations; i++) {
        // Allocate some memory
        data.push_back(std::vector<int>(1000, i));
        
        // Do some computation
        int sum = 0;
        for (const auto& vec : data) {
            for (int val : vec) {
                sum += val;
            }
        }
        
        if (print_progress && i % 100 == 0) {
            std::cout << "  Iteration " << i << "/" << iterations 
                      << " (sum=" << sum << ")\n";
        }
        
        // Clear some data to avoid OOM
        if (data.size() > 100) {
            data.erase(data.begin(), data.begin() + 50);
        }
    }
}

// Test 1: Without GC bridge (normal C++)
void test_without_gc_bridge() {
    std::cout << "\n=== Test 1: Without GC Bridge ===\n";
    
    auto start_mem = get_memory_stats();
    auto start_time = high_resolution_clock::now();
    
    std::cout << "Starting heavy work (no GC coordination)...\n";
    do_heavy_work_with_allocations(500);
    
    auto end_time = high_resolution_clock::now();
    auto end_mem = get_memory_stats();
    
    auto duration_ms = duration_cast<milliseconds>(end_time - start_time).count();
    
    std::cout << "Completed in " << duration_ms << " ms\n";
    std::cout << "Memory before: "; start_mem.print();
    std::cout << "Memory after:  "; end_mem.print();
    std::cout << "Memory delta:  " 
              << ((end_mem.allocated_bytes - start_mem.allocated_bytes) / 1024.0 / 1024.0) 
              << " MB\n";
}

// Test 2: With GC bridge - switched to Native state
void test_with_gc_bridge_native() {
    std::cout << "\n=== Test 2: With GC Bridge (Native State) ===\n";
    
    auto start_mem = get_memory_stats();
    auto start_time = high_resolution_clock::now();
    
    std::cout << "Creating KotlinNativeStateGuard (switching to Native)...\n";
    {
        KotlinNativeStateGuard guard;
        
        std::cout << "Starting heavy work (in Native state)...\n";
        do_heavy_work_with_allocations(500);
        
        std::cout << "Guard going out of scope (switching back to Runnable)...\n";
    }
    
    auto end_time = high_resolution_clock::now();
    auto end_mem = get_memory_stats();
    
    auto duration_ms = duration_cast<milliseconds>(end_time - start_time).count();
    
    std::cout << "Completed in " << duration_ms << " ms\n";
    std::cout << "Memory before: "; start_mem.print();
    std::cout << "Memory after:  "; end_mem.print();
    std::cout << "Memory delta:  " 
              << ((end_mem.allocated_bytes - start_mem.allocated_bytes) / 1024.0 / 1024.0) 
              << " MB\n";
}

// Test 3: With periodic safepoint checks
void test_with_safepoint_checks() {
    std::cout << "\n=== Test 3: With Safepoint Checks ===\n";
    
    auto start_mem = get_memory_stats();
    auto start_time = high_resolution_clock::now();
    
    std::cout << "Starting work with periodic safepoint checks...\n";
    {
        KotlinNativeStateGuard guard;
        
        for (int batch = 0; batch < 5; batch++) {
            std::cout << "  Batch " << (batch + 1) << "/5...\n";
            do_heavy_work_with_allocations(100);
            
            // Check safepoint between batches
            std::cout << "  Checking safepoint...\n";
            check_safepoint();
        }
    }
    
    auto end_time = high_resolution_clock::now();
    auto end_mem = get_memory_stats();
    
    auto duration_ms = duration_cast<milliseconds>(end_time - start_time).count();
    
    std::cout << "Completed in " << duration_ms << " ms\n";
    std::cout << "Memory before: "; start_mem.print();
    std::cout << "Memory after:  "; end_mem.print();
}

// Test 4: Multi-threaded scenario
void test_multithreaded() {
    std::cout << "\n=== Test 4: Multi-threaded ===\n";
    
    std::atomic<int> completed{0};
    const int num_threads = 4;
    
    auto start_time = high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back([i, &completed]() {
            std::cout << "Thread " << i << " starting...\n";
            
            {
                KotlinNativeStateGuard guard;
                do_heavy_work_with_allocations(200);
            }
            
            completed++;
            std::cout << "Thread " << i << " completed\n";
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end_time = high_resolution_clock::now();
    auto duration_ms = duration_cast<milliseconds>(end_time - start_time).count();
    
    std::cout << "All " << num_threads << " threads completed in " 
              << duration_ms << " ms\n";
}

// Test 5: Switching between states (simulating callbacks)
void test_state_switching() {
    std::cout << "\n=== Test 5: State Switching (Simulated Callbacks) ===\n";
    
    auto start_time = high_resolution_clock::now();
    
    std::cout << "Starting with multiple state switches...\n";
    
    for (int i = 0; i < 5; i++) {
        std::cout << "  Cycle " << (i + 1) << "/5:\n";
        
        // Switch to Native
        std::cout << "    -> Native state\n";
        Kotlin_mm_switchThreadStateNative();
        do_heavy_work_with_allocations(50);
        
        // Switch to Runnable (simulate Kotlin callback)
        std::cout << "    -> Runnable state (simulated callback)\n";
        Kotlin_mm_switchThreadStateRunnable();
        std::this_thread::sleep_for(milliseconds(10));
        
        // Back to Native
        std::cout << "    -> Native state\n";
        Kotlin_mm_switchThreadStateNative();
        do_heavy_work_with_allocations(50);
        
        // Back to Runnable
        std::cout << "    -> Runnable state\n";
        Kotlin_mm_switchThreadStateRunnable();
        std::this_thread::sleep_for(milliseconds(10));
    }
    
    auto end_time = high_resolution_clock::now();
    auto duration_ms = duration_cast<milliseconds>(end_time - start_time).count();
    
    std::cout << "Completed in " << duration_ms << " ms\n";
}

int main() {
    std::cout << "=================================================\n";
    std::cout << "Kotlin GC Bridge Integration Test\n";
    std::cout << "=================================================\n";
    
    // Check if Kotlin Native runtime is available
    std::cout << "\nKotlin Native runtime available: " 
              << (is_kotlin_native_runtime_available() ? "YES" : "NO") << "\n";
    
    if (!is_kotlin_native_runtime_available()) {
        std::cout << "\nNOTE: Running in standalone mode.\n";
        std::cout << "      GC bridge functions are no-ops.\n";
        std::cout << "      To test with Kotlin Native, call this from Kotlin.\n";
    }
    
    // Run tests
    test_without_gc_bridge();
    test_with_gc_bridge_native();
    test_with_safepoint_checks();
    test_multithreaded();
    test_state_switching();
    
    std::cout << "\n=================================================\n";
    std::cout << "All tests completed!\n";
    std::cout << "=================================================\n";
    
    return 0;
}
