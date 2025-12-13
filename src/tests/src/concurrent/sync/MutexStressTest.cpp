// Original: kotlinx-coroutines-core/concurrent/test/sync/MutexStressTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement Mutex, select, withLock
// TODO: Handle @Suppress annotation

namespace kotlinx {
    namespace coroutines {
        namespace sync {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlinx.coroutines.exceptions.*
            // TODO: import kotlinx.coroutines.selects.*
            // TODO: import kotlin.test.*

            class MutexStressTest : public TestBase {
            private:
                int n = 1000 * stress_test_multiplier; // It mostly stresses K/N as JVM Mutex is tested by lincheck

            public:
                // @Test
                // TODO: Convert test annotation
                void test_default_dispatcher() {
                    runTest([&]() {
                        // TODO: suspend function
                        test_body(Dispatchers::Default);
                    });
                }

                // @Test
                // TODO: Convert test annotation
                void test_single_thread_context() {
                    runTest([&]() {
                        // TODO: suspend function
                        auto context = new_single_thread_context("testSingleThreadContext");
                        context.use([&]() {
                            test_body(context);
                        });
                    });
                }

                // @Test
                // TODO: Convert test annotation
                void test_multi_threaded_context_with_single_worker() {
                    runTest([&]() {
                        // TODO: suspend function
                        auto context = new_fixed_thread_pool_context(1, "testMultiThreadedContextWithSingleWorker");
                        context.use([&]() {
                            test_body(context);
                        });
                    });
                }

                // @Test
                // TODO: Convert test annotation
                void test_multi_threaded_context() {
                    runTest([&]() {
                        // TODO: suspend function
                        auto context = new_fixed_thread_pool_context(8, "testMultiThreadedContext");
                        context.use([&]() {
                            test_body(context);
                        });
                    });
                }

                // @Suppress("SuspendFunctionOnCoroutineScope")
            private:
                void test_body(CoroutineDispatcher &dispatcher) {
                    // TODO: suspend function (CoroutineScope scope parameter)
                    int k = 100;
                    int shared = 0;
                    Mutex mutex;
                    std::vector<Job> jobs;
                    for (int i = 0; i < n; ++i) {
                        jobs.push_back(launch(dispatcher, [&]() {
                            // TODO: suspend function
                            for (int j = 0; j < k; ++j) {
                                mutex.lock();
                                shared++;
                                mutex.unlock();
                            }
                        }));
                    }
                    for (auto &job: jobs) {
                        job.join();
                    }
                    assertEquals(n * k, shared);
                }

            public:
                // @Test
                // TODO: Convert test annotation
                void test_stress_unlock_cancel_race() {
                    runTest([&]() {
                        // TODO: suspend function
                        int n = 10'000 * stress_test_multiplier;
                        auto mutex = Mutex(true); // create a locked mutex
                        auto pool = new_single_thread_context("SemaphoreStressTest");
                        pool.use([&]() {
                            for (int i = 0; i < n; ++i) {
                                // Initially, we hold the lock and no one else can `lock`,
                                // otherwise it's a bug.
                                assertTrue(mutex.is_locked());
                                bool job1_entered_critical_section = false;
                                auto job1 = launch(CoroutineStart::UNDISPATCHED, [&]() {
                                    // TODO: suspend function
                                    mutex.lock();
                                    job1_entered_critical_section = true;
                                    mutex.unlock();
                                });
                                // check that `job1` didn't finish the call to `acquire()`
                                assertEquals(false, job1_entered_critical_section);
                                auto job2 = launch(pool, [&]() {
                                    // TODO: suspend function
                                    mutex.unlock();
                                });
                                // Because `job2` executes in a separate thread, this
                                // cancellation races with the call to `unlock()`.
                                job1.cancel_and_join();
                                job2.join();
                                assertFalse(mutex.is_locked());
                                mutex.lock();
                            }
                        });
                    });
                }

                // @Test
                // TODO: Convert test annotation
                void test_stress_unlock_cancel_race_with_select() {
                    runTest([&]() {
                        // TODO: suspend function
                        int n = 10'000 * stress_test_multiplier;
                        auto mutex = Mutex(true); // create a locked mutex
                        auto pool = new_single_thread_context("SemaphoreStressTest");
                        pool.use([&]() {
                            for (int i = 0; i < n; ++i) {
                                // Initially, we hold the lock and no one else can `lock`,
                                // otherwise it's a bug.
                                assertTrue(mutex.is_locked());
                                bool job1_entered_critical_section = false;
                                auto job1 = launch(CoroutineStart::UNDISPATCHED, [&]() {
                                    // TODO: suspend function
                                    select<void>([&](auto &builder) {
                                        builder.on_lock(mutex, [&]() {
                                            job1_entered_critical_section = true;
                                            mutex.unlock();
                                        });
                                    });
                                });
                                // check that `job1` didn't finish the call to `acquire()`
                                assertEquals(false, job1_entered_critical_section);
                                auto job2 = launch(pool, [&]() {
                                    // TODO: suspend function
                                    mutex.unlock();
                                });
                                // Because `job2` executes in a separate thread, this
                                // cancellation races with the call to `unlock()`.
                                job1.cancel_and_join();
                                job2.join();
                                assertFalse(mutex.is_locked());
                                mutex.lock();
                            }
                        });
                    });
                }

                // @Test
                // TODO: Convert test annotation
                void test_should_be_unlocked_on_cancellation() {
                    runTest([&]() {
                        // TODO: suspend function
                        Mutex mutex;
                        int n = 1000 * stress_test_multiplier;
                        for (int i = 0; i < n; ++i) {
                            auto job = launch(Dispatchers::Default, [&]() {
                                // TODO: suspend function
                                mutex.lock();
                                mutex.unlock();
                            });
                            mutex.with_lock([&]() {
                                job.cancel();
                            });
                            job.join();
                            assertFalse(mutex.is_locked());
                        }
                    });
                }
            };
        } // namespace sync
    } // namespace coroutines
} // namespace kotlinx