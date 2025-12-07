// Original: kotlinx-coroutines-core/common/test/flow/IdFlowTest.kt
// @file:Suppress("NAMED_ARGUMENTS_NOT_ALLOWED") // KT-21913
// TODO: Translate imports to proper C++ includes
// TODO: Implement TestBase base class
// TODO: Implement @Test annotation equivalent
// TODO: Implement runTest with expected exception handler
// TODO: Implement Flow, flow builder, collect
// TODO: Implement CancellationException
// TODO: Implement coroutineContext.cancel()
// TODO: Implement hang utility
// TODO: Implement coroutineScope, produce, consumeEach

// See https://github.com/Kotlin/kotlinx.coroutines/issues/1128

namespace kotlinx {
namespace coroutines {
namespace flow {

class IdFlowTest : public TestBase {
public:
    // @Test
    void test_cancel_in_collect() {
        // runTest with expected exception predicate
        run_test(
            [](const std::exception_ptr& e) -> bool {
                // expected = { it is CancellationException }
                try {
                    if (e) std::rethrow_exception(e);
                    return false;
                } catch (const CancellationException&) {
                    return true;
                } catch (...) {
                    return false;
                }
            },
            []() -> /* suspend */ void {
                expect(1);
                flow([](FlowCollector<int>& collector) -> /* suspend */ void {
                    expect(2);
                    collector.emit(1);
                    expect(3);
                    hang([](){ finish(6); });
                }).id_scoped().collect([](int value) -> /* suspend */ void {
                    expect(4);
                    assert_equals(1, value);
                    kotlin::coroutines::coroutine_context.cancel();
                    expect(5);
                });
                expect_unreached();
            }
        );
    }

    // @Test
    void test_cancel_in_flow() {
        // runTest with expected exception predicate
        run_test(
            [](const std::exception_ptr& e) -> bool {
                // expected = { it is CancellationException }
                try {
                    if (e) std::rethrow_exception(e);
                    return false;
                } catch (const CancellationException&) {
                    return true;
                } catch (...) {
                    return false;
                }
            },
            []() -> /* suspend */ void {
                expect(1);
                flow([](FlowCollector<int>& collector) -> /* suspend */ void {
                    expect(2);
                    collector.emit(1);
                    kotlin::coroutines::coroutine_context.cancel();
                    expect(3);
                }).id_scoped().collect([](int value) -> /* suspend */ void {
                    finish(4);
                    assert_equals(1, value);
                });
                expect_unreached();
            }
        );
    }
};

/**
 * This flow should be "identity" function with respect to cancellation.
 */
template<typename T>
Flow<T> id_scoped(Flow<T>& self) {
    return flow([&](FlowCollector<T>& collector) -> /* suspend */ void {
        coroutine_scope([&]() -> /* suspend */ void {
            auto channel = produce([&](auto& producer) -> /* suspend */ void {
                self.collect([&](const T& value) -> /* suspend */ void {
                    producer.send(value);
                });
            });
            channel.consume_each([&](const T& value) -> /* suspend */ void {
                collector.emit(value);
            });
        });
    });
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
