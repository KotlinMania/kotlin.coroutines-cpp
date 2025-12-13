// Original file: kotlinx-coroutines-core/common/test/flow/operators/DropTest.kt
//
// TODO: Mechanical C++ transliteration - Requires comprehensive updates:
// - Import test framework headers
// - Map drop() operator to C++ equivalent
// - Map assertFailsWith for negative count validation

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlin.test.*

            class DropTest : public TestBase {
            public:
                // TODO: @Test
                void testDrop() {
                    // TODO: runTest {
                    auto flow_var = flow([](auto &emit) {
                        emit(1);
                        emit(2);
                        emit(3);
                    });

                    assertEquals(5, flow_var.drop(1).sum());
                    assertEquals(0, flow_var.drop(INT_MAX).sum());
                    assertNull(flow_var.drop(INT_MAX).single_or_null());
                    assertEquals(3, flow_var.drop(1).take(2).drop(1).single());
                    // TODO: }
                }

                // TODO: @Test
                void testEmptyFlow() {
                    // TODO: runTest {
                    assertEquals(0, flow_of<int>().drop(1).sum());
                    // TODO: }
                }

                // TODO: @Test
                void testNegativeCount() {
                    assertFailsWith<IllegalArgumentException>([&]() {
                        empty_flow<int>().drop(-1);
                    });
                }

                // TODO: @Test
                void testErrorCancelsUpstream() {
                    // TODO: runTest {
                    auto flow_var = flow([](auto &emit) {
                                coroutine_scope([&]() {
                                    launch(CoroutineStart::ATOMIC, [&]() {
                                        hang([&]() { expect(5); });
                                    });
                                    expect(2);
                                    emit(1);
                                    expect(3);
                                    emit(2);
                                    expectUnreached();
                                });
                            }).drop(1)
                            .map<int, int>([](auto it) {
                                expect(4);
                                throw TestException();
                                return it;
                            }).catch_error([](auto &emit, auto e) { emit(42); });

                    expect(1);
                    assertEquals(42, flow_var.single());
                    finish(6);
                    // TODO: }
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx