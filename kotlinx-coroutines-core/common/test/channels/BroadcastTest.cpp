// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/channels/BroadcastTest.kt
//
// TODO: Translate file-level annotations (@file:Suppress)
// TODO: Translate imports
// TODO: Translate suspend functions to C++ coroutines
// TODO: Translate test annotations to C++ test framework

// TODO: @file:Suppress("DEPRECATION_ERROR")

namespace kotlinx {
namespace coroutines {
namespace channels {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.selects.*
// TODO: import kotlin.test.*

class BroadcastTest : public TestBase {
public:
    // TODO: @Test
    void testBroadcastBasic() /* = runTest */ {
        expect(1);
        // TODO: auto b = broadcast([&]() {
        //     expect(4);
        //     send(1); // goes to receiver
        //     expect(5);
        //     select<void>([&]() { onSend(2, []() {}); }); // goes to buffer
        //     expect(6);
        //     send(3); // suspends, will not be consumes, but will not be cancelled either
        //     expect(10);
        // });
        // TODO: yield(); // has no effect, because default is lazy
        expect(2);

        // TODO: auto subscription = b.openSubscription();
        expect(3);
        // TODO: assertEquals(1, subscription.receive()); // suspends
        expect(7);
        // TODO: assertEquals(2, subscription.receive()); // suspends
        expect(8);
        // TODO: subscription.cancel();
        expect(9);
        // TODO: yield(); // to broadcast
        finish(11);
    }

    /**
     * See https://github.com/Kotlin/kotlinx.coroutines/issues/1713
     */
    // TODO: @Test
    void testChannelBroadcastLazyCancel() /* = runTest */ {
        expect(1);
        // TODO: auto a = produce([&]() {
        //     expect(3);
        //     assertFailsWith<CancellationException>([&]() { send("MSG"); });
        //     expect(5);
        // });
        expect(2);
        // TODO: yield(); // to produce
        // TODO: auto b = a.broadcast();
        // TODO: b.cancel();
        expect(4);
        // TODO: yield(); // to abort produce
        // TODO: assertTrue(a.isClosedForReceive); // the source channel was consumed
        finish(6);
    }

    // TODO: @Test
    void testChannelBroadcastLazyClose() /* = runTest */ {
        expect(1);
        // TODO: auto a = produce([&]() {
        //     expect(3);
        //     send("MSG");
        //     expectUnreached(); // is not executed, because send is cancelled
        // });
        expect(2);
        // TODO: yield(); // to produce
        // TODO: auto b = a.broadcast();
        // TODO: b.close();
        expect(4);
        // TODO: yield(); // to abort produce
        // TODO: assertTrue(a.isClosedForReceive); // the source channel was consumed
        finish(5);
    }

    // TODO: @Test
    void testChannelBroadcastEagerCancel() /* = runTest */ {
        expect(1);
        // TODO: auto a = produce<void>([&]() {
        //     expect(3);
        //     yield(); // back to main
        //     expectUnreached(); // will be cancelled
        // });
        expect(2);
        // TODO: auto b = a.broadcast(CoroutineStart::DEFAULT);
        // TODO: yield(); // to produce
        expect(4);
        // TODO: b.cancel();
        // TODO: yield(); // to produce (cancelled)
        // TODO: assertTrue(a.isClosedForReceive); // the source channel was consumed
        finish(5);
    }

    // TODO: @Test
    void testChannelBroadcastEagerClose() /* = runTest */ {
        expect(1);
        // TODO: auto a = produce<void>([&]() {
        //     expect(3);
        //     yield(); // back to main
        //     // shall eventually get cancelled
        //     assertFailsWith<CancellationException>([&]() {
        //         while (true) { send(void); }
        //     });
        // });
        expect(2);
        // TODO: auto b = a.broadcast(CoroutineStart::DEFAULT);
        // TODO: yield(); // to produce
        expect(4);
        // TODO: b.close();
        // TODO: yield(); // to produce (closed)
        // TODO: assertTrue(a.isClosedForReceive); // the source channel was consumed
        finish(5);
    }

    // TODO: @Test
    void testBroadcastCloseWithException() /* = runTest */ {
        expect(1);
        // TODO: auto b = broadcast(NonCancellable, /* capacity = */ 1, [&]() {
        //     expect(2);
        //     send(1);
        //     expect(3);
        //     send(2); // suspends
        //     expect(5);
        //     // additional attempts to send fail
        //     assertFailsWith<TestException>([&]() { send(3); });
        // });
        // TODO: auto sub = b.openSubscription();
        // TODO: yield(); // into broadcast
        expect(4);
        // TODO: b.close(TestException()); // close broadcast channel with exception
        // TODO: assertTrue(b.isClosedForSend); // sub was also closed
        // TODO: assertEquals(1, sub.receive()); // 1st element received
        // TODO: assertEquals(2, sub.receive()); // 2nd element received
        // TODO: assertFailsWith<TestException>([&]() { sub.receive(); }); // then closed with exception
        // TODO: yield(); // to cancel broadcast
        finish(6);
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
