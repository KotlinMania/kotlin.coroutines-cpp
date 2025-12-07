// Original: kotlinx-coroutines-core/concurrent/test/selects/SelectChannelStressTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement Channel, select, SelectBuilder
// TODO: Implement Channel.RENDEZVOUS constant

namespace kotlinx {
namespace coroutines {
namespace selects {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.channels.*
// TODO: import kotlin.test.*

class SelectChannelStressTest : public TestBase {
private:
    // Running less iterations on native platforms because of some performance regression
    int iterations = (is_native ? 1'000 : 1'000'000) * stress_test_multiplier;

public:
    // @Test
    // TODO: Convert test annotation
    void test_select_send_resource_cleanup_buffered_channel() {
        runTest([&]() {
            // TODO: suspend function
            auto channel = Channel<int>(1);
            expect(1);
            channel.send(-1); // fill the buffer, so all subsequent sends cannot proceed
            for (int i = 0; i < iterations; ++i) {
                select<void>([&](auto& builder) {
                    builder.on_send(channel, i, [&]() { expectUnreached(); });
                    builder.default_clause([&]() { expect(i + 2); });
                });
            }
            finish(iterations + 2);
        });
    }

    // @Test
    // TODO: Convert test annotation
    void test_select_receive_resource_cleanup_buffered_channel() {
        runTest([&]() {
            // TODO: suspend function
            auto channel = Channel<int>(1);
            expect(1);
            for (int i = 0; i < iterations; ++i) {
                select<void>([&](auto& builder) {
                    builder.on_receive(channel, [&](int) { expectUnreached(); });
                    builder.default_clause([&]() { expect(i + 2); });
                });
            }
            finish(iterations + 2);
        });
    }

    // @Test
    // TODO: Convert test annotation
    void test_select_send_resource_cleanup_rendezvous_channel() {
        runTest([&]() {
            // TODO: suspend function
            auto channel = Channel<int>(Channel::kRendezvous);
            expect(1);
            for (int i = 0; i < iterations; ++i) {
                select<void>([&](auto& builder) {
                    builder.on_send(channel, i, [&]() { expectUnreached(); });
                    builder.default_clause([&]() { expect(i + 2); });
                });
            }
            finish(iterations + 2);
        });
    }

    // @Test
    // TODO: Convert test annotation
    void test_select_receive_resource_rendezvous_channel() {
        runTest([&]() {
            // TODO: suspend function
            auto channel = Channel<int>(Channel::kRendezvous);
            expect(1);
            for (int i = 0; i < iterations; ++i) {
                select<void>([&](auto& builder) {
                    builder.on_receive(channel, [&](int) { expectUnreached(); });
                    builder.default_clause([&]() { expect(i + 2); });
                });
            }
            finish(iterations + 2);
        });
    }

    // internal fun <R> SelectBuilder<R>.default(block: suspend () -> R) = onTimeout(0, block)
    // TODO: Implement as extension or helper method
};

} // namespace selects
} // namespace coroutines
} // namespace kotlinx
