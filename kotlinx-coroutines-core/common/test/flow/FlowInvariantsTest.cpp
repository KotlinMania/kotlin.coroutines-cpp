// Original: kotlinx-coroutines-core/common/test/flow/FlowInvariantsTest.kt
// TODO: Translate imports to proper C++ includes
// TODO: Implement TestBase base class
// TODO: Implement @Test annotation equivalent
// TODO: Implement runTest, runCatching, exceptionOrNull
// TODO: Implement Flow, FlowCollector, AbstractFlow
// TODO: Implement withContext, coroutineScope, launch
// TODO: Implement channels (produce, Channel, consumeEach)
// TODO: Implement KClass reflection (expectedException checking)
// TODO: Implement CoroutineContext, CoroutineName, NonCancellable
// TODO: Implement Dispatchers and NamedDispatchers
// TODO: Implement flowOn, launchIn, buffer, onEach
// TODO: Implement assertFailsWith

#include <functional>
#include <optional>
#include <stdexcept>
// TODO: #include proper headers

namespace kotlinx {
namespace coroutines {
namespace flow {

class FlowInvariantsTest : public TestBase {
private:
    template<typename T>
    void run_parametrized_test(
        std::optional</* KClass<out Throwable> */> expected_exception = std::nullopt,
        std::function<void(std::function<Flow<T>(std::function<void(FlowCollector<T>&)>)>)> test_body
    ) {
        run_test([&]() -> /* suspend */ void {
            auto r1 = run_catching([&]() { test_body([](auto block) { return flow(block); }); }).exception_or_null();
            check(r1, expected_exception);
            reset();

            auto r2 = run_catching([&]() { test_body([](auto block) { return abstract_flow(block); }); }).exception_or_null();
            check(r2, expected_exception);
        });
    }

    template<typename T>
    Flow<T> abstract_flow(std::function<void(FlowCollector<T>&)> block) {
        // object : AbstractFlow<T>()
        struct FlowImpl : public AbstractFlow<T> {
            std::function<void(FlowCollector<T>&)> block_;

            FlowImpl(std::function<void(FlowCollector<T>&)> b) : block_(b) {}

            void collect_safely(FlowCollector<T>& collector) /* suspend */ override {
                block_(collector);
            }
        };
        return FlowImpl(block);
    }

    void check(std::exception_ptr exception, std::optional</* KClass<out Throwable> */> expected_exception) {
        if (expected_exception.has_value() && exception == nullptr) {
            fail("Expected exception, but test completed successfully");
        }
        if (expected_exception.has_value() && exception != nullptr) {
            // TODO: assertTrue(expectedException.isInstance(exception))
            assert_true(/* is_instance check */);
        }
        if (!expected_exception.has_value() && exception != nullptr) {
            std::rethrow_exception(exception);
        }
    }

public:
    // @Test
    void test_with_context_contract() {
        run_parametrized_test<int>(/* IllegalStateException::class */ std::nullopt, [](auto flow_factory) -> /* suspend */ void {
            flow_factory([](FlowCollector<int>& collector) -> /* suspend */ void {
                with_context(NonCancellable, [&]() -> /* suspend */ void {
                    collector.emit(1);
                });
            }).collect([](int value) {
                expect_unreached();
            });
        });
    }

    // @Test
    void test_with_dispatcher_contract_violated() {
        run_parametrized_test<int>(/* IllegalStateException::class */ std::nullopt, [](auto flow_factory) -> /* suspend */ void {
            flow_factory([](FlowCollector<int>& collector) -> /* suspend */ void {
                with_context(NamedDispatchers("foo"), [&]() -> /* suspend */ void {
                    collector.emit(1);
                });
            }).collect([](int value) {
                expect_unreached();
            });
        });
    }

    // @Test
    void test_with_name_contract_violated() {
        run_parametrized_test<int>(/* IllegalStateException::class */ std::nullopt, [](auto flow_factory) -> /* suspend */ void {
            flow_factory([](FlowCollector<int>& collector) -> /* suspend */ void {
                with_context(CoroutineName("foo"), [&]() -> /* suspend */ void {
                    collector.emit(1);
                });
            }).collect([](int value) {
                expect_unreached();
            });
        });
    }

    // @Test
    void test_with_context_does_not_change_execution() {
        run_test([]() -> /* suspend */ void {
            auto flow_instance = flow([](FlowCollector<std::string>& collector) -> /* suspend */ void {
                collector.emit(NamedDispatchers::name());
            }).flow_on(NamedDispatchers("original"));

            std::string result = "unknown";
            with_context(NamedDispatchers("misc"), [&]() -> /* suspend */ void {
                flow_instance
                    .flow_on(NamedDispatchers("upstream"))
                    .launch_in(*this + NamedDispatchers("consumer"), [&](auto& scope) {
                        scope.on_each([&](const std::string& it) {
                            result = it;
                        });
                    }).join();
            });
            assert_equals("original", result);
        });
    }

    // @Test
    void test_scoped_job() {
        run_parametrized_test<int>(/* IllegalStateException::class */ std::nullopt, [](auto flow_factory) -> /* suspend */ void {
            flow_factory([](FlowCollector<int>& collector) -> /* suspend */ void {
                collector.emit(1);
            }).buffer(EmptyCoroutineContext, flow_factory).collect([](int value) {
                expect(1);
            });
            finish(2);
        });
    }

    // @Test
    void test_scoped_job_with_violation() {
        run_parametrized_test<int>(/* IllegalStateException::class */ std::nullopt, [](auto flow_factory) -> /* suspend */ void {
            flow_factory([](FlowCollector<int>& collector) -> /* suspend */ void {
                collector.emit(1);
            }).buffer(Dispatchers::Unconfined, flow_factory).collect([](int value) {
                expect(1);
            });
            finish(2);
        });
    }

    // @Test
    void test_merge_violation() {
        run_parametrized_test<int>(std::nullopt, [](auto flow_factory) -> /* suspend */ void {
            auto merge = [](Flow<int>& self, Flow<int>& other) -> Flow<int> {
                return flow_factory([&](FlowCollector<int>& collector) -> /* suspend */ void {
                    coroutine_scope([&]() -> /* suspend */ void {
                        launch([&]() -> /* suspend */ void {
                            self.collect([&](int value) -> /* suspend */ void {
                                collector.emit(value);
                            });
                        });
                        other.collect([&](int value) -> /* suspend */ void {
                            collector.emit(value);
                        });
                    });
                });
            };

            auto tricky_merge = [](Flow<int>& self, Flow<int>& other) -> Flow<int> {
                return flow_factory([&](FlowCollector<int>& collector) -> /* suspend */ void {
                    coroutine_scope([&]() -> /* suspend */ void {
                        launch([&]() -> /* suspend */ void {
                            self.collect([&](int value) -> /* suspend */ void {
                                coroutine_scope([&]() -> /* suspend */ void {
                                    collector.emit(value);
                                });
                            });
                        });
                        other.collect([&](int value) -> /* suspend */ void {
                            collector.emit(value);
                        });
                    });
                });
            };

            auto flow_instance = flow_of(1);
            assert_fails_with<IllegalStateException>([&]() { merge(flow_instance, flow_instance).to_list(); });
            assert_fails_with<IllegalStateException>([&]() { tricky_merge(flow_instance, flow_instance).to_list(); });
        });
    }

    // @Test
    void test_no_merge_violation() {
        run_test([]() -> /* suspend */ void {
            auto merge = [](Flow<int>& self, Flow<int>& other) -> Flow<int> {
                return channel_flow([&](auto& channel_scope) -> /* suspend */ void {
                    channel_scope.launch([&]() -> /* suspend */ void {
                        self.collect([&](int value) -> /* suspend */ void {
                            channel_scope.send(value);
                        });
                    });
                    other.collect([&](int value) -> /* suspend */ void {
                        channel_scope.send(value);
                    });
                });
            };

            auto tricky_merge = [](Flow<int>& self, Flow<int>& other) -> Flow<int> {
                return channel_flow([&](auto& channel_scope) -> /* suspend */ void {
                    coroutine_scope([&]() -> /* suspend */ void {
                        launch([&]() -> /* suspend */ void {
                            self.collect([&](int value) -> /* suspend */ void {
                                coroutine_scope([&]() -> /* suspend */ void {
                                    channel_scope.send(value);
                                });
                            });
                        });
                        other.collect([&](int value) -> /* suspend */ void {
                            channel_scope.send(value);
                        });
                    });
                });
            };

            auto flow_instance = flow_of(1);
            assert_equals(std::vector<int>{1, 1}, merge(flow_instance, flow_instance).to_list());
            assert_equals(std::vector<int>{1, 1}, tricky_merge(flow_instance, flow_instance).to_list());
        });
    }

    // @Test
    void test_scoped_coroutine_no_violation() {
        run_parametrized_test<int>(std::nullopt, [](auto flow_factory) -> /* suspend */ void {
            auto buffer_fn = [&](Flow<int>& self) -> Flow<int> {
                return flow_factory([&](FlowCollector<int>& collector) -> /* suspend */ void {
                    coroutine_scope([&]() -> /* suspend */ void {
                        auto channel = produce([&](auto& producer) -> /* suspend */ void {
                            self.collect([&](int value) -> /* suspend */ void {
                                producer.send(value);
                            });
                        });
                        channel.consume_each([&](int value) -> /* suspend */ void {
                            collector.emit(value);
                        });
                    });
                });
            };
            auto flow_instance = flow_of(1, 1);
            assert_equals(std::vector<int>{1, 1}, buffer_fn(flow_instance).to_list());
        });
    }

private:
    Flow<int> buffer(Flow<int>& self, CoroutineContext& coroutine_context,
                     std::function<Flow<int>(std::function<void(FlowCollector<int>&)>)> flow_factory) {
        return flow_factory([&](FlowCollector<int>& collector) -> /* suspend */ void {
            coroutine_scope([&]() -> /* suspend */ void {
                Channel<int> channel;
                launch([&]() -> /* suspend */ void {
                    self.collect([&](int value) -> /* suspend */ void {
                        channel.send(value);
                    });
                    channel.close();
                });

                launch(coroutine_context, [&]() -> /* suspend */ void {
                    for (int i : channel) {
                        collector.emit(i);
                    }
                });
            });
        });
    }

public:
    // @Test
    void test_empty_coroutine_context_map() {
        run_test([]() -> /* suspend */ void {
            empty_context_test([](Flow<int>& flow) -> Flow<int> {
                return flow.map([](int it) {
                    expect(it);
                    return it + 1;
                });
            });
        });
    }

    // @Test
    void test_empty_coroutine_context_transform() {
        run_test([]() -> /* suspend */ void {
            empty_context_test([](Flow<int>& flow) -> Flow<int> {
                return flow.transform([](int it, FlowCollector<int>& collector) -> /* suspend */ void {
                    expect(it);
                    collector.emit(it + 1);
                });
            });
        });
    }

    // @Test
    void test_empty_coroutine_context_transform_while() {
        run_test([]() -> /* suspend */ void {
            empty_context_test([](Flow<int>& flow) -> Flow<int> {
                return flow.transform_while([](int it, FlowCollector<int>& collector) -> /* suspend */ bool {
                    expect(it);
                    collector.emit(it + 1);
                    return true;
                });
            });
        });
    }

    // @Test
    void test_empty_coroutine_context_violation_transform() {
        run_test([]() -> /* suspend */ void {
            try {
                empty_context_test([](Flow<int>& flow) -> Flow<int> {
                    return flow.transform([](int it, FlowCollector<int>& collector) -> /* suspend */ void {
                        expect(it);
                        with_context(Dispatchers::Unconfined, [&]() -> /* suspend */ void {
                            collector.emit(it + 1);
                        });
                    });
                });
                expect_unreached();
            } catch (const IllegalStateException& e) {
                assert_true(std::string(e.what()).find("Flow invariant is violated") != std::string::npos);
                finish(2);
            }
        });
    }

    // @Test
    void test_empty_coroutine_context_violation_transform_while() {
        run_test([]() -> /* suspend */ void {
            try {
                empty_context_test([](Flow<int>& flow) -> Flow<int> {
                    return flow.transform_while([](int it, FlowCollector<int>& collector) -> /* suspend */ bool {
                        expect(it);
                        with_context(Dispatchers::Unconfined, [&]() -> /* suspend */ void {
                            collector.emit(it + 1);
                        });
                        return true;
                    });
                });
                expect_unreached();
            } catch (const IllegalStateException& e) {
                assert_true(std::string(e.what()).find("Flow invariant is violated") != std::string::npos);
                finish(2);
            }
        });
    }

private:
    void empty_context_test(std::function<Flow<int>(Flow<int>&)> block) /* suspend */ {
        auto collector = [&]() -> /* suspend */ int {
            int result = -1;
            channel_flow([](auto& scope) -> /* suspend */ void {
                scope.send(1);
            }).block()
                .collect([&](int it) -> /* suspend */ void {
                    expect(it);
                    result = it;
                });
            return result;
        };

        int result = with_empty_context([&]() { return collector(); });
        assert_equals(2, result);
        finish(3);
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
