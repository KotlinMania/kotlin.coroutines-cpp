// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/channels/SendReceiveStressTest.kt
//
// TODO: Translate imports
// TODO: Translate suspend functions to C++ coroutines
// TODO: Translate test annotations to C++ test framework

namespace kotlinx {
namespace coroutines {
namespace channels {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlin.test.*

class SendReceiveStressTest : public TestBase {
public:

    // Emulate parametrized by hand :(

    // TODO: @Test
    void testBufferedChannel() /* = runTest */ {
        // TODO: testStress(Channel(2));
    }

    // TODO: @Test
    void testUnlimitedChannel() /* = runTest */ {
        // TODO: testStress(Channel(Channel::UNLIMITED));
    }

    // TODO: @Test
    void testRendezvousChannel() /* = runTest */ {
        // TODO: testStress(Channel(Channel::RENDEZVOUS));
    }

private:
    /* suspend */ void testStress(/* Channel<int> channel */) /* = coroutineScope */ {
        int n = 100; // Do not increase, otherwise node.js will fail with timeout :(
        // TODO: auto sender = launch([&]() {
        //     for (int i = 1; i <= n; i++) {
        //         channel.send(i);
        //     }
        //     expect(2);
        // });
        // TODO: auto receiver = launch([&]() {
        //     for (int i = 1; i <= n; i++) {
        //         auto next = channel.receive();
        //         check(next == i);
        //     }
        //     expect(3);
        // });
        expect(1);
        // TODO: sender.join();
        // TODO: receiver.join();
        finish(4);
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
