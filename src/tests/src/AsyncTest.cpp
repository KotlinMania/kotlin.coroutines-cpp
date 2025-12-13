/**
 * @file AsyncTest.cpp
 * @brief Tests for async coroutine builder
 *
 * Transliterated from: kotlinx-coroutines-core/common/test/AsyncTest.kt
 */

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include "kotlinx/coroutines/Deferred.hpp"
#include "kotlinx/coroutines/NonCancellable.hpp"
#include "kotlinx/coroutines/Yield.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include <limits>

namespace kotlinx::coroutines {

using namespace testing;

class AsyncTest : public testing::TestBase {
public:
    // @Test
    void test_simple() {
        run_test([this](CoroutineScope* scope) {
            expect(1);
            auto d = async<int>(scope, [this](CoroutineScope*) {
                expect(3);
                return 42;
            });
            expect(2);
            assert_true(d->is_active());
            assert_equals(d->await(), 42);
            assert_true(!d->is_active());
            expect(4);
            assert_equals(d->await(), 42); // second await -- same result
            finish(5);
        });
    }

    // @Test
    void test_undispatched() {
        run_test([this](CoroutineScope* scope) {
            expect(1);
            auto d = async<int>(scope, nullptr, CoroutineStart::UNDISPATCHED, [this](CoroutineScope*) {
                expect(2);
                return 42;
            });
            expect(3);
            assert_true(!d->is_active());
            assert_equals(d->await(), 42);
            finish(4);
        });
    }

    // @Test
    void test_simple_exception() {
        run_test(
            [](std::exception_ptr e) {
                try { std::rethrow_exception(e); }
                catch (const testing::TestException&) { return true; }
                catch (...) { return false; }
            },
            [this](CoroutineScope* scope) {
                expect(1);
                auto d = async<Unit>(scope, [this](CoroutineScope*) -> Unit {
                    finish(3);
                    throw testing::TestException();
                });
                expect(2);
                d->await(); // will throw TestException
            }
        );
    }

    // @Test
    void test_cancellation_with_cause() {
        run_test([this](CoroutineScope* scope) {
            expect(1);
            auto d = async<Unit>(scope, non_cancellable(), CoroutineStart::ATOMIC, [this](CoroutineScope*) -> Unit {
                expect(3);
                yield();
                return Unit{};
            });
            expect(2);
            d->cancel(std::make_exception_ptr(testing::TestCancellationException("TEST")));
            try {
                d->await();
            } catch (const testing::TestCancellationException& e) {
                finish(4);
                assert_equals(std::string("TEST"), std::string(e.what()));
            }
        });
    }

    // @Test
    void test_lost_exception() {
        run_test([this](CoroutineScope* scope) {
            expect(1);
            auto deferred = async<Unit>(scope, make_job(), [this](CoroutineScope*) -> Unit {
                expect(2);
                throw std::runtime_error("Exception");
            });

            // Exception is not consumed -> nothing is reported
            deferred->join_blocking();
            finish(3);
        });
    }

    // @Test
    void test_parallel_decomposition_caught_exception() {
        run_test([this](CoroutineScope* scope) {
            auto deferred = async<int>(scope, non_cancellable(), [this](CoroutineScope* inner_scope) -> int {
                auto decomposed = async<int>(inner_scope, non_cancellable(), [](CoroutineScope*) -> int {
                    throw testing::TestException();
                    return 1;
                });
                try {
                    return decomposed->await();
                } catch (const testing::TestException&) {
                    return 42;
                }
            });
            assert_equals(42, deferred->await());
        });
    }

    // @Test
    void test_parallel_decomposition_caught_exception_with_inherited_parent() {
        run_test([this](CoroutineScope* scope) {
            expect(1);
            auto deferred = async<int>(scope, non_cancellable(), [this](CoroutineScope* inner_scope) -> int {
                expect(2);
                auto decomposed = async<int>(inner_scope, [this](CoroutineScope*) -> int { // inherits parent job!
                    expect(3);
                    throw testing::TestException();
                    return 1;
                });
                try {
                    return decomposed->await();
                } catch (const testing::TestException&) {
                    expect(4); // Should catch this exception, but parent is already cancelled
                    return 42;
                }
            });
            try {
                // This will fail
                assert_equals(42, deferred->await());
            } catch (const testing::TestException&) {
                finish(5);
            }
        });
    }

