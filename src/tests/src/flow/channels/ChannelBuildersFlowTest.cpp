// Original: kotlinx-coroutines-core/common/test/flow/channels/ChannelBuildersFlowTest.kt
// TODO: Translate imports to proper C++ includes
// TODO: Implement TestBase base class
// TODO: Implement @Test annotation equivalent
// TODO: Implement runTest coroutine runner
// TODO: Implement produce, Channel
// TODO: Implement consumeAsFlow, receiveAsFlow
// TODO: Implement Flow operations: sum, collect, take, toList
// TODO: Implement repeat, send
// TODO: Implement assertFailsWith, assertTrue
// TODO: Implement NonCancellable, TestException
// TODO: Implement buffer, onCompletion
// TODO: Implement produceIn, cancel, join
// TODO: Implement isClosedForReceive, yield
// TODO: Implement assertSame, assertNotSame
// TODO: Implement wrapperDispatcher

#include <vector>
// TODO: #include proper headers

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            class ChannelBuildersFlowTest : public TestBase {
            public:
                // @Test
                void test_channel_consume_as_flow() {
                    run_test([]() -> /* suspend */ void {
                        auto channel = produce([](auto &producer) -> /* suspend */ void {
                            for (int i = 0; i < 10; ++i) {
                                producer.send(i + 1);
                            }
                        });
                        auto flow_instance = channel.consume_as_flow();
                        assert_equals(55, flow_instance.sum());
                        assert_fails_with<IllegalStateException>([&]() { flow_instance.collect(); });
                    });
                }

                // @Test
                void test_channel_receive_as_flow() {
                    run_test([]() -> /* suspend */ void {
                        auto channel = produce([](auto &producer) -> /* suspend */ void {
                            for (int i = 0; i < 10; ++i) {
                                producer.send(i + 1);
                            }
                        });
                        auto flow_instance = channel.receive_as_flow();
                        assert_equals(55, flow_instance.sum());
                        assert_equals(std::vector<int>(), flow_instance.to_list());
                    });
                }

                // @Test
                void test_consume_as_flow_cancellation() {
                    run_test([]() -> /* suspend */ void {
                        auto channel = produce(NonCancellable, [](auto &producer) -> /* suspend */ void {
                            // otherwise failure will cancel scope as well
                            for (int i = 0; i < 10; ++i) {
                                producer.send(i + 1);
                            }
                            throw TestException();
                        });
                        auto flow_instance = channel.consume_as_flow();
                        assert_equals(15, flow_instance.take(5).sum());
                        // the channel should have been canceled, even though took only 5 elements
                        assert_true(channel.is_closed_for_receive());
                        assert_fails_with<IllegalStateException>([&]() { flow_instance.collect(); });
                    });
                }

                // @Test
                void test_receive_as_flow_cancellation() {
                    run_test([]() -> /* suspend */ void {
                        auto channel = produce(NonCancellable, [](auto &producer) -> /* suspend */ void {
                            // otherwise failure will cancel scope as well
                            for (int i = 0; i < 10; ++i) {
                                producer.send(i + 1);
                            }
                            throw TestException();
                        });
                        auto flow_instance = channel.receive_as_flow();
                        assert_equals(15, flow_instance.take(5).sum()); // sum of first 5
                        assert_equals(40, flow_instance.take(5).sum()); // sum the rest 5
                        assert_fails_with<TestException>([&]() { flow_instance.sum(); }); // exception in the rest
                    });
                }

                // @Test
                void test_consume_as_flow_exception() {
                    run_test([]() -> /* suspend */ void {
                        auto channel = produce(NonCancellable, [](auto &producer) -> /* suspend */ void {
                            // otherwise failure will cancel scope as well
                            for (int i = 0; i < 10; ++i) {
                                producer.send(i + 1);
                            }
                            throw TestException();
                        });
                        auto flow_instance = channel.consume_as_flow();
                        assert_fails_with<TestException>([&]() { flow_instance.sum(); });
                        assert_fails_with<IllegalStateException>([&]() { flow_instance.collect(); });
                    });
                }

                // @Test
                void test_receive_as_flow_exception() {
                    run_test([]() -> /* suspend */ void {
                        auto channel = produce(NonCancellable, [](auto &producer) -> /* suspend */ void {
                            // otherwise failure will cancel scope as well
                            for (int i = 0; i < 10; ++i) {
                                producer.send(i + 1);
                            }
                            throw TestException();
                        });
                        auto flow_instance = channel.receive_as_flow();
                        assert_fails_with<TestException>([&]() { flow_instance.sum(); });
                        assert_fails_with<TestException>([&]() { flow_instance.collect(); });
                        // repeated collection -- same exception
                    });
                }

                // @Test
                void test_consume_as_flow_produce_fusing() {
                    run_test([]() -> /* suspend */ void {
                        auto channel = produce([](auto &producer) -> /* suspend */ void {
                            producer.send("OK");
                        });
                        auto flow_instance = channel.consume_as_flow();
                        assert_same(&channel, &flow_instance.produce_in(*this));
                        assert_fails_with<IllegalStateException>([&]() { flow_instance.produce_in(*this); });
                        channel.cancel();
                    });
                }

                // @Test
                void test_receive_as_flow_produce_fusing() {
                    run_test([]() -> /* suspend */ void {
                        auto channel = produce([](auto &producer) -> /* suspend */ void {
                            producer.send("OK");
                        });
                        auto flow_instance = channel.receive_as_flow();
                        assert_same(&channel, &flow_instance.produce_in(*this));
                        assert_same(&channel, &flow_instance.produce_in(*this)); // can use produce multiple times
                        channel.cancel();
                    });
                }

                // @Test
                void test_consume_as_flow_produce_buffered() {
                    run_test([]() -> /* suspend */ void {
                        expect(1);
                        auto channel = produce([](auto &producer) -> /* suspend */ void {
                            expect(3);
                            for (int i = 1; i <= 10; ++i) {
                                producer.send(i);
                            }
                            expect(4); // produces everything because of buffering
                        });
                        auto flow_instance = channel.consume_as_flow().buffer(); // request buffering
                        expect(2); // producer is not running yet
                        auto result = flow_instance.produce_in(*this);
                        // run the flow pipeline until it consumes everything into buffer
                        while (!channel.is_closed_for_receive()) {
                            yield();
                        }
                        expect(5); // produced had done running (buffered stuff)
                        assert_not_same(&channel, &result);
                        assert_fails_with<IllegalStateException>([&]() { flow_instance.produce_in(*this); });
                        // check that we received everything
                        std::vector<int> expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
                        assert_equals(expected, result.to_list());
                        finish(6);
                    });
                }

                // @Test
                void test_produce_in_atomicity() {
                    run_test([]() -> /* suspend */ void {
                        auto flow_instance = flow_of(1).on_completion([](auto) {
                            expect(2);
                        });
                        CoroutineScope scope(wrapper_dispatcher());
                        flow_instance.produce_in(scope);
                        expect(1);
                        scope.cancel();
                        scope.coroutine_context()[Job].join();
                        finish(3);
                    });
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx