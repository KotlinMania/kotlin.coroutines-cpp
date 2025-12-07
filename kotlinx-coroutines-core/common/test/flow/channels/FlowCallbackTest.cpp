// Original: kotlinx-coroutines-core/common/test/flow/channels/FlowCallbackTest.kt
// @file:Suppress("NAMED_ARGUMENTS_NOT_ALLOWED") // KT-21913
// TODO: Translate imports to proper C++ includes
// TODO: Implement TestBase base class
// TODO: Implement @Test annotation equivalent
// TODO: Implement runTest coroutine runner
// TODO: Implement callbackFlow builder
// TODO: Implement CoroutineScope, Job
// TODO: Implement launch, send, close, awaitClose
// TODO: Implement toList
// TODO: Implement IllegalStateException
// TODO: Implement string contains checking

#include <vector>
#include <string>
#include <stdexcept>
// TODO: #include proper headers

namespace kotlinx {
namespace coroutines {
namespace flow {

class FlowCallbackTest : public TestBase {
public:
    // @Test
    void test_closed_prematurely() {
        run_test([]() -> /* suspend */ void {
            CoroutineScope& outer_scope = *this;
            auto flow_instance = callback_flow([&](auto& scope) -> /* suspend */ void {
                // ~ callback-based API
                outer_scope.launch(Job(), [&](auto& launch_scope) -> /* suspend */ void {
                    expect(2);
                    try {
                        scope.send(1);
                        expect_unreached();
                    } catch (const IllegalStateException& e) {
                        expect(3);
                        assert_true(std::string(e.what()).find("awaitClose") != std::string::npos);
                    }
                });
                expect(1);
            });

            try {
                flow_instance.collect();
            } catch (const IllegalStateException& e) {
                expect(4);
                assert_true(std::string(e.what()).find("awaitClose") != std::string::npos);
            }
            finish(5);
        });
    }

    // @Test
    void test_not_closed_prematurely() {
        run_test([]() -> /* suspend */ void {
            CoroutineScope& outer_scope = *this;
            auto flow_instance = callback_flow([&](auto& scope) -> /* suspend */ void {
                // ~ callback-based API
                outer_scope.launch(Job(), [&](auto& launch_scope) -> /* suspend */ void {
                    expect(2);
                    scope.send(1);
                    scope.close();
                });
                expect(1);
                scope.await_close();
            });

            assert_equals(std::vector<int>{1}, flow_instance.to_list());
            finish(3);
        });
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
