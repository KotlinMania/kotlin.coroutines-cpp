// Original file: kotlinx-coroutines-core/common/test/flow/operators/FlattenConcatTest.kt
//
// TODO: Mechanical C++ transliteration - Requires comprehensive updates:
// - Import test framework headers
// - Extend FlatMapBaseTest
// - Map flattenConcat operator

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlin.test.*

            class FlattenConcatTest : public FlatMapBaseTest {
            public:
                Flow<int> flat_map(Flow<int> flow_var, auto mapper) override {
                    return flow_var.map(mapper).flatten_concat();
                }

                // TODO: @Test
                void testFlatMapConcurrency() {
                    // TODO: runTest {
                    int concurrent_requests = 0;
                    auto flow_var = as_flow(1, 100).map([&](int value) {
                        return flow([&, value](auto &emit) {
                            ++concurrent_requests;
                            emit(value);
                            delay(LONG_MAX);
                        });
                    }).flatten_concat();

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

                // TODO: @Test
                void testCancellation() {
                    // TODO: runTest {
                    auto flow_var = flow([](auto &emit) {
                        for (int i = 0; i < 5; ++i) {
                            emit(flow([i](auto &emit_inner) {
                                if (i == 2) throw CancellationException("");
                                emit_inner(1);
                            }));
                        }
                    });
                    assertFailsWith<CancellationException>([&]() { flow_var.flatten_concat(); });
                    // TODO: }
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx