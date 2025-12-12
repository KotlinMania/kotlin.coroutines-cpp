// Original file: kotlinx-coroutines-core/common/test/flow/operators/MergeTest.kt
// TODO: handle imports (kotlinx.coroutines.testing, kotlinx.coroutines, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle Flow types and operations
// TODO: translate 'import kotlinx.coroutines.flow.merge as originalMerge'

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            template<typename T>
            class MergeTest : public TestBase {
            public:
                virtual Flow<T> merge(const std::vector<Flow<T> > &flows) = 0;

                // @Test
                void test_merge() /* TODO: = runTest */ {
                    int n = 100;
                    std::vector<Flow<int> > flows;
                    for (int i = 1; i <= n; ++i) {
                        flows.push_back(flow_of(i));
                    }
                    auto sum = merge(flows).sum();

                    assert_equals(n * (n + 1) / 2, sum);
                }

                // @Test
                void test_single() /* TODO: = runTest */ {
                    auto flow = merge({flow_of<int>(), flow_of(42), flow_of<int>()});
                    auto value = flow.single();
                    assert_equals(42, value);
                }

                // @Test
                void test_nulls() /* TODO: = runTest */ {
                    auto list = merge({flow_of(1), flow_of(nullptr), flow_of(2)}).to_list();
                    assert_equals(std::vector<int *>{1, nullptr, 2}, list); // TODO: nullable handling
                }

                // @Test
                void test_context() /* TODO: = runTest */ {
                    auto flow = flow([]() /* TODO: suspend */ {
                        emit(NamedDispatchers::name());
                    }).flow_on(NamedDispatchers("source"));

                    auto result = merge({flow}).flow_on(NamedDispatchers("irrelevant")).to_list();
                    assert_equals(std::vector<std::string>{"source"}, result);
                }

                // @Test
                void test_one_source_cancelled() /* TODO: = runTest */ {
                    auto flow = flow([]() /* TODO: suspend */ {
                        expect(1);
                        emit(1);
                        expect(2);
                        yield();
                        throw CancellationException("");
                    });

                    auto other_flow = flow([]() /* TODO: suspend */ {
                        for (int i = 0; i < 5; ++i) {
                            emit(1);
                            yield();
                        }

                        expect(3);
                    });

                    auto result = merge({flow, other_flow}).to_list();
                    assert_equals(std::vector<int>(6, 1), result);
                    finish(4);
                }

                // @Test
                void test_one_source_cancelled_non_fused() /* TODO: = runTest */ {
                    auto flow = flow([]() /* TODO: suspend */ {
                        expect(1);
                        emit(1);
                        expect(2);
                        yield();
                        throw CancellationException("");
                    });

                    auto other_flow = flow([]() /* TODO: suspend */ {
                        for (int i = 0; i < 5; ++i) {
                            emit(1);
                            yield();
                        }

                        expect(3);
                    });

                    auto result = non_fuseable_merge({flow, other_flow}).to_list();
                    assert_equals(std::vector<int>(6, 1), result);
                    finish(4);
                }

                // @Test
                void test_isolated_context() /* TODO: = runTest */ {
                    auto flow = flow([]() /* TODO: suspend */ {
                        emit(NamedDispatchers::name());
                    });

                    auto result = merge({flow.flow_on(NamedDispatchers("1")), flow.flow_on(NamedDispatchers("2"))})
                            .flow_on(NamedDispatchers("irrelevant"))
                            .to_list();
                    assert_equals(std::vector<std::string>{"1", "2"}, result);
                }

            private:
                template<typename T>
                Flow<T> non_fuseable_merge(const std::vector<Flow<T> > &flows) {
                    return channel_flow([flows](auto &channel) /* TODO: suspend */ {
                        for (auto &flow: flows) {
                            launch([&]() /* TODO: suspend */ {
                                flow.collect([&](T it) { channel.send(it); });
                            });
                        }
                    });
                }
            };

            class IterableMergeTest : public MergeTest<int> {
            public:
                Flow<int> merge(const std::vector<Flow<int> > &flows) override {
                    return original_merge(flows); // TODO: function reference
                }
            };

            class VarargMergeTest : public MergeTest<int> {
            public:
                Flow<int> merge(const std::vector<Flow<int> > &flows) override {
                    // TODO: translate *toList().toTypedArray() (vararg expansion)
                    return original_merge(flows);
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx