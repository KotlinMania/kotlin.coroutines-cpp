// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/AsyncTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

// @file:Suppress("NAMED_ARGUMENTS_NOT_ALLOWED", "UNREACHABLE_CODE", "USELESS_IS_CHECK") // KT-21913

#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/test/TestBuilders.hpp"


    namespace kotlinx::coroutines {
        // TODO: import kotlinx.coroutines.testing.*
        // TODO: import kotlin.test.*

        class AsyncTest : public TestBase {
        public:
            // @Test
            // TODO: Translate @Test annotation
            void test_simple() {
                test::run_test([this]() {
                    expect(1);
                    auto d = async([this]() {
                        expect(3);
                        return 42;
                    });
                    expect(2);
                    assert_true(d.is_active());
                    assert_equals(d.await(), 42);
                    assert_true(!d.is_active());
                    expect(4);
                    assert_equals(d.await(), 42); // second await -- same result
                    finish(5);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_undispatched() {
                run_test([this]() {
                    expect(1);
                    auto d = async(CoroutineStart::UNDISPATCHED, [this]() {
                        expect(2);
                        return 42;
                    });
                    expect(3);
                    assert_true(!d.is_active());
                    assert_equals(d.await(), 42);
                    finish(4);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_simple_exception() {
                run_test([](auto it) { return dynamic_cast<TestException *>(it) != nullptr; },
                         [this]() {
                             expect(1);
                             auto d = async<void>([this]() {
                                 finish(3);
                                 throw TestException();
                             });
                             expect(2);
                             d.await(); // will throw TestException
                         });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_cancellation_with_cause() {
                run_test([this]() {
                    expect(1);
                    auto d = async(NonCancellable, CoroutineStart::ATOMIC, [this]() {
                        expect(3);
                        yield();
                    });
                    expect(2);
                    d.cancel(TestCancellationException("TEST"));
                    try {
                        d.await();
                    } catch (const TestCancellationException &e) {
                        finish(4);
                        assert_equals("TEST", e.message);
                    }
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_lost_exception() {
                test::run_test([this]() {
                    expect(1);
                    auto deferred = async(Job(), [this]() {
                        expect(2);
                        throw Exception();
                    });

                    // Exception is not consumed -> nothing is reported
                    deferred.join();
                    finish(3);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_parallel_decomposition_caught_exception() {
                test::run_test([this]() {
                    auto deferred = async(NonCancellable, [this]() {
                        auto decomposed = async(NonCancellable, [this]() {
                            throw TestException();
                            return 1;
                        });
                        try {
                            decomposed.await();
                        } catch (const TestException &e) {
                            return 42;
                        }
                    });
                    assert_equals(42, deferred.await());
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_parallel_decomposition_caught_exception_with_inherited_parent() {
                test::run_test([this]() {
                    expect(1);
                    auto deferred = async(NonCancellable, [this]() {
                        expect(2);
                        auto decomposed = async([this]() {
                            // inherits parent job!
                            expect(3);
                            throw TestException();
                            return 1;
                        });
                        try {
                            decomposed.await();
                        } catch (const TestException &e) {
                            expect(4); // Should catch this exception, but parent is already cancelled
                            return 42;
                        }
                    });
                    try {
                        // This will fail
                        assert_equals(42, deferred.await());
                    } catch (const TestException &e) {
                        finish(5);
                    }
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_parallel_decomposition_uncaught_exception_with_inherited_parent() {
                run_test([](auto it) { return dynamic_cast<TestException *>(it) != nullptr; },
                         [this]() {
                             auto deferred = async(NonCancellable, [this]() {
                                 auto decomposed = async([this]() {
                                     throw TestException();
                                     return 1;
                                 });

                                 decomposed.await();
                             });

                             deferred.await();
                             expect_unreached();
                         });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_parallel_decomposition_uncaught_exception() {
                run_test([](auto it) { return dynamic_cast<TestException *>(it) != nullptr; },
                         [this]() {
                             auto deferred = async(NonCancellable, [this]() {
                                 auto decomposed = async([this]() {
                                     throw TestException();
                                     return 1;
                                 });

                                 decomposed.await();
                             });

                             deferred.await();
                             expect_unreached();
                         });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_cancellation_transparency() {
                run_test([this]() {
                    auto deferred = async(NonCancellable, CoroutineStart::ATOMIC, [this]() {
                        expect(2);
                        throw TestException();
                    });
                    expect(1);
                    deferred.cancel();
                    try {
                        deferred.await();
                    } catch (const TestException &e) {
                        finish(3);
                    }
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_defer_and_yield_exception() {
                run_test([](auto it) { return dynamic_cast<TestException *>(it) != nullptr; },
                         [this]() {
                             expect(1);
                             auto d = async<void>([this]() {
                                 expect(3);
                                 yield(); // no effect, parent waiting
                                 finish(4);
                                 throw TestException();
                             });
                             expect(2);
                             d.await(); // will throw IOException
                         });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_defer_with_two_waiters() {
                run_test([this]() {
                    expect(1);
                    auto d = async([this]() {
                        expect(5);
                        yield();
                        expect(9);
                        return 42;
                    });
                    expect(2);
                    launch([this, &d]() {
                        expect(6);
                        assert_equals(d.await(), 42);
                        expect(11);
                    });
                    expect(3);
                    launch([this, &d]() {
                        expect(7);
                        assert_equals(d.await(), 42);
                        expect(12);
                    });
                    expect(4);
                    yield();
                    // this actually yields control to async, which produces results and resumes both waiters (in order)
                    expect(8);
                    yield(); // yield again to "d", which completes
                    expect(10);
                    yield(); // yield to both waiters
                    finish(13);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_defer_bad_class() {
                run_test([this]() {
                    auto bad = BadClass();
                    auto d = async([this, &bad]() {
                        expect(1);
                        return bad;
                    });
                    assert_same(d.await(), bad);
                    finish(2);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_overridden_parent() {
                run_test([this]() {
                    auto parent = Job();
                    auto deferred = async(parent, CoroutineStart::ATOMIC, [this]() {
                        expect(2);
                        delay(LONG_MAX);
                    });

                    parent.cancel();
                    try {
                        expect(1);
                        deferred.await();
                    } catch (const CancellationException &e) {
                        finish(3);
                    }
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_incomplete_async_state() {
                run_test([this]() {
                    auto deferred = async([this]() {
                        return coroutine_context[Job()].invoke_on_completion([]() {
                        });
                    });

                    deferred.await().dispose();
                    assert_is<DisposableHandle>(deferred.get_completed());
                    assert_null(deferred.get_completion_exception_or_null());
                    assert_true(deferred.is_completed());
                    assert_false(deferred.is_active());
                    assert_false(deferred.is_cancelled());
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_incomplete_async_fast_path() {
                run_test([this]() {
                    auto deferred = async(Dispatchers::Unconfined, [this]() {
                        return coroutine_context[Job()].invoke_on_completion([]() {
                        });
                    });

                    deferred.await().dispose();
                    assert_is<DisposableHandle>(deferred.get_completed());
                    assert_null(deferred.get_completion_exception_or_null());
                    assert_true(deferred.is_completed());
                    assert_false(deferred.is_active());
                    assert_false(deferred.is_cancelled());
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_async_with_finally() {
                run_test([this]() {
                    expect(1);

                    // @Suppress("UNREACHABLE_CODE")
                    auto d = async([this]() {
                        expect(3);
                        try {
                            yield(); // to main, will cancel
                        } catch (...) {
                            expect(6); // will go there on await
                            return "Fail"; // result will not override cancellation
                        }
                        expect_unreached();
                        return "Fail2";
                    });
                    expect(2);
                    yield(); // to async
                    expect(4);
                    check(d.is_active() && !d.is_completed() && !d.is_cancelled());
                    d.cancel();
                    check(!d.is_active() && !d.is_completed() && d.is_cancelled());
                    check(!d.is_active() && !d.is_completed() && d.is_cancelled());
                    expect(5);
                    try {
                        d.await(); // awaits
                        expect_unreached(); // does not complete normally
                    } catch (const std::exception &e) {
                        expect(7);
                        check(dynamic_cast<const CancellationException *>(&e) != nullptr);
                    }
                    check(!d.is_active() && d.is_completed() && d.is_cancelled());
                    finish(8);
                });
            }
        };
    } // namespace kotlinx::coroutines
