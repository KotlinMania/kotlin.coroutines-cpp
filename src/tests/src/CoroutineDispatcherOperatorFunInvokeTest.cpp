// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/CoroutineDispatcherOperatorFunInvokeTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.coroutines.testing.*
        // TODO: import kotlin.coroutines.ContinuationInterceptor
        // TODO: import kotlin.coroutines.CoroutineContext
        // TODO: import kotlin.test.*

        class CoroutineDispatcherOperatorFunInvokeTest : public TestBase {
        public:
            /**
     * Copy pasted from [WithContextTest.testThrowException],
     * then edited to use operator.
     */
            // @Test
            void test_throw_exception() {
                run_test([this]() {
                    expect(1);
                    try {
                        wrapped_current_dispatcher()([this]() {
                            expect(2);
                            throw AssertionError();
                        });
                    } catch (const AssertionError &e) {
                        expect(3);
                    }

                    yield();
                    finish(4);
                });
            }

            /**
     * Copy pasted from [WithContextTest.testWithContextChildWaitSameContext],
     * then edited to use operator fun invoke for [CoroutineDispatcher].
     */
            // @Test
            void test_with_context_child_wait_same_context() {
                run_test([this]() {
                    expect(1);
                    wrapped_current_dispatcher()([this]() {
                        expect(2);
                        launch([this]() {
                            // ^^^ schedules to main thread
                            expect(4); // waits before return
                        });
                        expect(3);
                        return Wrapper("OK");
                    }).unwrap();
                    finish(5);
                });
            }

        private:
            class Wrapper : public Incomplete {
            public:
                std::string value;

                explicit Wrapper(std::string v) : value(std::move(v)) {
                }

                bool is_active() const override {
                    throw std::runtime_error("");
                }

                NodeList *list() const override {
                    throw std::runtime_error("");
                }
            };

            Wrapper wrap(const std::string &value) {
                return Wrapper(value);
            }

            std::string unwrap(const Wrapper &wrapper) {
                return wrapper.value;
            }

            class WrappedDispatcher : public CoroutineDispatcher {
            public:
                CoroutineDispatcher &dispatcher;

                explicit WrappedDispatcher(CoroutineDispatcher &d) : dispatcher(d) {
                }

                void dispatch(CoroutineContext context, Runnable block) override {
                    dispatcher.dispatch(context, block);
                }

                bool is_dispatch_needed(CoroutineContext context) const override {
                    return dispatcher.is_dispatch_needed(context);
                }

                // @InternalCoroutinesApi
                void dispatch_yield(CoroutineContext context, Runnable block) override {
                    dispatcher.dispatch_yield(context, block);
                }
            };

            WrappedDispatcher wrapped_current_dispatcher() {
                auto &dispatcher = static_cast<CoroutineDispatcher &>(
                    *coroutine_context[ContinuationInterceptor()]
                );
                return WrappedDispatcher(dispatcher);
            }
        };
    } // namespace coroutines
} // namespace kotlinx