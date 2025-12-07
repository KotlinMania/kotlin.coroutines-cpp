// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/CancellableContinuationHandlersTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

// @file:Suppress("NAMED_ARGUMENTS_NOT_ALLOWED") // KT-21913

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.internal.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.test.*

class CancellableContinuationHandlersTest : public TestBase {
public:
    // @Test
    void test_double_subscription() {
        run_test([](auto it) { return dynamic_cast<IllegalStateException*>(it) != nullptr; },
        [this]() {
            suspend_cancellable_coroutine<void>([this](auto c) {
                c.invoke_on_cancellation([this]() { finish(1); });
                c.invoke_on_cancellation([this]() { expect_unreached(); });
            });
        });
    }

    // @Test
    void test_double_subscription_after_completion() {
        run_test([this]() {
            suspend_cancellable_coroutine<void>([this](auto c) {
                c.resume();
                // First invokeOnCancellation is Ok
                c.invoke_on_cancellation([this]() { expect_unreached(); });
                // Second invokeOnCancellation is not allowed
                assert_fails_with<IllegalStateException>([&]() {
                    c.invoke_on_cancellation([this]() { expect_unreached(); });
                });
            });
        });
    }

    // @Test
    void test_double_subscription_after_completion_with_exception() {
        run_test([this]() {
            assert_fails_with<TestException>([this]() {
                suspend_cancellable_coroutine<void>([this](auto c) {
                    c.resume_with_exception(TestException());
                    // First invokeOnCancellation is Ok
                    c.invoke_on_cancellation([this]() { expect_unreached(); });
                    // Second invokeOnCancellation is not allowed
                    assert_fails_with<IllegalStateException>([&]() {
                        c.invoke_on_cancellation([this]() { expect_unreached(); });
                    });
                });
            });
        });
    }

    // @Test
    void test_double_subscription_after_cancellation() {
        run_test([this]() {
            try {
                suspend_cancellable_coroutine<void>([this](auto c) {
                    c.cancel();
                    c.invoke_on_cancellation([this](auto it) {
                        assert_is<CancellationException>(it);
                        expect(1);
                    });
                    assert_fails_with<IllegalStateException>([&]() {
                        c.invoke_on_cancellation([this]() { expect_unreached(); });
                    });
                });
            } catch (const CancellationException& e) {
                finish(2);
            }
        });
    }

    // @Test
    void test_second_subscription_after_cancellation() {
        run_test([this]() {
            try {
                suspend_cancellable_coroutine<void>([this](auto c) {
                    // Set IOC first
                    c.invoke_on_cancellation([this](auto it) {
                        assert_null(it);
                        expect(2);
                    });
                    expect(1);
                    // then cancel (it gets called)
                    c.cancel();
                    // then try to install another one
                    assert_fails_with<IllegalStateException>([&]() {
                        c.invoke_on_cancellation([this]() { expect_unreached(); });
                    });
                });
            } catch (const CancellationException& e) {
                finish(3);
            }
        });
    }

    // @Test
    void test_second_subscription_after_resume_cancel_and_dispatch() {
        run_test([this]() {
            CancellableContinuation<void>* cont = nullptr;
            auto job = launch(CoroutineStart::kUndispatched, [this, &cont]() {
                // will be cancelled during dispatch
                assert_fails_with<CancellationException>([this, &cont]() {
                    suspend_cancellable_coroutine<void>([this, &cont](auto c) {
                        cont = &c;
                        // Set IOC first -- not called (completed)
                        c.invoke_on_cancellation([this](auto it) {
                            assert_is<CancellationException>(it);
                            expect(4);
                        });
                        expect(1);
                    });
                });
                expect(5);
            });
            expect(2);
            // then resume it
            cont->resume(); // schedule cancelled continuation for dispatch
            // then cancel the job during dispatch
            job.cancel();
            expect(3);
            yield(); // finish dispatching (will call IOC handler here!)
            expect(6);
            // then try to install another one after we've done dispatching it
            assert_fails_with<IllegalStateException>([&]() {
                cont->invoke_on_cancellation([this]() { expect_unreached(); });
            });
            finish(7);
        });
    }

    // @Test
    void test_double_subscription_after_cancellation_with_cause() {
        run_test([this]() {
            try {
                suspend_cancellable_coroutine<void>([this](auto c) {
                    c.cancel(AssertionError());
                    c.invoke_on_cancellation([this](auto it) {
                        require(dynamic_cast<AssertionError*>(it) != nullptr);
                        expect(1);
                    });
                    assert_fails_with<IllegalStateException>([&]() {
                        c.invoke_on_cancellation([this]() { expect_unreached(); });
                    });
                });
            } catch (const AssertionError& e) {
                finish(2);
            }
        });
    }

    // @Test
    void test_double_subscription_mixed() {
        run_test([this]() {
            try {
                suspend_cancellable_coroutine<void>([this](auto c) {
                    c.invoke_on_cancellation([this](auto it) {
                        require(dynamic_cast<IndexOutOfBoundsException*>(it) != nullptr);
                        expect(1);
                    });
                    c.cancel(IndexOutOfBoundsException());
                    assert_fails_with<IllegalStateException>([&]() {
                        c.invoke_on_cancellation([this]() { expect_unreached(); });
                    });
                });
            } catch (const IndexOutOfBoundsException& e) {
                finish(2);
            }
        });
    }

    // @Test
    void test_exception_in_handler() {
        run_test(
            /* unhandled = */ std::vector<std::function<bool(std::exception*)>>{
                [](auto it) { return dynamic_cast<CompletionHandlerException*>(it) != nullptr; }
            },
            [this]() {
                expect(1);
                try {
                    suspend_cancellable_coroutine<void>([this](auto c) {
                        c.invoke_on_cancellation([this]() { throw AssertionError(); });
                        c.cancel();
                    });
                } catch (const CancellationException& e) {
                    expect(2);
                }
                finish(3);
            }
        );
    }

    // @Test
    void test_segment_as_handler() {
        run_test([this]() {
            class MySegment : public Segment<MySegment> {
            public:
                MySegment() : Segment<MySegment>(0, nullptr, 0) {}

                int number_of_slots() const override { return 0; }

                bool invoke_on_cancellation_called = false;
                void on_cancellation(int index, std::exception* cause, CoroutineContext context) override {
                    invoke_on_cancellation_called = true;
                }
            };

            MySegment s;
            expect(1);
            try {
                suspend_cancellable_coroutine<void>([this, &s](auto c) {
                    expect(2);
                    // TODO: c as CancellableContinuationImpl<*>
                    c.invoke_on_cancellation(s, 0);
                    c.cancel();
                });
            } catch (const CancellationException& e) {
                expect(3);
            }
            expect(4);
            check(s.invoke_on_cancellation_called);
            finish(5);
        });
    }
};

} // namespace coroutines
} // namespace kotlinx