    // @Test
    void test_parallel_decomposition_uncaught_exception_with_inherited_parent() {
        run_test(
            [](std::exception_ptr e) {
                try { std::rethrow_exception(e); }
                catch (const testing::TestException&) { return true; }
                catch (...) { return false; }
            },
            [this](CoroutineScope* scope) {
                auto deferred = async<int>(scope, non_cancellable(), [](CoroutineScope* inner_scope) -> int {
                    auto decomposed = async<int>(inner_scope, [](CoroutineScope*) -> int {
                        throw testing::TestException();
                        return 1;
                    });

                    return decomposed->await();
                });

                deferred->await();
                expect_unreached();
            }
        );
    }

    // @Test
    void test_parallel_decomposition_uncaught_exception() {
        run_test(
            [](std::exception_ptr e) {
                try { std::rethrow_exception(e); }
                catch (const testing::TestException&) { return true; }
                catch (...) { return false; }
            },
            [this](CoroutineScope* scope) {
                auto deferred = async<int>(scope, non_cancellable(), [](CoroutineScope* inner_scope) -> int {
                    auto decomposed = async<int>(inner_scope, [](CoroutineScope*) -> int {
                        throw testing::TestException();
                        return 1;
                    });

                    return decomposed->await();
                });

                deferred->await();
                expect_unreached();
            }
        );
    }

    // @Test
    void test_cancellation_transparency() {
        run_test([this](CoroutineScope* scope) {
            auto deferred = async<Unit>(scope, non_cancellable(), CoroutineStart::ATOMIC, [this](CoroutineScope*) -> Unit {
                expect(2);
                throw testing::TestException();
            });
            expect(1);
            deferred->cancel();
            try {
                deferred->await();
            } catch (const testing::TestException&) {
                finish(3);
            }
        });
    }

    // @Test
    void test_defer_and_yield_exception() {
        run_test(
            [](std::exception_ptr e) {
                try { std::rethrow_exception(e); }
                catch (const testing::TestException&) { return true; }
                catch (...) { return false; }
            },
            [this](CoroutineScope* scope) {
                expect(1);
                auto d = async<Unit>(scope, [this](CoroutineScope*) -> Unit {
                    expect(3);
                    yield(); // no effect, parent waiting
                    finish(4);
                    throw testing::TestException();
                });
                expect(2);
                d->await(); // will throw TestException
            }
        );
    }

    // @Test
    void test_defer_with_two_waiters() {
        run_test([this](CoroutineScope* scope) {
            expect(1);
            auto d = async<int>(scope, [this](CoroutineScope*) -> int {
                expect(5);
                yield();
                expect(9);
                return 42;
            });
            expect(2);
            launch(scope, [this, &d](CoroutineScope*) {
                expect(6);
                assert_equals(d->await(), 42);
                expect(11);
            });
            expect(3);
            launch(scope, [this, &d](CoroutineScope*) {
                expect(7);
                assert_equals(d->await(), 42);
                expect(12);
            });
            expect(4);
            yield(); // this actually yields control to async, which produces results and resumes both waiters (in order)
            expect(8);
            yield(); // yield again to "d", which completes
            expect(10);
            yield(); // yield to both waiters
            finish(13);
        });
    }

    // @Test
    void test_defer_bad_class() {
        run_test([this](CoroutineScope* scope) {
            testing::BadClass bad;
            auto d = async<testing::BadClass>(scope, [this, &bad](CoroutineScope*) -> testing::BadClass {
                expect(1);
                return bad;
            });
            // Note: Can't use assertSame with BadClass since equals throws
            d->await();
            finish(2);
        });
    }

    // @Test
    void test_overridden_parent() {
        run_test([this](CoroutineScope* scope) {
            auto parent = make_job();
            auto deferred = async<Unit>(scope, std::static_pointer_cast<CoroutineContext>(parent), CoroutineStart::ATOMIC, [this](CoroutineScope*) -> Unit {
                expect(2);
                delay(std::numeric_limits<long long>::max());
                return Unit{};
            });

            parent->cancel();
            try {
                expect(1);
                deferred->await();
            } catch (const CancellationException&) {
                finish(3);
            }
        });
    }

    // @Test
    void test_incomplete_async_state() {
        run_test([this](CoroutineScope* scope) {
            auto deferred = async<std::shared_ptr<DisposableHandle>>(scope, [](CoroutineScope* s) {
                auto job = s->get_coroutine_context()->get(Job::type_key);
                auto job_ptr = std::dynamic_pointer_cast<Job>(job);
                return job_ptr->invoke_on_completion([](std::exception_ptr) {});
            });

            deferred->await()->dispose();
            // assertIs<DisposableHandle>(deferred->get_completed()); // Type is already DisposableHandle
            assert_null(deferred->get_completion_exception_or_null());
            assert_true(deferred->is_completed());
            assert_false(deferred->is_active());
            assert_false(deferred->is_cancelled());
        });
    }

