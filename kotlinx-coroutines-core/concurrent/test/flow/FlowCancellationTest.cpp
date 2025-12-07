// Original: kotlinx-coroutines-core/concurrent/test/flow/FlowCancellationTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement flow, launchIn, currentCoroutineContext
// TODO: Implement withEmptyContext

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.channels.*
// TODO: import kotlinx.coroutines.flow.*
// TODO: import kotlin.test.*

class FlowCancellationTest : public TestBase {
public:
    // @Test
    // TODO: Convert test annotation
    void test_emit_is_cooperative() {
        runTest([&]() {
            // TODO: suspend function
            auto latch = Channel<void>(1);
            auto job = flow([&](auto& emitter) {
                // TODO: suspend function
                expect(1);
                latch.send(Unit);
                while (true) {
                    emitter.emit(42);
                }
            }).launch_in(*this + Dispatchers::Default);

            latch.receive();
            expect(2);
            job.cancel_and_join();
            finish(3);
        });
    }

    // @Test
    // TODO: Convert test annotation
    void test_is_active_on_current_context() {
        runTest([&]() {
            // TODO: suspend function
            auto latch = Channel<void>(1);
            auto job = flow<void>([&](auto& emitter) {
                // TODO: suspend function
                expect(1);
                latch.send(Unit);
                while (current_coroutine_context().is_active()) {
                    // Do nothing
                }
            }).launch_in(*this + Dispatchers::Default);

            latch.receive();
            expect(2);
            job.cancel_and_join();
            finish(3);
        });
    }

    // @Test
    // TODO: Convert test annotation
    void test_flow_with_empty_context() {
        runTest([&]() {
            // TODO: suspend function
            expect(1);
            with_empty_context([&]() {
                // TODO: suspend function
                auto flow_obj = flow([&](auto& emitter) {
                    // TODO: suspend function
                    expect(2);
                    emitter.emit("OK");
                });
                flow_obj.collect([&](const std::string& it) {
                    expect(3);
                    assertEquals("OK", it);
                });
            });
            finish(4);
        });
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
