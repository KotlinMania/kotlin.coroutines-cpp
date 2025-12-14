/**
 * @file test_sync.cpp
 * @brief Tests for Mutex and Semaphore implementations.
 *
 * Tests the lock-free segment-based implementations transliterated from
 * kotlinx-coroutines-core/common/src/sync/Mutex.kt and Semaphore.kt.
 */

#include <iostream>
#include <cassert>
#include <thread>
#include <vector>
#include <atomic>

#include "kotlinx/coroutines/sync/Mutex.hpp"
#include "kotlinx/coroutines/sync/MutexImpl.hpp"
#include "kotlinx/coroutines/sync/Semaphore.hpp"
#include "kotlinx/coroutines/sync/SemaphoreImpl.hpp"

using namespace kotlinx::coroutines::sync;

// Test basic mutex lock/unlock
void test_mutex_basic() {
    std::cout << "test_mutex_basic... ";

    auto mutex = make_mutex(false);

    assert(!mutex->is_locked());
    assert(mutex->try_lock());
    assert(mutex->is_locked());
    mutex->unlock();
    assert(!mutex->is_locked());

    std::cout << "PASSED\n";
}

// Test mutex owner tracking
void test_mutex_owner() {
    std::cout << "test_mutex_owner... ";

    auto mutex = make_mutex(false);
    void* owner1 = reinterpret_cast<void*>(1);
    void* owner2 = reinterpret_cast<void*>(2);

    assert(mutex->try_lock(owner1));
    assert(mutex->holds_lock(owner1));
    assert(!mutex->holds_lock(owner2));
    mutex->unlock(owner1);
    assert(!mutex->holds_lock(owner1));

    std::cout << "PASSED\n";
}

// Test mutex reentrant check (should throw)
void test_mutex_reentrant() {
    std::cout << "test_mutex_reentrant... ";

    auto mutex = make_mutex(false);
    void* owner = reinterpret_cast<void*>(1);

    mutex->try_lock(owner);

    bool threw = false;
    try {
        mutex->try_lock(owner); // Should throw
    } catch (const std::logic_error& e) {
        threw = true;
    }
    assert(threw);

    mutex->unlock(owner);
    std::cout << "PASSED\n";
}

// Test mutex created locked
void test_mutex_created_locked() {
    std::cout << "test_mutex_created_locked... ";

    auto mutex = make_mutex(true);
    assert(mutex->is_locked());
    assert(!mutex->try_lock());

    mutex->unlock();
    assert(!mutex->is_locked());

    std::cout << "PASSED\n";
}

// Test basic semaphore
void test_semaphore_basic() {
    std::cout << "test_semaphore_basic... ";

    auto sem = create_semaphore(2);

    assert(sem->available_permits() == 2);
    assert(sem->try_acquire());
    assert(sem->available_permits() == 1);
    assert(sem->try_acquire());
    assert(sem->available_permits() == 0);
    assert(!sem->try_acquire());

    sem->release();
    assert(sem->available_permits() == 1);
    sem->release();
    assert(sem->available_permits() == 2);

    std::cout << "PASSED\n";
}

// Test semaphore with acquired permits
void test_semaphore_acquired() {
    std::cout << "test_semaphore_acquired... ";

    auto sem = create_semaphore(3, 2);

    assert(sem->available_permits() == 1);
    assert(sem->try_acquire());
    assert(sem->available_permits() == 0);
    assert(!sem->try_acquire());

    std::cout << "PASSED\n";
}

// Test semaphore release overflow (should throw)
void test_semaphore_overflow() {
    std::cout << "test_semaphore_overflow... ";

    auto sem = create_semaphore(1);

    bool threw = false;
    try {
        sem->release(); // More releases than acquires
    } catch (const std::logic_error& e) {
        threw = true;
    }
    assert(threw);

    std::cout << "PASSED\n";
}

// Test concurrent mutex access
void test_mutex_concurrent() {
    std::cout << "test_mutex_concurrent... ";

    auto mutex = make_mutex(false);
    std::atomic<int> counter{0};
    constexpr int iterations = 1000;
    constexpr int num_threads = 4;

    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&mutex, &counter]() {
            for (int i = 0; i < iterations; ++i) {
                mutex->lock();
                int prev = counter.load();
                counter.store(prev + 1);
                mutex->unlock();
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    assert(counter.load() == num_threads * iterations);
    std::cout << "PASSED\n";
}

// Test concurrent semaphore access
void test_semaphore_concurrent() {
    std::cout << "test_semaphore_concurrent... ";

    auto sem = create_semaphore(2);
    std::atomic<int> active{0};
    std::atomic<int> max_active{0};
    constexpr int iterations = 100;
    constexpr int num_threads = 4;

    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&sem, &active, &max_active]() {
            for (int i = 0; i < iterations; ++i) {
                sem->acquire(); // blocking acquire
                int cur = ++active;
                // Track max concurrent
                int prev_max = max_active.load();
                while (cur > prev_max && !max_active.compare_exchange_weak(prev_max, cur));

                // Simulate work
                std::this_thread::yield();

                --active;
                sem->release();
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    // Max active should never exceed semaphore permits
    assert(max_active.load() <= 2);
    std::cout << "PASSED (max_active=" << max_active.load() << ")\n";
}

int main() {
    std::cout << "=== Sync Module Tests ===\n";

    test_mutex_basic();
    test_mutex_owner();
    test_mutex_reentrant();
    test_mutex_created_locked();
    test_semaphore_basic();
    test_semaphore_acquired();
    test_semaphore_overflow();
    test_mutex_concurrent();
    test_semaphore_concurrent();

    std::cout << "\nAll tests passed!\n";
    return 0;
}
