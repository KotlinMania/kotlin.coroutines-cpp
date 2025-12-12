// Original file: kotlinx-coroutines-core/common/test/flow/operators/FlatMapConcatTest.kt
//
// TODO: Mechanical C++ transliteration - Requires comprehensive updates:
// - Import test framework headers
// - Extend FlatMapBaseTest
// - Map flatMapConcat operator

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlin.test.*

            class FlatMapConcatTest : public FlatMapBaseTest {
            public:
                Flow<int> flat_map(Flow<int> flow_var, auto mapper) override {
                    return flow_var.flat_map_concat(mapper);
                }

                // TODO: @Test
                void testFlatMapConcurrency() {
                    // TODO: runTest {
                    int concurrent_requests = 0;
                    auto flow_var = as_flow(1, 100).flat_map_concat([&](int value) {
                        return flow([&, value](auto &emit) {
                            ++concurrent_requests;
                            emit(value);
                            delay(LONG_MAX);
                        });
                    });

                    auto consumer = launch([&]() {
                        flow_var.collect([&](int value) {
                            expect(value);
                        });
                    });

                    for (int i = 0; i < 4; ++i) {
                        yield();
                    }

                    assertEquals(1, concurrent_requests);
                    consumer.cancel_and_join();
                    finish(2);
                    // TODO: }
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx