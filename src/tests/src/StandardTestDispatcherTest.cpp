// Original file: kotlinx-coroutines-test/common/test/StandardTestDispatcherTest.kt
// TODO: Remove or convert import statements
// TODO: Convert @Test, @BeforeTest, @AfterTest annotations to appropriate test framework
// TODO: Convert suspend functions and coroutine builders
// TODO: Handle OrderedExecutionTestBase inheritance
// TODO: Convert void() extension function pattern

namespace kotlinx {
    namespace coroutines {
        namespace test {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlinx.coroutines.flow.*
            // TODO: import kotlin.test.*

            class StandardTestDispatcherTest : public OrderedExecutionTestBase {
            private:
                TestScope scope_{StandardTestDispatcher()};

            public:
                // TODO: @BeforeTest
                void init() {
                    scope_.as_specific_implementation().enter();
                }

                // TODO: @AfterTest
                void cleanup() {
                    scope_.run_current();
                    auto exceptions = scope_.as_specific_implementation().legacy_leave();
                    assert(exceptions.empty());
                }

                /** Tests that the [StandardTestDispatcher] follows an execution order similar to `runBlocking`. */
                // TODO: @Test
                void test_flows_not_skipping_values() {
                    // https://github.com/Kotlin/kotlinx.coroutines/issues/1626#issuecomment-554632852
                    scope_.launch([&]() {
                        auto list = flow_of(1)
                                .on_start([&]() { emit(0); })
                                .combine(flow_of("A"), [](int int_val, std::string str) {
                                    return str + std::to_string(int_val);
                                })
                                .to_list();
                        assert(list == std::vector<std::string>{"A0", "A1"});
                    }).void_result();
                }

                /** Tests that each [launch] gets dispatched. */
                // TODO: @Test
                void test_launch_dispatched() {
                    scope_.launch([&]() {
                        expect(1);
                        launch([&]() {
                            expect(3);
                        });
                        finish(2);
                    }).void_result();
                }

                /** Tests that dispatching is done in a predictable order and [yield] puts this task at the end of the queue. */
                // TODO: @Test
                void test_yield() {
                    scope_.launch([&]() {
                        expect(1);
                        scope_.launch([&]() {
                            expect(3);
                            yield();
                            expect(6);
                        });
                        scope_.launch([&]() {
                            expect(4);
                            yield();
                            finish(7);
                        });
                        expect(2);
                        yield();
                        expect(5);
                    }).void_result();
                }

                /** Tests that the [TestCoroutineScheduler] used for [Dispatchers.Main] gets used by default. */
                // TODO: @Test
                void test_scheduler_reuse() {
                    auto dispatcher1 = StandardTestDispatcher();
                    Dispatchers::set_main(dispatcher1);
                    try {
                        auto dispatcher2 = StandardTestDispatcher();
                        // TODO: assertSame
                        assert(dispatcher1.scheduler == dispatcher2.scheduler);
                    }
                    finally{
                        Dispatchers::reset_main();
        
                    }
                }
            };
        } // namespace test
    } // namespace coroutines
} // namespace kotlinx