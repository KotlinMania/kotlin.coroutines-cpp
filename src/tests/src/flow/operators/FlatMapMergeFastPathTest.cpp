// Original file: kotlinx-coroutines-core/common/test/flow/operators/FlatMapMergeFastPathTest.kt
//
// TODO: Mechanical C++ transliteration - Requires comprehensive updates:
// - Import test framework headers
// - Extend FlatMapMergeBaseTest
// - Map flatMapMerge with buffer

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlin.test.*
            // TODO: import kotlin.test.assertFailsWith

            class FlatMapMergeFastPathTest : public FlatMapMergeBaseTest {
            public:
                Flow<int> flat_map(Flow<int> flow_var, auto mapper) override {
                    return flow_var.flat_map_merge(mapper).buffer(64);
                }

                // TODO: @Test
                void testFlatMapConcurrency() override {
                    // TODO: runTest {
                    int concurrent_requests = 0;
                    auto flow_var = as_flow(1, 100).flat_map_merge(2, [&](int value) {
                        return flow([&, value](auto &emit) {
                            ++concurrent_requests;
                            emit(value);
                            delay(LONG_MAX);
                        });
                    }).buffer(64);

                    auto consumer = launch([&]() {
                        flow_var.collect([&](int value) {
                            expect(value);
                        });
                    });

                    for (int i = 0; i < 4; ++i) {
                        yield();
                    }

                    assertEquals(2, concurrent_requests);
                    consumer.cancel_and_join();
                    finish(3);
                    // TODO: }
                }

                // TODO: @Test
                void testCancellationExceptionDownstream() {
                    // TODO: runTest {
                    auto flow_var = flow_of(1, 2, 3).flat_map_merge([](auto it) {
                        return flow([it](auto &emit) {
                            emit(it);
                            throw CancellationException("");
                        });
                    }).buffer(64);

                    assertEquals(std::vector<int>{1, 2, 3}, flow_var.to_list());
                    // TODO: }
                }

                // TODO: @Test
                void testCancellationExceptionUpstream() {
                    // TODO: runTest {
                    auto flow_var = flow([](auto &emit) {
                        expect(1);
                        emit(1);
                        expect(2);
                        yield();
                        throw CancellationException("");
                    }).flat_map_merge([](auto it) {
                        return flow([it](auto &emit) {
                            expect(3);
                            emit(it);
                            hang([&]() { expect(4); });
                        });
                    }).buffer(64);

                    assertFailsWith<CancellationException>(flow_var);
                    finish(5);
                    // TODO: }
                }

                // TODO: @Test
                void testCancellation() {
                    // TODO: runTest {
                    auto result = flow([](auto &emit) {
                                emit(1);
                                emit(2);
                                emit(3);
                                emit(4);
                                expectUnreached(); // Cancelled by take
                                emit(5);
                            }).flat_map_merge(2, [](auto v) { return flow([v](auto &emit) { emit(v); }); })
                            .buffer(64)
                            .take(2)
                            .to_list();
                    assertEquals(std::vector<int>{1, 2}, result);
                    // TODO: }
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx