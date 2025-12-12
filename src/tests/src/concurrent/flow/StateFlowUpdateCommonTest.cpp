// Original: kotlinx-coroutines-core/concurrent/test/flow/StateFlowUpdateCommonTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement MutableStateFlow with update, updateAndGet, getAndUpdate

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlinx.coroutines.flow.*
            // TODO: import kotlin.test.*

            // A simplified version of StateFlowUpdateStressTest
            class StateFlowUpdateCommonTest : public TestBase {
            private:
                int iterations = 100'000 * stress_test_multiplier;

            public:
                // @Test
                // TODO: Convert test annotation
                void test_update() {
                    do_test([](MutableStateFlow<int> &flow) {
                        flow.update([](int it) { return it + 1; });
                    });
                }

                // @Test
                // TODO: Convert test annotation
                void test_update_and_get() {
                    do_test([](MutableStateFlow<int> &flow) {
                        flow.update_and_get([](int it) { return it + 1; });
                    });
                }

                // @Test
                // TODO: Convert test annotation
                void test_get_and_update() {
                    do_test([](MutableStateFlow<int> &flow) {
                        flow.get_and_update([](int it) { return it + 1; });
                    });
                }

            private:
                template<typename IncrementFunc>
                void do_test(IncrementFunc increment) {
                    runTest([&]() {
                        // TODO: suspend function
                        MutableStateFlow<int> flow{0};
                        auto j1 = launch(Dispatchers::Default, [&]() {
                            // TODO: suspend function
                            for (int i = 0; i < iterations / 2; ++i) {
                                increment(flow);
                            }
                        });

                        for (int i = 0; i < iterations / 2; ++i) {
                            increment(flow);
                        }

                        join_all(j1);
                        assertEquals(iterations, flow.value);
                    });
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx