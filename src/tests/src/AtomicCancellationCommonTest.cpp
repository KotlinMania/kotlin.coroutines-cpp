// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/AtomicCancellationCommonTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/test/TestBuilders.hpp"

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.coroutines.testing.*
        // TODO: import kotlinx.coroutines.selects.*
        // TODO: import kotlinx.coroutines.sync.*
        // TODO: import kotlin.test.*

        class AtomicCancellationCommonTest : public TestBase {
        public:
            // @Test
            // TODO: Translate @Test annotation
            void test_cancellable_launch() {
                test::run_test([this]() {
                    expect(1);
                    auto job = launch([this]() {
                        expect_unreached(); // will get cancelled before start
                    });
                    expect(2);
                    job.cancel();
                    finish(3);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_atomic_launch() {
                test::run_test([this]() {
                    expect(1);
                    auto job = launch(CoroutineStart::ATOMIC, [this]() {
                        finish(4); // will execute even after it was cancelled
                    });
                    expect(2);
                    job.cancel();
                    expect(3);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_undispatched_launch() {
                test::run_test([this]() {
                    expect(1);
                    assert_fails_with<CancellationException>([this]() {
                        with_context(Job(), [this]() {
                            cancel();
                            launch(CoroutineStart::UNDISPATCHED, [this]() {
                                expect(2);
                                yield();
                                expect_unreached();
                            });
                        });
                    });
                    finish(3);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_undispatched_launch_with_unconfined_context() {
                test::run_test([this]() {
                    expect(1);
                    assert_fails_with<CancellationException>([this]() {
                        with_context(Dispatchers::Unconfined + Job(), [this]() {
                            cancel();
                            launch(CoroutineStart::UNDISPATCHED, [this]() {
                                expect(2);
                                std::this_thread::yield();
                                expect_unreached();
                            });
                        });
                    });
                    finish(3);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_deferred_await_cancellable() {
                test::run_test([this]() {
                    expect(1);
                    auto deferred = async([this]() {
                        // deferred, not yet complete
                        expect(4);
                        return "OK";
                    });
                    assert_equals(false, deferred.is_completed());
                    Job *job = nullptr;
                    launch([this, &deferred, &job]() {
                        // will cancel job as soon as deferred completes
                        expect(5);
                        assert_equals(true, deferred.is_completed());
                        job->cancel();
                    });
                    job = &launch(CoroutineStart::UNDISPATCHED, [this, &deferred]() {
                        expect(2);
                        try {
                            deferred.await(); // suspends
                            expect_unreached(); // will not execute -- cancelled while dispatched
                        } catch (...) {
                            finish(7); // but will execute finally blocks
                        }
                    });
                    expect(3); // continues to execute when the job suspends
                    yield(); // to deferred & canceller
                    expect(6);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_job_join_cancellable() {
                run_test([this]() {
                    expect(1);
                    auto job_to_join = launch([this]() {
                        // not yet complete
                        expect(4);
                    });
                    assert_equals(false, job_to_join.is_completed());
                    Job *job = nullptr;
                    launch([this, &job_to_join, &job]() {
                        // will cancel job as soon as jobToJoin completes
                        expect(5);
                        assert_equals(true, job_to_join.is_completed());
                        job->cancel();
                    });
                    job = &launch(CoroutineStart::UNDISPATCHED, [this, &job_to_join]() {
                        expect(2);
                        try {
                            job_to_join.join(); // suspends
                            expect_unreached(); // will not execute -- cancelled while dispatched
                        } catch (...) {
                            finish(7); // but will execute finally blocks
                        }
                    });
                    expect(3); // continues to execute when the job suspends
                    yield(); // to jobToJoin & canceller
                    expect(6);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_lock_cancellable() {
                run_test([this]() {
                    expect(1);
                    auto mutex = Mutex(true); // locked mutex
                    auto job = launch(CoroutineStart::UNDISPATCHED, [this, &mutex]() {
                        expect(2);
                        mutex.lock(); // suspends
                        expect_unreached(); // should NOT execute because of cancellation
                    });
                    expect(3);
                    mutex.unlock(); // unlock mutex first
                    job.cancel(); // cancel the job next
                    yield(); // now yield
                    finish(4);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_select_lock_cancellable() {
                run_test([this]() {
                    expect(1);
                    auto mutex = Mutex(true); // locked mutex
                    auto job = launch(CoroutineStart::UNDISPATCHED, [this, &mutex]() {
                        expect(2);
                        select<std::string>([this, &mutex]() {
                            // suspends
                            mutex.on_lock([this]() {
                                expect(4);
                                return "OK";
                            });
                        });
                        expect_unreached(); // should NOT execute because of cancellation
                    });
                    expect(3);
                    mutex.unlock(); // unlock mutex first
                    job.cancel(); // cancel the job next
                    yield(); // now yield
                    finish(4);
                });
            }
        };
    } // namespace coroutines
} // namespace kotlinx