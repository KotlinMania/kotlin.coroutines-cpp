// Original file: kotlinx-coroutines-core/common/test/flow/operators/DistinctUntilChangedTest.kt
//
// TODO: Mechanical C++ transliteration - Requires comprehensive updates:
// - Import test framework headers
// - Map Flow operators to C++ equivalents
// - Map distinctUntilChanged variants
// - Handle private class Box
// - Map member reference Box::i to C++ equivalent

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlin.test.*

            class DistinctUntilChangedTest : public TestBase {
            private:
                class Box {
                public:
                    int i;

                    explicit Box(int i) : i(i) {
                    }
                };

            public:
                // TODO: @Test
                void testDistinctUntilChanged() {
                    // TODO: runTest {
                    auto flow_var = flow_of(1, 1, 2, 2, 1).distinct_until_changed();
                    assertEquals(4, flow_var.sum());
                    // TODO: }
                }

                // TODO: @Test
                void testDistinctUntilChangedKeySelector() {
                    // TODO: runTest {
                    auto flow_var = flow([](auto &emit) {
                        emit(Box(1));
                        emit(Box(1));
                        emit(Box(2));
                        emit(Box(1));
                    });

                    int sum1 = flow_var.distinct_until_changed().map([](auto it) { return it.i; }).sum();
                    int sum2 = flow_var.distinct_until_changed_by([](auto box) { return box.i; }).map([](auto it) {
                        return it.i;
                    }).sum();
                    assertEquals(5, sum1);
                    assertEquals(4, sum2);
                    // TODO: }
                }

                // TODO: @Test
                void testDistinctUntilChangedAreEquivalent() {
                    // TODO: runTest {
                    auto flow_var = flow([](auto &emit) {
                        emit(Box(1));
                        emit(Box(1));
                        emit(Box(2));
                        emit(Box(1));
                    });

                    int sum1 = flow_var.distinct_until_changed().map([](auto it) { return it.i; }).sum();
                    int sum2 = flow_var.distinct_until_changed([](auto old_val, auto new_val) {
                        return old_val.i == new_val.i;
                    }).map([](auto it) { return it.i; }).sum();
                    assertEquals(5, sum1);
                    assertEquals(4, sum2);
                    // TODO: }
                }

                // TODO: @Test
                void testDistinctUntilChangedAreEquivalentSingleValue() {
                    // TODO: runTest {
                    auto flow_var = flow_of(1);
                    auto values = flow_var.distinct_until_changed([](auto, auto) {
                        fail("Expected not to compare single value.");
                        return false;
                    }).to_list();
                    assertEquals(std::vector<int>{1}, values);
                    // TODO: }
                }

                // TODO: @Test
                void testThrowingKeySelector() {
                    // TODO: runTest {
                    auto flow_var = flow([](auto &emit) {
                        coroutine_scope([&]() {
                            launch(CoroutineStart::ATOMIC, [&]() {
                                hang([&]() { expect(3); });
                            });
                            expect(2);
                            emit(1);
                        });
                    }).distinct_until_changed_by([](auto) {
                        throw TestException();
                        return 0;
                    });

                    expect(1);
                    assertFailsWith<TestException>(flow_var);
                    finish(4);
                    // TODO: }
                }

                // TODO: @Test
                void testThrowingAreEquivalent() {
                    // TODO: runTest {
                    auto flow_var = flow([](auto &emit) {
                        coroutine_scope([&]() {
                            launch(CoroutineStart::ATOMIC, [&]() {
                                hang([&]() { expect(3); });
                            });
                            expect(2);
                            emit(1);
                            emit(2);
                        });
                    }).distinct_until_changed([](auto, auto) {
                        throw TestException();
                        return false;
                    });

                    expect(1);
                    assertFailsWith<TestException>(flow_var);
                    finish(4);
                    // TODO: }
                }

                // TODO: @Test
                void testDistinctUntilChangedNull() {
                    // TODO: runTest {
                    auto flow_var = flow_of<std::optional<int> >(std::nullopt, 1, std::nullopt, std::nullopt).
                            distinct_until_changed();
                    assertEquals(std::vector<std::optional<int> >{std::nullopt, 1, std::nullopt}, flow_var.to_list());
                    // TODO: }
                }

                // TODO: @Test
                void testRepeatedDistinctFusionDefault() {
                    test_repeated_distinct_fusion([](auto flow) {
                        return flow.distinct_until_changed();
                    });
                }

                // A separate variable is needed for K/N that does not optimize non-captured lambdas (yet)
            private:
                std::function<bool(int, int)> are_equivalent_test_fun = [](int old_val, int new_val) {
                    return old_val == new_val;
                };

            public:
                // TODO: @Test
                void testRepeatedDistinctFusionAreEquivalent() {
                    test_repeated_distinct_fusion([&](auto flow) {
                        return flow.distinct_until_changed(are_equivalent_test_fun);
                    });
                }

                // A separate variable is needed for K/N that does not optimize non-captured lambdas (yet)
            private:
                std::function<int(int)> key_selector_test_fun = [](int it) { return it % 2; };

            public:
                // TODO: @Test
                void testRepeatedDistinctFusionByKey() {
                    test_repeated_distinct_fusion([&](auto flow) {
                        return flow.distinct_until_changed_by(key_selector_test_fun);
                    });
                }

            private:
                void test_repeated_distinct_fusion(auto op) {
                    // TODO: runTest {
                    auto flow_var = as_flow(1, 10);
                    auto d1 = op(flow_var);
                    assertNotSame(flow_var, d1);
                    auto d2 = op(d1);
                    assertSame(d1, d2);
                    // TODO: }
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx