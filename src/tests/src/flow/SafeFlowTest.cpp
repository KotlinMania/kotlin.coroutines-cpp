// Original: kotlinx-coroutines-core/common/test/flow/SafeFlowTest.kt
// TODO: Translate imports to proper C++ includes
// TODO: Implement TestBase base class
// TODO: Implement @Test annotation equivalent
// TODO: Implement runTest coroutine runner
// TODO: Implement Flow, flow builder
// TODO: Implement FlowCollector and emit
// TODO: Implement onEach, yield, toList

#include <vector>
// TODO: #include proper headers

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            class SafeFlowTest : public TestBase {
            public:
                // @Test
                void test_emissions_from_different_state_machine() {
                    run_test([]() -> /* suspend */ void {
                        auto result = flow<int>([](FlowCollector<int> &collector) -> /* suspend */ void {
                            emit1(collector, 1);
                            emit2(collector, 2);
                        }).on_each([](int) -> /* suspend */ void {
                            yield();
                        }).to_list();

                        assert_equals(std::vector<int>{1, 2}, result);
                        finish(3);
                    });
                }

            private:
                static void emit1(FlowCollector<int> &collector, int expect_value) /* suspend */ {
                    collector.emit(expect_value);
                    expect(expect_value);
                }

                static void emit2(FlowCollector<int> &collector, int expect_value) /* suspend */ {
                    collector.emit(expect_value);
                    expect(expect_value);
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx