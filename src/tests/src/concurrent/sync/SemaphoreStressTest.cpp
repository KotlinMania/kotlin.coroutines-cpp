// Original: kotlinx-coroutines-core/concurrent/test/sync/SemaphoreStressTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement Semaphore, withPermit

namespace kotlinx {
    namespace coroutines {
        namespace sync {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlinx.coroutines.exceptions.*
            // TODO: import kotlin.test.*

            class SemaphoreStressTest : public TestBase {
            private:
                int iterations = (is_native ? 1'000 : 10'000) * stress_test_multiplier;

            public:
                // @Test
                // TODO: Convert test annotation
                void test_stress_test_as_mutex() {
                    runTest([&]() {
                        // TODO: suspend function
                        int n = iterations;
                        int k = 100;
                        int shared = 0;
                        Semaphore semaphore(1);
                        std::vector<Job> jobs;
                        for (int i = 0; i < n; ++i) {
                            jobs.push_back(launch(Dispatchers::Default, [&]() {
                                // TODO: suspend function
                                for (int j = 0; j < k; ++j) {
                                    semaphore.acquire();
                                    shared++;
                                    semaphore.release();
                                }
                            }));
                        }
                        for (auto &job: jobs) {
                            job.join();
                        }
                        assertEquals(n * k, shared);
                    });
                }

                // @Test
                // TODO: Convert test annotation
                void test_stress() {
                    runTest([&]() {
                        // TODO: suspend function
                        int n = iterations;
                        int k = 100;
                        Semaphore semaphore(10);
                        std::vector<Job> jobs;
                        for (int i = 0; i < n; ++i) {
                            jobs.push_back(launch(Dispatchers::Default, [&]() {
                                // TODO: suspend function
                                for (int j = 0; j < k; ++j) {
                                    semaphore.acquire();
                                    semaphore.release();
                                }
                            }));
                        }
                        for (auto &job: jobs) {
                            job.join();
                        }
                    });
                }

                // @Test
                // TODO: Convert test annotation
                void test_stress_as_mutex() {
                    runTest([&]() {
                        // TODO: suspend function
                        runBlocking(Dispatchers::Default, [&]() {
                            // TODO: suspend function
                            int n = iterations;
                            int k = 100;
                            int shared = 0;
                            Semaphore semaphore(1);
                            std::vector<Job> jobs;
                            for (int i = 0; i < n; ++i) {
                                jobs.push_back(launch([&]() {
                                    // TODO: suspend function
                                    for (int j = 0; j < k; ++j) {
                                        semaphore.acquire();
                                        shared++;
                                        semaphore.release();
                                    }
                                }));
                            }
                            for (auto &job: jobs) {
                                job.join();
                            }
                            assertEquals(n * k, shared);
                        });
                    });
                }

                // @Test
                // TODO: Convert test annotation
                void test_stress_cancellation() {
                    runTest([&]() {
                        // TODO: suspend function
                        int n = iterations;
                        Semaphore semaphore(1);
                        semaphore.acquire();
                        for (int i = 0; i < n; ++i) {
                            auto job = launch(Dispatchers::Default, [&]() {
                                // TODO: suspend function
                                semaphore.acquire();
                            });
                            yield();
                            job.cancel_and_join();
                        }
                        assertEquals(0, semaphore.available_permits());
                        semaphore.release();
                        assertEquals(1, semaphore.available_permits());
                    });
                }

                /**
     * This checks if repeated releases that race with cancellations put
     * the semaphore into an incorrect state where permits are leaked.
     */
                // @Test
                // TODO: Convert test annotation
                void test_stress_release_cancel_race() {
                    runTest([&]() {
                        // TODO: suspend function
                        int n = iterations;
                        Semaphore semaphore(1, 1);
                        auto pool = new_single_thread_context("SemaphoreStressTest");
                        pool.use([&]() {
                            for (int i = 0; i < n; ++i) {
                                // Initially, we hold the permit and no one else can `acquire`,
                                // otherwise it's a bug.
                                assertEquals(0, semaphore.available_permits());
                                bool job1_entered_critical_section = false;
                                auto job1 = launch(CoroutineStart::kUndispatched, [&]() {
                                    // TODO: suspend function
                                    semaphore.acquire();
                                    job1_entered_critical_section = true;
                                    semaphore.release();
                                });
                                // check that `job1` didn't finish the call to `acquire()`
                                assertEquals(false, job1_entered_critical_section);
                                auto job2 = launch(pool, [&]() {
                                    // TODO: suspend function
                                    semaphore.release();
                                });
                                // Because `job2` executes in a separate thread, this
                                // cancellation races with the call to `release()`.
                                job1.cancel_and_join();
                                job2.join();
                                assertEquals(1, semaphore.available_permits());
                                semaphore.acquire();
                            }
                        });
                    });
                }

                // @Test
                // TODO: Convert test annotation
                void test_should_be_unlocked_on_cancellation() {
                    runTest([&]() {
                        // TODO: suspend function
                        Semaphore semaphore(1);
                        int n = 1000 * stress_test_multiplier;
                        for (int i = 0; i < n; ++i) {
                            auto job = launch(Dispatchers::Default, [&]() {
                                // TODO: suspend function
                                semaphore.acquire();
                                semaphore.release();
                            });
                            semaphore.with_permit([&]() {
                                job.cancel();
                            });
                            job.join();
                            assertTrue(semaphore.available_permits() == 1);
                        }
                    });
                }
            };
        } // namespace sync
    } // namespace coroutines
} // namespace kotlinx