    // @Test
    void test_incomplete_async_fast_path() {
        run_test([this](CoroutineScope* scope) {
            // TODO: Pass Dispatchers::get_unconfined() when dispatcher as context is supported
            // For now, use nullptr context (default dispatcher)
            auto deferred = async<std::shared_ptr<DisposableHandle>>(scope, [](CoroutineScope* s) {
                auto job = s->get_coroutine_context()->get(Job::type_key);
                auto job_ptr = std::dynamic_pointer_cast<Job>(job);
                return job_ptr->invoke_on_completion([](std::exception_ptr) {});
            });

            deferred->await()->dispose();
            // assertIs<DisposableHandle>(deferred->get_completed());
            assert_null(deferred->get_completion_exception_or_null());
            assert_true(deferred->is_completed());
            assert_false(deferred->is_active());
            assert_false(deferred->is_cancelled());
        });
    }

    // @Test
    void test_async_with_finally() {
        run_test([this](CoroutineScope* scope) {
            expect(1);

            auto d = async<std::string>(scope, [this](CoroutineScope*) -> std::string {
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
            check(d->is_active() && !d->is_completed() && !d->is_cancelled());
            d->cancel();
            check(!d->is_active() && !d->is_completed() && d->is_cancelled());
            check(!d->is_active() && !d->is_completed() && d->is_cancelled());
            expect(5);
            try {
                d->await(); // awaits
                expect_unreached(); // does not complete normally
            } catch (const std::exception& e) {
                expect(7);
                check(dynamic_cast<const CancellationException*>(&e) != nullptr);
            }
            check(!d->is_active() && d->is_completed() && d->is_cancelled());
            finish(8);
        });
    }

private:
    void check(bool condition) {
        if (!condition) {
            report_error(std::make_exception_ptr(std::logic_error("Check failed")));
            throw std::logic_error("Check failed");
        }
    }
};

} // namespace kotlinx::coroutines

// =============================================================================
// Main - run all tests
// =============================================================================

int main() {
    using namespace kotlinx::coroutines;

    AsyncTest test;
    int failed = 0;

    auto run = [&](const char* name, void (AsyncTest::*method)()) {
        std::cout << "Running " << name << "..." << std::endl;
        try {
            (test.*method)();
            test.reset();
            std::cout << "  PASSED" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "  FAILED: " << e.what() << std::endl;
            failed++;
            test.reset();
        }
    };

    std::cout << "=== AsyncTest ===" << std::endl;

    run("test_simple", &AsyncTest::test_simple);
    run("test_undispatched", &AsyncTest::test_undispatched);
    run("test_simple_exception", &AsyncTest::test_simple_exception);
    run("test_cancellation_with_cause", &AsyncTest::test_cancellation_with_cause);
    run("test_lost_exception", &AsyncTest::test_lost_exception);
    run("test_parallel_decomposition_caught_exception", &AsyncTest::test_parallel_decomposition_caught_exception);
    run("test_parallel_decomposition_caught_exception_with_inherited_parent", &AsyncTest::test_parallel_decomposition_caught_exception_with_inherited_parent);
    run("test_parallel_decomposition_uncaught_exception_with_inherited_parent", &AsyncTest::test_parallel_decomposition_uncaught_exception_with_inherited_parent);
    run("test_parallel_decomposition_uncaught_exception", &AsyncTest::test_parallel_decomposition_uncaught_exception);
    run("test_cancellation_transparency", &AsyncTest::test_cancellation_transparency);
    run("test_defer_and_yield_exception", &AsyncTest::test_defer_and_yield_exception);
    run("test_defer_with_two_waiters", &AsyncTest::test_defer_with_two_waiters);
    run("test_defer_bad_class", &AsyncTest::test_defer_bad_class);
    run("test_overridden_parent", &AsyncTest::test_overridden_parent);
    run("test_incomplete_async_state", &AsyncTest::test_incomplete_async_state);
    run("test_incomplete_async_fast_path", &AsyncTest::test_incomplete_async_fast_path);
    run("test_async_with_finally", &AsyncTest::test_async_with_finally);

    std::cout << "=== Results: " << (17 - failed) << "/17 passed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
