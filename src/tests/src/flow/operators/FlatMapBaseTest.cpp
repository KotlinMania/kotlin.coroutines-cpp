// Original file: kotlinx-coroutines-core/common/test/flow/operators/FlatMapBaseTest.kt
//
// TODO: Mechanical C++ transliteration - Requires comprehensive updates:
// - Import test framework headers
// - Map abstract class with pure virtual flatMap method
// - Map NamedDispatchers to C++ equivalent

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlin.test.*

            class FlatMapBaseTest : public TestBase {
            public:
                virtual Flow<int> flat_map(Flow<int> flow_var, auto mapper) = 0;

                // TODO: @Test
                void testFlatMap() {
                    // TODO: runTest {
                    int n = 100;
                    int sum = as_flow(1, 100)
                            .flat_map([](int value) {
                                // 1 + (1 + 2) + (1 + 2 + 3) + ... (1 + .. + n)
                                return flow([value](auto &emit) {
                                    for (int i = 0; i < value; ++i) {
                                        emit(i + 1);
                                    }
                                });
                            }).sum();

                    assertEquals(n * (n + 1) * (n + 2) / 6, sum);
                    // TODO: }
                }

                // TODO: @Test
                void testSingle() {
                    // TODO: runTest {
                    auto flow_var = flow([](auto &emit) {
                        for (int i = 0; i < 100; ++i) {
                            emit(i);
                        }
                    }).flat_map([](int value) {
                        if (value == 99) return flow_of(42);
                        else return flow_of<int>();
                    });

                    int value = flow_var.single();
                    assertEquals(42, value);
                    // TODO: }
                }

                // TODO: @Test
                void testNulls() {
                    // TODO: runTest {
                    auto list = flow_of<std::optional<int> >(1, std::nullopt, 2).flat_map([](auto it) {
                        return flow_of<std::optional<int> >(1, std::nullopt, std::nullopt, 2);
                    }).to_list();

                    std::vector<std::optional<int> > expected;
                    for (int i = 0; i < 3; ++i) {
                        expected.push_back(1);
                        expected.push_back(std::nullopt);
                        expected.push_back(std::nullopt);
                        expected.push_back(2);
                    }
                    assertEquals(expected, list);
                    // TODO: }
                }

                // TODO: @Test
                void testContext() {
                    // TODO: runTest {
                    std::vector<std::string> captured;
                    auto flow_var = flow_of(1)
                            .flow_on(NamedDispatchers("irrelevant"))
                            .flat_map([&](auto it) {
                                captured.push_back(NamedDispatchers::name());
                                return flow([&](auto &emit) {
                                    captured.push_back(NamedDispatchers::name());
                                    emit(it);
                                });
                            });

                    flow_var.flow_on(NamedDispatchers("1")).sum();
                    flow_var.flow_on(NamedDispatchers("2")).sum();
                    assertEquals(std::vector<std::string>{"1", "1", "2", "2"}, captured);
                    // TODO: }
                }

                // TODO: @Test
                void testIsolatedContext() {
                    // TODO: runTest {
                    auto flow_var = flow_of(1)
                            .flow_on(NamedDispatchers("irrelevant"))
                            .flat_map([](auto it) {
                                return flow([it](auto &emit) {
                                    assertEquals("inner", NamedDispatchers::name());
                                    emit(it);
                                });
                            }).flow_on(NamedDispatchers("inner"))
                            .flat_map([](auto it) {
                                return flow([it](auto &emit) {
                                    assertEquals("outer", NamedDispatchers::name());
                                    emit(it);
                                });
                            }).flow_on(NamedDispatchers("outer"));

                    assertEquals(1, flow_var.single_or_null());
                    // TODO: }
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx