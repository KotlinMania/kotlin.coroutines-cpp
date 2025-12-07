// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/channels/UnlimitedChannelTest.kt
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

class UnlimitedChannelTest : public TestBase {
public:
    // TODO: @Test
    void testBasic() /* = runTest */ {
        // TODO: auto c = Channel<int>(Channel::UNLIMITED);
        // TODO: c.send(1);
        // TODO: assertTrue(c.trySend(2).isSuccess);
        // TODO: c.send(3);
        // TODO: check(c.close());
        // TODO: check(!c.close());
        // TODO: assertEquals(1, c.receive());
        // TODO: assertEquals(2, c.tryReceive().getOrNull());
        // TODO: assertEquals(3, c.receiveCatching().getOrNull());
        // TODO: assertNull(c.receiveCatching().getOrNull());
    }

    // TODO: @Test
    void testConsumeAll() /* = runTest */ {
        // TODO: auto q = Channel<int>(Channel::UNLIMITED);
        // TODO: for (int i = 1; i <= 10; i++) {
        //     q.send(i); // buffers
        // }
        // TODO: q.cancel();
        // TODO: check(q.isClosedForSend);
        // TODO: check(q.isClosedForReceive);
        // TODO: assertFailsWith<CancellationException>([&]() { q.receive(); });
    }

    // TODO: @Test
    void testCancelWithCause() /* = runTest({ it is TestCancellationException }) */ {
        // TODO: auto channel = Channel<int>(Channel::UNLIMITED);
        // TODO: channel.cancel(TestCancellationException());
        // TODO: channel.receive();
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
