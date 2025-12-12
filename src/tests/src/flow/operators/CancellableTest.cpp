// Original file: kotlinx-coroutines-core/common/test/flow/operators/CancellableTest.kt
//
// TODO: Mechanical C++ transliteration - Requires comprehensive updates:
// - Import test framework headers
// - Implement suspend functions as regular functions
// - Map Flow operators to C++ equivalents
// - Map currentCoroutineContext() to C++ equivalent
// - Map assertNotSame, assertSame to C++ test assertions

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlin.test.*

            class CancellableTest : public TestBase {
            public:
                // TODO: @Test
                void testCancellable() {
                    // TODO: runTest {
                    int sum = 0;
                    auto flow_var = as_flow(0, 1000)
                            .on_each([&](int it) {
                                if (it != 0) current_coroutine_context().cancel();
                                sum += it;
                            });

                    flow_var.launch_in(/* this */).join();
                    assertEquals(500500, sum);

                    sum = 0;
                    flow_var.cancellable().launch_in(/* this */).join();
                    assertEquals(1, sum);
                    // TODO: }
                }

                // TODO: @Test
                void testFastPath() {
                    auto flow_var = as_flow(std::vector<int>{1});
                    assertNotSame(flow_var, flow_var.cancellable());

                    auto cancellable_flow = flow([](auto &emit) { emit(42); });
                    assertSame(cancellable_flow, cancellable_flow.cancellable());
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx