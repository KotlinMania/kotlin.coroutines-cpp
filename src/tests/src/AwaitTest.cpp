// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/AwaitTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.coroutines.testing.*
        // TODO: import kotlin.test.*

        class AwaitTest : public TestBase {
        public:
            // @Test
            void test_await_all() {
                run_test([this]() {
                    expect(1);
                    auto d = async([this]() {
                        expect(3);
                        return "OK";
                    });

                    auto d2 = async([this]() {
                        yield();
                        expect(4);
                        return 1L;
                    });

                    expect(2);
                    require(d2.is_active() && !d2.is_completed());

                    assert_equals(std::vector<std::any>({"OK", 1L}), await_all(d, d2));
                    expect(5);

                    require(d.is_completed() && d2.is_completed());
                    require(!d.is_cancelled() && !d2.is_cancelled());
                    finish(6);
                });
            }

            // @Test
            void test_await_all_lazy() {
                run_test([this]() {
                    expect(1);
                    auto d = async(CoroutineStart::LAZY, [this]() {
                        expect(2);
                        return 1;
                    });
                    auto d2 = async(CoroutineStart::LAZY, [this]() {
                        expect(3);
                        return 2;
                    });
                    assert_equals(std::vector<int>({1, 2}), await_all(d, d2));
                    finish(4);
                });
            }

            // @Test
            void test_await_all_typed() {
                run_test([this]() {
                    auto d1 = async([]() { return 1L; });
                    auto d2 = async([]() { return ""; });
                    auto d3 = async([]() {
                        /* void */
                    });

                    assert_equals(std::vector<std::any>({1L, ""}), await_all({d1, d2}));
                    assert_equals(std::vector<std::any>({1L, std::monostate{}}), await_all({d1, d3}));
                    assert_equals(std::vector<std::any>({"", std::monostate{}}), await_all({d2, d3}));
                });
            }

            // @Test
            void test_await_all_exceptionally() {
                run_test([this]() {
                    expect(1);
                    auto d = async([this]() {
                        expect(3);
                        return "OK";
                    });

                    auto d2 = async(NonCancellable, [this]() {
                        yield();
                        throw TestException();
                    });

                    auto d3 = async([this]() {
                        expect(4);
                        delay(LONG_MAX);
                        return 1;
                    });

                    expect(2);
                    try {
                        await_all(d, d2, d3);
                    } catch (const TestException &e) {
                        expect(5);
                    }

                    yield();
                    require(d.is_completed() && d2.is_cancelled() && d3.is_active());
                    d3.cancel();
                    finish(6);
                });
            }

            // @Test
            void test_await_all_multiple_exceptions() {
                run_test([this]() {
                    auto d = async(NonCancellable, [this]() {
                        expect(2);
                        throw TestException();
                    });

                    auto d2 = async(NonCancellable, [this]() {
                        yield();
                        throw TestException();
                    });

                    auto d3 = async([this]() {
                        yield();
                    });

                    expect(1);
                    try {
                        await_all(d, d2, d3);
                    } catch (const TestException &e) {
                        expect(3);
                    }

                    finish(4);
                });
            }

            // @Test
            void test_await_all_cancellation() {
                run_test([this]() {
                    auto outer = async([this]() {
                        expect(1);
                        auto inner = async([this]() {
                            expect(4);
                            delay(LONG_MAX);
                        });

                        expect(2);
                        await_all(inner);
                        expect_unreached();
                    });

                    yield();
                    expect(3);
                    yield();
                    require(outer.is_active());
                    outer.cancel();
                    require(outer.is_cancelled());
                    finish(5);
                });
            }

            // @Test
            void test_await_all_partially_completed() {
                run_test([this]() {
                    auto d1 = async([this]() {
                        expect(1);
                        return 1;
                    });
                    d1.await();
                    auto d2 = async([this]() {
                        expect(3);
                        return 2;
                    });
                    expect(2);
                    assert_equals(std::vector<int>({1, 2}), await_all(d1, d2));
                    require(d1.is_completed() && d2.is_completed());
                    finish(4);
                });
            }

            // @Test
            void test_await_all_partially_completed_exceptionally() {
                run_test([this]() {
                    auto d1 = async(NonCancellable, [this]() {
                        expect(1);
                        throw TestException();
                    });

                    yield();

                    // This job is called after exception propagation
                    auto d2 = async([this]() { expect(4); });

                    expect(2);
                    try {
                        await_all(d1, d2);
                        expect_unreached();
                    } catch (const TestException &e) {
                        expect(3);
                    }

                    require(d2.is_active());
                    d2.await();
                    require(d1.is_completed() && d2.is_completed());
                    finish(5);
                });
            }

            // @Test
            void test_await_all_fully_completed() {
                run_test([this]() {
                    auto d1 = CompletableDeferred<void>();
                    auto d2 = CompletableDeferred<void>();
                    auto job = async([this]() { expect(3); });
                    expect(1);
                    await_all(d1, d2);
                    expect(2);
                    job.await();
                    finish(4);
                });
            }

            // @Test
            void test_await_on_set() {
                run_test([this]() {
                    auto d1 = CompletableDeferred<void>();
                    auto d2 = CompletableDeferred<void>();
                    auto job = async([this]() { expect(2); });
                    expect(1);
                    std::vector<Deferred<void> > list = {d1, d2, job};
                    await_all(list);
                    finish(3);
                });
            }

            // @Test
            void test_await_all_fully_completed_exceptionally() {
                run_test([this]() {
                    auto d1 = CompletableDeferred<void>(/* parent = */ nullptr);
                    d1.complete_exceptionally(TestException());
                    auto d2 = CompletableDeferred<void>(/* parent = */ nullptr);
                    d2.complete_exceptionally(TestException());
                    auto job = async([this]() { expect(3); });
                    expect(1);
                    try {
                        await_all(d1, d2);
                    } catch (const TestException &e) {
                        expect(2);
                    }

                    job.await();
                    finish(4);
                });
            }

            // @Test
            void test_await_all_same_job_multiple_times() {
                run_test([this]() {
                    auto d = async([]() { return "OK"; });
                    // Duplicates are allowed though kdoc doesn't guarantee that
                    assert_equals(std::vector<std::string>({"OK", "OK", "OK"}), await_all(d, d, d));
                });
            }

            // @Test
            void test_await_all_same_throwing_job_multiple_times() {
                run_test([this]() {
                    auto d1 = async(NonCancellable, [this]() { throw TestException(); });
                    auto d2 = async([this]() {
                        /* do nothing */
                    });

                    try {
                        expect(1);
                        // Duplicates are allowed though kdoc doesn't guarantee that
                        await_all(d1, d2, d1, d2);
                        expect_unreached();
                    } catch (const TestException &e) {
                        finish(2);
                    }
                });
            }

            // @Test
            void test_await_all_empty() {
                run_test([this]() {
                    expect(1);
                    assert_equals(std::vector<void>(), await_all<void>());
                    std::vector<Deferred<void> > empty_list;
                    assert_equals(std::vector<void>(), await_all(empty_list));
                    finish(2);
                });
            }

            // joinAll

            // @Test
            void test_join_all() {
                run_test([this]() {
                    auto d1 = launch([this]() { expect(2); });
                    auto d2 = async([this]() {
                        expect(3);
                        return "OK";
                    });
                    auto d3 = launch([this]() { expect(4); });

                    expect(1);
                    join_all(d1, d2, d3);
                    finish(5);
                });
            }

            // @Test
            void test_join_all_lazy() {
                run_test([this]() {
                    expect(1);
                    auto d = async(CoroutineStart::LAZY, [this]() {
                        expect(2);
                    });
                    auto d2 = launch(CoroutineStart::LAZY, [this]() {
                        expect(3);
                    });
                    join_all(d, d2);
                    finish(4);
                });
            }

            // @Test
            void test_join_all_exceptionally() {
                run_test([this]() {
                    auto d1 = launch([this]() {
                        expect(2);
                    });
                    auto d2 = async(NonCancellable, [this]() {
                        expect(3);
                        throw TestException();
                    });
                    auto d3 = async([this]() {
                        expect(4);
                    });

                    expect(1);
                    join_all(d1, d2, d3);
                    finish(5);
                });
            }

            // @Test
            void test_join_all_cancellation() {
                run_test([this]() {
                    auto outer = launch([this]() {
                        expect(2);
                        auto inner = launch([this]() {
                            expect(3);
                            delay(LONG_MAX);
                        });

                        join_all(inner);
                        expect_unreached();
                    });

                    expect(1);
                    yield();
                    require(outer.is_active());
                    yield();
                    outer.cancel();
                    outer.join();
                    finish(4);
                });
            }

            // @Test
            void test_join_all_already_completed() {
                run_test([this]() {
                    auto job = launch([this]() {
                        expect(1);
                    });

                    job.join();
                    expect(2);

                    join_all(job);
                    finish(3);
                });
            }

            // @Test
            void test_join_all_empty() {
                run_test([this]() {
                    expect(1);
                    join_all();
                    std::vector<Job *> empty_list;
                    join_all(empty_list);
                    finish(2);
                });
            }

            // @Test
            void test_join_all_same_job() {
                run_test([this]() {
                    auto job = launch([]() {
                    });
                    join_all(job, job, job);
                });
            }

            // @Test
            void test_join_all_same_job_exceptionally() {
                run_test([this]() {
                    auto job = async(NonCancellable, [this]() { throw TestException(); });
                    join_all(job, job, job);
                });
            }

            // @Test
            void test_await_all_delegates() {
                run_test([this]() {
                    expect(1);
                    auto deferred = CompletableDeferred<std::string>();
                    // @OptIn(InternalForInheritanceCoroutinesApi::class)
                    // TODO: object : Deferred<String> by deferred {}
                    Deferred<std::string> *delegate = &deferred;
                    launch([this, &deferred]() {
                        expect(3);
                        deferred.complete("OK");
                    });
                    expect(2);
                    await_all(delegate);
                    finish(4);
                });
            }

            // @Test
            void test_cancel_await_all_delegate() {
                run_test([this]() {
                    expect(1);
                    auto deferred = CompletableDeferred<std::string>();
                    // @OptIn(InternalForInheritanceCoroutinesApi::class)
                    // TODO: object : Deferred<String> by deferred {}
                    Deferred<std::string> *delegate = &deferred;
                    launch([this, &deferred]() {
                        expect(3);
                        deferred.cancel();
                    });
                    expect(2);
                    assert_fails_with<CancellationException>([&]() { await_all(delegate); });
                    finish(4);
                });
            }
        };
    } // namespace coroutines
} // namespace kotlinx