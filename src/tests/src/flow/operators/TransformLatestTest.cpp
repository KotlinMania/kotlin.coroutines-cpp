// Original file: kotlinx-coroutines-core/common/test/flow/operators/TransformLatestTest.kt
// TODO: handle imports (kotlinx.coroutines.testing, kotlinx.coroutines, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle Flow types and operations

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            class TransformLatestTest : public TestBase {
            public:
                // @Test
                void test_transform_latest() /* TODO: = runTest */ {
                    auto flow = flow_of(1, 2, 3).transform_latest([](int value) /* TODO: suspend */ {
                        emit(value);
                        emit(value + 1);
                    });
                    assert_equals(std::vector<int>{1, 2, 2, 3, 3, 4}, flow.to_list());
                }

                // @Test
                void test_emission() /* TODO: = runTest */ {
                    auto list = flow([]() /* TODO: suspend */ {
                        for (int i = 0; i < 5; ++i) {
                            emit(i);
                        }
                    }).transform_latest([](int it) /* TODO: suspend */ {
                        emit(it);
                    }).to_list();
                    assert_equals(std::vector<int>{0, 1, 2, 3, 4}, list);
                }

                // @Test
                void test_switch_intuitive_behaviour() /* TODO: = runTest */ {
                    auto flow = flow_of(1, 2, 3, 4, 5);
                    flow.transform_latest([](int it) /* TODO: suspend */ {
                        expect(it);
                        emit(it);
                        yield(); // Explicit cancellation check
                        if (it != 5) expect_unreached();
                        else expect(6);
                    }).collect();
                    finish(7);
                }

                // @Test
                void test_switch_rendezvous_buffer() /* TODO: = runTest */ {
                    auto flow = flow_of(1, 2, 3, 4, 5);
                    flow.transform_latest([](int it) /* TODO: suspend */ {
                        emit(it);
                        // Reach here every uneven element because of channel's unfairness
                        expect(it);
                    }).buffer(0).on_each([](int it) { expect(it + 1); }).collect();
                    finish(7);
                }

                // @Test
                void test_switch_buffer() /* TODO: = runTest */ {
                    auto flow = flow_of(1, 2, 3, 42, 4);
                    flow.transform_latest([](int it) /* TODO: suspend */ {
                        emit(it);
                        expect(it);
                    }).buffer(2).collect();
                    finish(5);
                }

                // @Test
                void test_hang_flows() /* TODO: = runTest */ {
                    auto flow = std::vector<int>{1, 2, 3, 4}.as_flow();
                    auto result = flow.transform_latest([](int value) /* TODO: suspend */ {
                        if (value != 4) hang([value]() { expect(value); });
                        emit(42);
                    }).to_list();

                    assert_equals(std::vector<int>{42}, result);
                    finish(4);
                }

                // @Test
                void test_empty_flow() /* TODO: = runTest */ {
                    assert_null(
                        empty_flow<int>().transform_latest([](int) /* TODO: suspend */ { emit(1); }).single_or_null());
                }

                // @Test
                void test_isolated_context() /* TODO: = runTest */ {
                    auto flow = flow([]() /* TODO: suspend */ {
                        assert_equals("source", NamedDispatchers::name());
                        expect(1);
                        emit(4);
                        expect(2);
                        emit(5);
                        expect(3);
                    }).flow_on(NamedDispatchers("source")).transform_latest<int, int>(
                        [](int value) /* TODO: suspend */ {
                            emit_all(flow < int > ([value]() /* TODO: suspend */ {
                                assert_equals("switch" + std::to_string(value), NamedDispatchers::name());
                                expect(value);
                                emit(value);
                            }).flow_on(NamedDispatchers("switch" + std::to_string(value))));
                        }).on_each([](int it) {
                        expect(it + 2);
                        assert_equals("main", NamedDispatchers::name_or("main"));
                    });
                    assert_equals(2, flow.count());
                    finish(8);
                }

                // @Test
                void test_failure_in_transform() /* TODO: = runTest */ {
                    auto flow = flow_of(1, 2).transform_latest([](int value) /* TODO: suspend */ {
                        if (value == 1) {
                            emit(1);
                            hang([]() { expect(1); });
                        } else {
                            expect(2);
                            throw TestException();
                        }
                    });
                    assert_fails_with<TestException>(flow);
                    finish(3);
                }

                // @Test
                void test_failure_downstream() /* TODO: = runTest */ {
                    auto flow = flow_of(1).transform_latest([](int value) /* TODO: suspend */ {
                        expect(1);
                        emit(value);
                        expect(2);
                        hang([]() { expect(4); });
                    }).flow_on(NamedDispatchers("downstream")).on_each([](int) {
                        expect(3);
                        throw TestException();
                    });
                    assert_fails_with<TestException>(flow);
                    finish(5);
                }

                // @Test
                void test_failure_upstream() /* TODO: = runTest */ {
                    auto flow = flow([]() /* TODO: suspend */ {
                        expect(1);
                        emit(1);
                        yield();
                        expect(3);
                        throw TestException();
                    }).transform_latest<int, int64_t>([](int) /* TODO: suspend */ {
                        expect(2);
                        hang([]() {
                            expect(4);
                        });
                    });
                    assert_fails_with<TestException>(flow);
                    finish(5);
                }

                // @Test
                void test_take() /* TODO: = runTest */ {
                    auto flow = flow_of(1, 2, 3, 4, 5).transform_latest([](int it) /* TODO: suspend */ { emit(it); });
                    assert_equals(std::vector<int>{1}, flow.take(1).to_list());
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx