// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/AsyncLazyTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

// @file:Suppress("NAMED_ARGUMENTS_NOT_ALLOWED") // KT-21913

#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/test/TestBuilders.hpp"

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.coroutines.testing.*
        // TODO: import kotlin.test.*

        class AsyncLazyTest : public TestBase {
        public:
            // @Test
            // TODO: Translate @Test annotation
            void test_simple() {
                test::run_test([this]() {
                    expect(1);
                    auto d = async(CoroutineStart::kLazy, [this]() {
                        expect(3);
                        return 42;
                    });
                    expect(2);
                    assert_true(!d.is_active() && !d.is_completed());
                    assert_equals(d.await(), 42);
                    assert_true(!d.is_active() && d.is_completed() && !d.is_cancelled());
                    expect(4);
                    assert_equals(d.await(), 42); // second await -- same result
                    finish(5);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_lazy_defer_and_yield() {
                test::run_test([this]() {
                    expect(1);
                    auto d = async(CoroutineStart::kLazy, [this]() {
                        expect(3);
                        std::this_thread::yield(); // this has not effect, because parent coroutine is waiting
                        expect(4);
                        return 42;
                    });
                    expect(2);
                    assert_true(!d.is_active() && !d.is_completed());
                    assert_equals(d.await(), 42);
                    assert_true(!d.is_active() && d.is_completed() && !d.is_cancelled());
                    expect(5);
                    assert_equals(d.await(), 42); // second await -- same result
                    finish(6);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_lazy_defer_and_yield2() {
                test::run_test([this]() {
                    expect(1);
                    auto d = async(CoroutineStart::kLazy, [this]() {
                        expect(7);
                        return 42;
                    });
                    expect(2);
                    assert_true(!d.is_active() && !d.is_completed());
                    launch([this, &d]() {
                        // see how it looks from another coroutine
                        expect(4);
                        assert_true(!d.is_active() && !d.is_completed());
                        yield(); // yield back to main
                        expect(6);
                        assert_true(d.is_active() && !d.is_completed()); // implicitly started by main's await
                        yield(); // yield to d
                    });
                    expect(3);
                    assert_true(!d.is_active() && !d.is_completed());
                    yield(); // yield to second child (lazy async is not computing yet)
                    expect(5);
                    assert_true(!d.is_active() && !d.is_completed());
                    assert_equals(d.await(), 42); // starts computing
                    assert_true(!d.is_active() && d.is_completed() && !d.is_cancelled());
                    finish(8);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_simple_exception() {
                run_test([](auto it) {
                             return /* TODO: lambda to check TestException */
                                     dynamic_cast<TestException *>(it) != nullptr;
                         },
                         [this]() {
                             expect(1);
                             auto d = async<void>(CoroutineStart::kLazy, [this]() {
                                 finish(3);
                                 throw TestException();
                             });
                             expect(2);
                             assert_true(!d.is_active() && !d.is_completed());
                             d.await(); // will throw IOException
                         });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_lazy_defer_and_yield_exception() {
                run_test([](auto it) { return dynamic_cast<TestException *>(it) != nullptr; },
                         [this]() {
                             expect(1);
                             auto d = async<void>(CoroutineStart::kLazy, [this]() {
                                 expect(3);
                                 yield(); // this has not effect, because parent coroutine is waiting
                                 finish(4);
                                 throw TestException();
                             });
                             expect(2);
                             assert_true(!d.is_active() && !d.is_completed());
                             d.await(); // will throw IOException
                         });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_catch_exception() {
                run_test([this]() {
                    expect(1);
                    auto d = async<void>(NonCancellable, CoroutineStart::kLazy, [this]() {
                        expect(3);
                        throw TestException();
                    });
                    expect(2);
                    assert_true(!d.is_active() && !d.is_completed());
                    try {
                        d.await(); // will throw IOException
                    } catch (const TestException &e) {
                        assert_true(!d.is_active() && d.is_completed() && d.is_cancelled());
                        expect(4);
                    }
                    finish(5);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_start() {
                run_test([this]() {
                    expect(1);
                    auto d = async(CoroutineStart::kLazy, [this]() {
                        expect(4);
                        return 42;
                    });
                    expect(2);
                    assert_true(!d.is_active() && !d.is_completed());
                    assert_true(d.start());
                    assert_true(d.is_active() && !d.is_completed());
                    expect(3);
                    assert_true(!d.start());
                    yield(); // yield to started coroutine
                    assert_true(!d.is_active() && d.is_completed() && !d.is_cancelled()); // and it finishes
                    expect(5);
                    assert_equals(d.await(), 42); // await sees result
                    finish(6);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_cancel_before_start() {
                run_test([](auto it) { return dynamic_cast<CancellationException *>(it) != nullptr; },
                         [this]() {
                             expect(1);
                             auto d = async(CoroutineStart::kLazy, [this]() {
                                 expect_unreached();
                                 return 42;
                             });
                             expect(2);
                             assert_true(!d.is_active() && !d.is_completed());
                             d.cancel();
                             assert_true(!d.is_active() && d.is_completed() && d.is_cancelled());
                             assert_true(!d.start());
                             finish(3);
                             assert_equals(d.await(), 42); // await shall throw CancellationException
                             expect_unreached();
                         });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_cancel_while_computing() {
                run_test([](auto it) { return dynamic_cast<CancellationException *>(it) != nullptr; },
                         [this]() {
                             expect(1);
                             auto d = async(CoroutineStart::kLazy, [this]() {
                                 expect(4);
                                 yield(); // yield to main, that is going to cancel us
                                 expect_unreached();
                                 return 42;
                             });
                             expect(2);
                             assert_true(!d.is_active() && !d.is_completed() && !d.is_cancelled());
                             assert_true(d.start());
                             assert_true(d.is_active() && !d.is_completed() && !d.is_cancelled());
                             expect(3);
                             yield(); // yield to d
                             expect(5);
                             assert_true(d.is_active() && !d.is_completed() && !d.is_cancelled());
                             d.cancel();
                             assert_true(!d.is_active() && d.is_cancelled()); // cancelling !
                             assert_true(!d.is_active() && d.is_cancelled()); // still cancelling
                             finish(6);
                             assert_equals(d.await(), 42); // await shall throw CancellationException
                             expect_unreached();
                         });
            }
        };
    } // namespace coroutines
} // namespace kotlinx