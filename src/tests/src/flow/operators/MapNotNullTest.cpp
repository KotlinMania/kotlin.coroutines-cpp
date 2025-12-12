// Original file: kotlinx-coroutines-core/common/test/flow/operators/MapNotNullTest.kt
// TODO: handle imports (kotlinx.coroutines.testing, kotlinx.coroutines, kotlinx.coroutines.channels, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle Flow types and operations
// TODO: handle nullable types

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            class MapNotNullTest : public TestBase {
            public:
                // @Test
                void test_map() /* TODO: = runTest */ {
                    auto flow = flow([]() /* TODO: suspend */ {
                        emit(1);
                        emit(nullptr); // TODO: nullable type
                        emit(2);
                    });

                    auto result = flow.map_not_null([](auto it) { return it; }).sum();
                    assert_equals(3, result);
                }

                // @Test
                void test_empty_flow() /* TODO: = runTest */ {
                    auto sum = empty_flow<int>().map_not_null([](int it) {
                        expect_unreached();
                        return it;
                    }).sum();
                    assert_equals(0, sum);
                }

                // @Test
                void test_error_cancels_upstream() /* TODO: = runTest */ {
                    bool cancelled = false;
                    Channel<Unit> latch;
                    auto flow = flow([&]() /* TODO: suspend */ {
                        coroutine_scope([&]() /* TODO: suspend */ {
                            launch([&]() /* TODO: suspend */ {
                                latch.send(Unit{});
                                hang([&]() { cancelled = true; });
                            });
                            emit(1);
                        });
                    }).map_not_null<int, int>([&](int) {
                        latch.receive();
                        throw TestException();
                    }).catch_error([](auto) { emit(42); });

                    assert_equals(42, flow.single());
                    assert_true(cancelled);
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx