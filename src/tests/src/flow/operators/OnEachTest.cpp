// Original file: kotlinx-coroutines-core/common/test/flow/operators/OnEachTest.kt
// TODO: handle imports (kotlinx.coroutines.testing, kotlinx.coroutines, kotlinx.coroutines.channels, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle Flow types and operations

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            class OnEachTest : public TestBase {
            public:
                // @Test
                void test_on_each() /* TODO: = runTest */ {
                    auto flow = flow([]() /* TODO: suspend */ {
                        emit(1);
                        emit(2);
                    });

                    auto result = flow.on_each([](int it) { expect(it); }).sum();
                    assert_equals(3, result);
                    finish(3);
                }

                // @Test
                void test_empty_flow() /* TODO: = runTest */ {
                    auto value = empty_flow<int>().on_each([](int) { fail(); }).single_or_null();
                    assert_null(value);
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
                    }).on_each([&](int) {
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