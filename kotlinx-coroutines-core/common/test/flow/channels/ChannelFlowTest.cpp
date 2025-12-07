// Original: kotlinx-coroutines-core/common/test/flow/channels/ChannelFlowTest.kt
// TODO: Translate imports to proper C++ includes
// TODO: Implement TestBase base class
// TODO: Implement @Test annotation equivalent
// TODO: Implement runTest coroutine runner
// TODO: Implement channelFlow builder
// TODO: Implement trySend, Channel constants (CONFLATED)
// TODO: Implement buffer, toList
// TODO: Implement onEach, TestException
// TODO: Implement assertFailsWith
// TODO: Implement invokeOnClose, launch with CoroutineStart.ATOMIC
// TODO: Implement hang, take
// TODO: Implement callbackFlow, awaitClose, close
// TODO: Implement @Ignore annotation equivalent

#include <vector>
// TODO: #include proper headers

namespace kotlinx {
namespace coroutines {
namespace flow {

class ChannelFlowTest : public TestBase {
public:
    // @Test
    void test_regular() {
        run_test([]() -> /* suspend */ void {
            auto flow_instance = channel_flow([](auto& scope) -> /* suspend */ void {
                assert_true(scope.try_send(1).is_success());
                assert_true(scope.try_send(2).is_success());
                assert_true(scope.try_send(3).is_success());
            });
            assert_equals(std::vector<int>{1, 2, 3}, flow_instance.to_list());
        });
    }

    // @Test
    void test_buffer() {
        run_test([]() -> /* suspend */ void {
            auto flow_instance = channel_flow([](auto& scope) -> /* suspend */ void {
                assert_true(scope.try_send(1).is_success());
                assert_true(scope.try_send(2).is_success());
                assert_false(scope.try_send(3).is_success());
            }).buffer(1);
            assert_equals(std::vector<int>{1, 2}, flow_instance.to_list());
        });
    }

    // @Test
    void test_conflated() {
        run_test([]() -> /* suspend */ void {
            auto flow_instance = channel_flow([](auto& scope) -> /* suspend */ void {
                assert_true(scope.try_send(1).is_success());
                assert_true(scope.try_send(2).is_success());
                assert_true(scope.try_send(3).is_success());
                assert_true(scope.try_send(4).is_success());
            }).buffer(Channel::CONFLATED);
            assert_equals(std::vector<int>{1, 4}, flow_instance.to_list()); // two elements in the middle got conflated
        });
    }

    // @Test
    void test_failure_cancels_channel() {
        run_test([]() -> /* suspend */ void {
            auto flow_instance = channel_flow([](auto& scope) -> /* suspend */ void {
                scope.try_send(1);
                scope.invoke_on_close([](auto) {
                    expect(2);
                });
            }).on_each([](int) {
                throw TestException();
            });

            expect(1);
            assert_fails_with<TestException>([&]() { flow_instance.collect(); });
            finish(3);
        });
    }

    // @Test
    void test_failure_in_source_cancels_consumer() {
        run_test([]() -> /* suspend */ void {
            auto flow_instance = channel_flow<int>([](auto& scope) -> /* suspend */ void {
                expect(2);
                throw TestException();
            }).on_each([](int) {
                expect_unreached();
            });

            expect(1);
            assert_fails_with<TestException>([&]() { flow_instance.collect(); });
            finish(3);
        });
    }

    // @Test
    void test_scoped_cancellation() {
        run_test([]() -> /* suspend */ void {
            auto flow_instance = channel_flow<int>([](auto& scope) -> /* suspend */ void {
                expect(2);
                scope.launch(CoroutineStart::ATOMIC, [](auto&) -> /* suspend */ void {
                    hang([](){ expect(3); });
                });
                throw TestException();
            }).on_each([](int) {
                expect_unreached();
            });

            expect(1);
            assert_fails_with<TestException>([&]() { flow_instance.collect(); });
            finish(4);
        });
    }

    // @Test
    void test_merge_one_coroutine_with_cancellation() {
        run_test([]() -> /* suspend */ void {
            auto flow_instance = flow_of(1, 2, 3);
            auto f = merge_one_coroutine(flow_instance, flow_instance).take(2);
            assert_equals(std::vector<int>{1, 1}, f.to_list());
        });
    }

    // @Test
    void test_merge_two_coroutines_with_cancellation() {
        run_test([]() -> /* suspend */ void {
            auto flow_instance = flow_of(1, 2, 3);
            auto f = merge_two_coroutines(flow_instance, flow_instance).take(2);
            assert_equals(std::vector<int>{1, 1}, f.to_list());
        });
    }

private:
    Flow<int> merge_two_coroutines(Flow<int>& self, Flow<int>& other) {
        return channel_flow([&](auto& scope) -> /* suspend */ void {
            scope.launch([&]() -> /* suspend */ void {
                self.collect([&](int it) -> /* suspend */ void {
                    scope.send(it);
                    yield();
                });
            });
            scope.launch([&]() -> /* suspend */ void {
                other.collect([&](int it) -> /* suspend */ void {
                    scope.send(it);
                });
            });
        });
    }

    Flow<int> merge_one_coroutine(Flow<int>& self, Flow<int>& other) {
        return channel_flow([&](auto& scope) -> /* suspend */ void {
            scope.launch([&]() -> /* suspend */ void {
                self.collect([&](int it) -> /* suspend */ void {
                    scope.send(it);
                    yield();
                });
            });

            other.collect([&](int it) -> /* suspend */ void {
                scope.send(it);
                yield();
            });
        });
    }

public:
    // @Test
    // @Ignore // #1374
    void test_buffer_with_timeout() {
        run_test([]() -> /* suspend */ void {
            auto buffer_with_timeout = [](Flow<int>& flow_instance) -> Flow<int> {
                return channel_flow([&](auto& scope) -> /* suspend */ void {
                    expect(2);
                    scope.launch([](auto&) -> /* suspend */ void {
                        expect(3);
                        hang([](){ expect(5); });
                    });
                    scope.launch([&]() -> /* suspend */ void {
                        expect(4);
                        flow_instance.collect([&](int it) -> /* suspend */ void {
                            with_timeout(-1, [&]() -> /* suspend */ void {
                                scope.send(it);
                            });
                            expect_unreached();
                        });
                        expect_unreached();
                    });
                });
            };

            auto flow_instance = flow_of(1, 2, 3);
            auto result = buffer_with_timeout(flow_instance);
            expect(1);
            assert_fails_with<TimeoutCancellationException>([&]() { result.collect(); });
            finish(6);
        });
    }

    // @Test
    void test_child_cancellation() {
        run_test([]() -> /* suspend */ void {
            channel_flow([](auto& scope) -> /* suspend */ void {
                auto job = scope.launch([](auto&) -> /* suspend */ void {
                    expect(2);
                    hang([](){ expect(4); });
                });
                expect(1);
                yield();
                expect(3);
                job.cancel_and_join();
                scope.send(5);
            }).collect([](int it) {
                expect(it);
            });

            finish(6);
        });
    }

    // @Test
    void test_closed_prematurely() {
        // runTest with unhandled exception list
        run_test(
            /* unhandled = */ [](const std::exception_ptr& e) -> bool {
                try {
                    if (e) std::rethrow_exception(e);
                    return false;
                } catch (const ClosedSendChannelException&) {
                    return true;
                } catch (...) {
                    return false;
                }
            },
            []() -> /* suspend */ void {
                CoroutineScope& outer_scope = *this;
                auto flow_instance = channel_flow([&](auto& scope) -> /* suspend */ void {
                    // ~ callback-based API, no children
                    outer_scope.launch(Job(), [&](auto& launch_scope) -> /* suspend */ void {
                        expect(2);
                        scope.send(1);
                        expect_unreached();
                    });
                    expect(1);
                });
                assert_equals(std::vector<int>(), flow_instance.to_list());
                finish(3);
            }
        );
    }

    // @Test
    void test_not_closed_prematurely() {
        run_test([]() -> /* suspend */ void {
            CoroutineScope& outer_scope = *this;
            auto flow_instance = channel_flow([&](auto& scope) -> /* suspend */ void {
                // ~ callback-based API
                outer_scope.launch(Job(), [&](auto& launch_scope) -> /* suspend */ void {
                    expect(2);
                    scope.send(1);
                    scope.close();
                });
                expect(1);
                scope.await_close();
            });

            assert_equals(std::vector<int>{1}, flow_instance.to_list());
            finish(3);
        });
    }

    // @Test
    void test_cancelled_on_completion() {
        run_test([]() -> /* suspend */ void {
            auto my_flow = callback_flow</* Any */ int>([](auto& scope) -> /* suspend */ void {
                expect(2);
                scope.close();
                hang([](){ expect(3); });
            });

            expect(1);
            my_flow.collect();
            finish(4);
        });
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
