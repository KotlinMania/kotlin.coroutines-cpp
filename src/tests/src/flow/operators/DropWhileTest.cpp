// Original file: kotlinx-coroutines-core/common/test/flow/operators/DropWhileTest.kt
//
// TODO: Mechanical C++ transliteration - Requires comprehensive updates:
// - Import test framework headers
// - Map dropWhile() operator to C++ equivalent

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlin.test.*

            class DropWhileTest : public TestBase {
            public:
                // TODO: @Test
                void testDropWhile() {
                    // TODO: runTest {
                    auto flow_var = flow([](auto &emit) {
                        emit(1);
                        emit(2);
                        emit(3);
                    });

                    assertEquals(6, flow_var.drop_while([](auto) { return false; }).sum());
                    assertNull(flow_var.drop_while([](auto) { return true; }).single_or_null());
                    assertEquals(5, flow_var.drop_while([](auto it) { return it < 2; }).sum());
                    assertEquals(1, flow_var.take(1).drop_while([](auto it) { return it > 1; }).single());
                    // TODO: }
                }

                // TODO: @Test
                void testEmptyFlow() {
                    // TODO: runTest {
                    assertEquals(0, flow_of<int>().drop_while([](auto) { return true; }).sum());
                    assertEquals(0, flow_of<int>().drop_while([](auto) { return false; }).sum());
                    // TODO: }
                }

                // TODO: @Test
                void testErrorCancelsUpstream() {
                    // TODO: runTest {
                    auto flow_var = flow([](auto &emit) {
                        coroutine_scope([&]() {
                            launch(CoroutineStart::kAtomic, [&]() {
                                hang([&]() { expect(4); });
                            });
                            expect(2);
                            emit(1);
                            expectUnreached();
                        });
                    }).drop_while([](auto it) {
                        expect(3);
                        throw TestException();
                        return false;
                    });

                    expect(1);
                    assertFailsWith<TestException>(flow_var);
                    finish(5);
                    // TODO: }
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx