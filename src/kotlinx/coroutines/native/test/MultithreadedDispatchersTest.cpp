// Original file: kotlinx-coroutines-core/native/test/MultithreadedDispatchersTest.kt
// TODO: Remove or convert import statements
// TODO: Convert @Test annotation to appropriate test framework
// TODO: Convert atomic operations and channels
// TODO: Convert suspend functions and coroutine builders
// TODO: Handle Worker and SynchronizedObject

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.atomicfu.*
        // TODO: import kotlinx.coroutines.channels.*
        // TODO: import kotlinx.coroutines.internal.*
        // TODO: import kotlin.native.concurrent.*
        // TODO: import kotlin.test.*
        // TODO: import kotlin.time.Duration.Companion.seconds

        class BlockingBarrier {
        private:
            int n_;
            std::atomic<int> counter_;
            Channel<void> wake_up_;

        public:
            BlockingBarrier(int n) : n_(n), counter_(0), wake_up_(n - 1) {
            }

            void await() {
                int count = counter_.fetch_add(1) + 1;
                if (count == n_) {
                    for (int i = 0; i < n_ - 1; ++i) {
                        // TODO: runBlocking is a suspend function
                        // runBlocking {
                        wake_up_.send();
                        // }
                    }
                } else if (count < n_) {
                    // TODO: runBlocking is a suspend function
                    // runBlocking {
                    wake_up_.receive();
                    // }
                }
            }
        };

        class MultithreadedDispatchersTest {
        public:
            /**
     * Test that [newFixedThreadPoolContext] does not allocate more dispatchers than it needs to.
     * Incidentally also tests that it will allocate enough workers for its needs. Otherwise, the test will hang.
     */
            // TODO: @Test
            void test_not_allocating_extra_dispatchers() {
                BlockingBarrier barrier(2);
                SynchronizedObject lock;
                auto spin = [&](std::set<Worker *> &set) {
                    // TODO: suspend function
                    for (int i = 0; i < 100; ++i) {
                        synchronized(lock)
                        {
                            set.insert(Worker::current);
                        }
                        delay(1);
                    }
                };
                auto dispatcher = new_fixed_thread_pool_context(64, "test");
                try {
                    // TODO: runBlocking is a suspend function
                    // runBlocking {
                    std::set<Worker *> encountered_workers;
                    auto coroutine1 = launch(dispatcher, [&]() {
                        barrier.await();
                        spin(encountered_workers);
                    });
                    auto coroutine2 = launch(dispatcher, [&]() {
                        barrier.await();
                        spin(encountered_workers);
                    });
                    std::vector{coroutine1, coroutine2}.join_all();
                    assert(encountered_workers.size() == 2);
                    // }
                }
                finally{
                    dispatcher.close();
        
                }
            }

            /**
     * Test that [newSingleThreadContext] will not wait for the cancelled scheduled coroutines before closing.
     */
            // TODO: @Test
            void timeouts_not_preventing_closing() {
                // TODO: runBlocking is a suspend function
                // runBlocking {
                auto dispatcher = WorkerDispatcher("test");
                with_context(dispatcher, [&]() {
                    // TODO: withTimeout is a suspend function
                    with_timeout(5000ms, [&]() {
                    });
                });
                with_timeout(1000ms, [&]() {
                    dispatcher.close(); // should not wait for the timeout
                    yield();
                });
                // }
            }
        };
    } // namespace coroutines
} // namespace kotlinx