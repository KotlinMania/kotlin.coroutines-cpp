// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/AbstractCoroutineTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

// @Suppress("DEPRECATION") // cancel(cause)


    namespace kotlinx::coroutines {
        // TODO: import kotlinx.coroutines.testing.*
        // TODO: import kotlin.coroutines.*
        // TODO: import kotlin.test.*

        class AbstractCoroutineTest : public TestBase {
        public:
            // @Test
            // TODO: Translate @Test annotation
            void test_notifications() {
                run_test([this]() {
                    expect(1);
                    auto coroutine_context_ref = coroutine_context; // workaround for KT-22984
                    auto coroutine = /* TODO: object expression */ [&]() {
                        struct : public AbstractCoroutine<std::string> {
                            AbstractCoroutineTest *test;

                            void on_start() override {
                                test->expect(3);
                            }

                            void on_cancelling(Throwable *cause) override {
                                assert_null(cause);
                                test->expect(5);
                            }

                            void on_completed(std::string value) override {
                                assert_equals("OK", value);
                                test->expect(6);
                            }

                            void on_cancelled(Throwable cause, bool handled) override {
                                test->expect_unreached();
                            }
                        } instance;
                        instance.test = this;
                        return instance;
                    }();

                    coroutine.invoke_on_completion(true, [this](auto it) {
                        assert_null(it);
                        expect(7);
                    });

                    coroutine.invoke_on_completion([this](auto it) {
                        assert_null(it);
                        expect(8);
                    });
                    expect(2);
                    coroutine.start();
                    expect(4);
                    coroutine.resume("OK");
                    finish(9);
                });
            }

            // @Test
            // TODO: Translate @Test annotation
            void test_notifications_with_exception() {
                run_test([this]() {
                    expect(1);
                    auto coroutine_context_ref = coroutine_context; // workaround for KT-22984
                    auto coroutine = /* TODO: object expression */ [&]() {
                        struct : public AbstractCoroutine<std::string> {
                            AbstractCoroutineTest *test;

                            void on_start() override {
                                test->expect(3);
                            }

                            void on_cancelling(Throwable *cause) override {
                                assert_is<TestException1>(cause);
                                test->expect(5);
                            }

                            void on_completed(std::string value) override {
                                test->expect_unreached();
                            }

                            void on_cancelled(Throwable cause, bool handled) override {
                                assert_is<TestException1>(cause);
                                test->expect(8);
                            }
                        } instance;
                        instance.test = this;
                        return instance;
                    }();

                    coroutine.invoke_on_completion(true, [this](auto it) {
                        assert_is<TestException1>(it);
                        expect(6);
                    });

                    coroutine.invoke_on_completion([this](auto it) {
                        assert_is<TestException1>(it);
                        expect(9);
                    });

                    expect(2);
                    coroutine.start();
                    expect(4);
                    coroutine.cancel_coroutine(TestException1());
                    expect(7);
                    coroutine.resume_with_exception(TestException2());
                    finish(10);
                });
            }
        };
    } // namespace kotlinx::coroutines
