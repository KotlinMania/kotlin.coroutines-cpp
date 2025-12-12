// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/CancelledParentAttachTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.coroutines.testing.*
        // TODO: import kotlinx.coroutines.channels.*
        // TODO: import kotlinx.coroutines.flow.internal.*
        // TODO: import kotlin.test.*

        class CancelledParentAttachTest : public TestBase {
        public:
            // @Test
            void test_async() {
                run_test([this]() {
                    for (auto start: CoroutineStart::entries) {
                        test_async_cancelled_parent(start);
                    }
                });
            }

        private:
            void test_async_cancelled_parent(CoroutineStart start) {
                try {
                    with_context(Job(), [this, start]() {
                        cancel();
                        expect(1);
                        auto d = async<int>(start, [this]() { return 42; });
                        expect(2);
                        d.invoke_on_completion([this]() {
                            finish(3);
                            reset();
                        });
                    });
                    expect_unreached();
                } catch (const CancellationException &) {
                    // Expected
                }
            }

        public:
            // @Test
            void test_launch() {
                run_test([this]() {
                    for (auto start: CoroutineStart::entries) {
                        test_launch_cancelled_parent(start);
                    }
                });
            }

        private:
            void test_launch_cancelled_parent(CoroutineStart start) {
                try {
                    with_context(Job(), [this, start]() {
                        cancel();
                        expect(1);
                        auto d = launch(start, [this]() {
                        });
                        expect(2);
                        d.invoke_on_completion([this]() {
                            finish(3);
                            reset();
                        });
                    });
                    expect_unreached();
                } catch (const CancellationException &) {
                    // Expected
                }
            }

        public:
            // @Test
            void test_produce() {
                run_test([](auto it) { return dynamic_cast<CancellationException *>(it) != nullptr; },
                         [this]() {
                             cancel();
                             expect(1);
                             auto d = produce<int>([this]() {
                             });
                             expect(2);
                             static_cast<Job &>(d).invoke_on_completion([this]() {
                                 finish(3);
                                 reset();
                             });
                         });
            }

            // @Test
            void test_broadcast() {
                run_test([this]() {
                    for (auto start: CoroutineStart::entries) {
                        test_broadcast_cancelled_parent(start);
                    }
                });
            }

        private:
            // @Suppress("DEPRECATION_ERROR")
            void test_broadcast_cancelled_parent(CoroutineStart start) {
                try {
                    with_context(Job(), [this, start]() {
                        cancel();
                        expect(1);
                        auto bc = broadcast<int>(start, []() {
                        });
                        expect(2);
                        static_cast<Job &>(bc).invoke_on_completion([this]() {
                            finish(3);
                            reset();
                        });
                    });
                    expect_unreached();
                } catch (const CancellationException &) {
                    // Expected
                }
            }

        public:
            // @Test
            void test_scopes() {
                run_test([this]() {
                    test_scope([]() {
                        coroutine_scope([]() {
                        });
                    });
                    test_scope([]() {
                        supervisor_scope([]() {
                        });
                    });
                    test_scope([]() {
                        flow_scope([]() {
                        });
                    });
                    test_scope([]() {
                        with_timeout(LONG_MAX, []() {
                        });
                    });
                    test_scope([]() {
                        with_context(Job(), []() {
                        });
                    });
                    test_scope([]() {
                        with_context(CoroutineName(""), []() {
                        });
                    });
                });
            }

        private:
            template<typename Block>
            void test_scope(Block block) {
                try {
                    with_context(Job(), [&block]() {
                        cancel();
                        block();
                    });
                    expect_unreached();
                } catch (const CancellationException &) {
                    // Expected
                }
            }
        };
    } // namespace coroutines
} // namespace kotlinx