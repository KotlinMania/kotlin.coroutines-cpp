/**
 * Transliterated from: kotlinx-coroutines-core/native/test/MultithreadedDispatchersTest.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 *   imports: kotlinx.atomicfu.*, kotlinx.coroutines.channels.*, kotlinx.coroutines.internal.*,
 *            kotlin.native.concurrent.*, kotlin.test.*, kotlin.time.Duration.Companion.seconds
 *
 * Smoke tests for the multithreaded dispatchers: that newFixedThreadPoolContext does not
 * allocate more dispatchers than required, and that newSingleThreadContext does not block
 * on cancelled scheduled coroutines during close. The Kotlin `runBlocking { ... }`
 * wrapper drives the suspending body to completion; the C++ port uses the Continuation
 * ABI directly via `run_blocking(...)` which performs the same in-place suspension drive.
 */

#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/MultithreadedDispatchers.hpp"
#include "kotlinx/coroutines/Timeout.hpp"
#include "kotlinx/coroutines/Yield.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/internal/Synchronized.common.cpp"

#include <atomic>
#include <cassert>
#include <chrono>
#include <set>

namespace kotlinx::coroutines {

/**
 * Upstream:
 *   private class BlockingBarrier(val n: Int) {
 *       val counter = atomic(0)
 *       val wakeUp = Channel<Unit>(n - 1)
 *       fun await() { ... }
 *   }
 */
class BlockingBarrier {
public:
    explicit BlockingBarrier(int n)
        : n_(n), counter_(0), wake_up_(channels::make_channel<Unit>(n - 1)) {}

    void await() {
        int count = counter_.fetch_add(1) + 1;
        if (count == n_) {
            for (int i = 0; i < n_ - 1; ++i) {
                run_blocking([&]() { wake_up_->send(Unit{}, nullptr); });
            }
        } else if (count < n_) {
            run_blocking([&]() { wake_up_->receive(nullptr); });
        }
    }

private:
    int n_;
    std::atomic<int> counter_;
    std::shared_ptr<channels::Channel<Unit>> wake_up_;
};

class MultithreadedDispatchersTest {
public:
    /**
     * Upstream:
     *   @Test
     *   fun testNotAllocatingExtraDispatchers() { ... }
     */
    void test_not_allocating_extra_dispatchers() {
        BlockingBarrier barrier(2);
        internal::SynchronizedObject lock;
        auto spin = [&](std::set<Worker*>& set) {
            for (int i = 0; i < 100; ++i) {
                internal::synchronized<void>(&lock, [&]() {
                    set.insert(Worker::current());
                });
                delay(1, nullptr);
            }
        };
        auto dispatcher = new_fixed_thread_pool_context(64, "test");
        try {
            run_blocking([&]() {
                std::set<Worker*> encountered_workers;
                auto coroutine1 = launch(dispatcher, [&]() {
                    barrier.await();
                    spin(encountered_workers);
                });
                auto coroutine2 = launch(dispatcher, [&]() {
                    barrier.await();
                    spin(encountered_workers);
                });
                join_all({coroutine1, coroutine2});
                assert(encountered_workers.size() == 2);
            });
        } catch (...) {
            dispatcher->close();
            throw;
        }
        dispatcher->close();
    }

    /**
     * Upstream:
     *   @Test
     *   fun timeoutsNotPreventingClosing(): Unit = runBlocking { ... }
     */
    void timeouts_not_preventing_closing() {
        run_blocking([&]() {
            auto dispatcher = std::make_shared<WorkerDispatcher>("test");
            with_context(dispatcher, [&]() {
                with_timeout(std::chrono::seconds(5), [&]() {});
            });
            with_timeout(std::chrono::seconds(1), [&]() {
                dispatcher->close();  // should not wait for the timeout
                yield(nullptr);
            });
        });
    }
};

} // namespace kotlinx::coroutines
