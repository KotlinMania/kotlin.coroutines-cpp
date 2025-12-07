// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/channels/BufferedBroadcastChannelTest.kt
//
// TODO: Translate imports
// TODO: Translate suspend functions to C++ coroutines
// TODO: Translate test annotations to C++ test framework
// TODO: Handle @Suppress annotations

namespace kotlinx {
namespace coroutines {
namespace channels {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlin.test.*

// TODO: @Suppress("DEPRECATION_ERROR")
class BufferedBroadcastChannelTest : public TestBase {
public:

    // TODO: @Test
    void testConcurrentModification() /* = runTest */ {
        // TODO: auto channel = BroadcastChannel<int>(1);
        // TODO: auto s1 = channel.openSubscription();
        // TODO: auto s2 = channel.openSubscription();

        // TODO: auto job1 = launch(Dispatchers::Unconfined, CoroutineStart::UNDISPATCHED, [&]() {
        //     expect(1);
        //     s1.receive();
        //     s1.cancel();
        // });

        // TODO: auto job2 = launch(Dispatchers::Unconfined, CoroutineStart::UNDISPATCHED, [&]() {
        //     expect(2);
        //     s2.receive();
        // });

        expect(3);
        // TODO: channel.send(1);
        // TODO: joinAll(job1, job2);
        finish(4);
    }

    // TODO: @Test
    void testBasic() /* = runTest */ {
        expect(1);
        // TODO: auto broadcast = BroadcastChannel<int>(1);
        // TODO: assertFalse(broadcast.isClosedForSend);
        // TODO: auto first = broadcast.openSubscription();
        // TODO: launch(CoroutineStart::UNDISPATCHED, [&]() {
        //     expect(2);
        //     assertEquals(1, first.receive()); // suspends
        //     assertFalse(first.isClosedForReceive);
        //     expect(5);
        //     assertEquals(2, first.receive()); // suspends
        //     assertFalse(first.isClosedForReceive);
        //     expect(10);
        //     assertTrue(first.receiveCatching().isClosed); // suspends
        //     assertTrue(first.isClosedForReceive);
        //     expect(14);
        // });
        expect(3);
        // TODO: broadcast.send(1);
        expect(4);
        // TODO: yield(); // to the first receiver
        expect(6);

        // TODO: auto second = broadcast.openSubscription();
        // TODO: launch(CoroutineStart::UNDISPATCHED, [&]() {
        //     expect(7);
        //     assertEquals(2, second.receive()); // suspends
        //     assertFalse(second.isClosedForReceive);
        //     expect(11);
        //     assertNull(second.receiveCatching().getOrNull()); // suspends
        //     assertTrue(second.isClosedForReceive);
        //     expect(15);
        // });
        expect(8);
        // TODO: broadcast.send(2);
        expect(9);
        // TODO: yield(); // to first & second receivers
        expect(12);
        // TODO: broadcast.close();
        expect(13);
        // TODO: assertTrue(broadcast.isClosedForSend);
        // TODO: yield(); // to first & second receivers
        finish(16);
    }

    // TODO: @Test
    void testSendSuspend() /* = runTest */ {
        expect(1);
        // TODO: auto broadcast = BroadcastChannel<int>(1);
        // TODO: auto first = broadcast.openSubscription();
        // TODO: launch([&]() {
        //     expect(4);
        //     assertEquals(1, first.receive());
        //     expect(5);
        //     assertEquals(2, first.receive());
        //     expect(6);
        // });
        expect(2);
        // TODO: broadcast.send(1); // puts to buffer, receiver not running yet
        expect(3);
        // TODO: broadcast.send(2); // suspends
        finish(7);
    }

    // TODO: @Test
    void testConcurrentSendCompletion() /* = runTest */ {
        expect(1);
        // TODO: auto broadcast = BroadcastChannel<int>(1);
        // TODO: auto sub = broadcast.openSubscription();
        // launch 3 concurrent senders (one goes buffer, two other suspend)
        // TODO: for (int x = 1; x <= 3; x++) {
        //     launch(CoroutineStart::UNDISPATCHED, [&, x]() {
        //         expect(x + 1);
        //         broadcast.send(x);
        //     });
        // }
        // and close it for send
        expect(5);
        // TODO: broadcast.close();
        // now must receive all 3 items
        expect(6);
        // TODO: assertFalse(sub.isClosedForReceive);
        // TODO: for (int x = 1; x <= 3; x++)
        //     assertEquals(x, sub.receiveCatching().getOrNull());
        // and receive close signal
        // TODO: assertNull(sub.receiveCatching().getOrNull());
        // TODO: assertTrue(sub.isClosedForReceive);
        finish(7);
    }

    // TODO: @Test
    void testForgetUnsubscribed() /* = runTest */ {
        expect(1);
        // TODO: auto broadcast = BroadcastChannel<int>(1);
        // TODO: broadcast.send(1);
        // TODO: broadcast.send(2);
        // TODO: broadcast.send(3);
        expect(2); // should not suspend anywhere above
        // TODO: auto sub = broadcast.openSubscription();
        // TODO: launch(CoroutineStart::UNDISPATCHED, [&]() {
        //     expect(3);
        //     assertEquals(4, sub.receive()); // suspends
        //     expect(5);
        // });
        expect(4);
        // TODO: broadcast.send(4); // sends
        // TODO: yield();
        finish(6);
    }

    // TODO: @Test
    void testReceiveFullAfterClose() /* = runTest */ {
        // TODO: auto channel = BroadcastChannel<int>(10);
        // TODO: auto sub = channel.openSubscription();
        // generate into buffer & close
        // TODO: for (int x = 1; x <= 5; x++) channel.send(x);
        // TODO: channel.close();
        // make sure all of them are consumed
        // TODO: check(!sub.isClosedForReceive);
        // TODO: for (int x = 1; x <= 5; x++) check(sub.receive() == x);
        // TODO: check(sub.receiveCatching().getOrNull() == nullptr);
        // TODO: check(sub.isClosedForReceive);
    }

    // TODO: @Test
    void testCloseSubDuringIteration() /* = runTest */ {
        // TODO: auto channel = BroadcastChannel<int>(1);
        // launch generator (for later) in this context
        // TODO: launch([&]() {
        //     for (int x = 1; x <= 5; x++) {
        //         channel.send(x);
        //     }
        //     channel.close();
        // });
        // start consuming
        // TODO: auto sub = channel.openSubscription();
        int expected = 0;
        // TODO: assertFailsWith<CancellationException>([&]() {
        //     sub.consumeEach([&](int it) {
        //         check(it == ++expected);
        //         if (it == 2) {
        //             sub.cancel();
        //         }
        //     });
        // });
        // TODO: check(expected == 2);
    }

    // TODO: @Test
    void testReceiveFromCancelledSub() /* = runTest */ {
        // TODO: auto channel = BroadcastChannel<int>(1);
        // TODO: auto sub = channel.openSubscription();
        // TODO: assertFalse(sub.isClosedForReceive);
        // TODO: sub.cancel();
        // TODO: assertTrue(sub.isClosedForReceive);
        // TODO: assertFailsWith<CancellationException>([&]() { sub.receive(); });
    }

    // TODO: @Test
    void testCancelWithCause() /* = runTest({ it is TestCancellationException }) */ {
        // TODO: auto channel = BroadcastChannel<int>(1);
        // TODO: auto subscription = channel.openSubscription();
        // TODO: subscription.cancel(TestCancellationException());
        // TODO: subscription.receive();
    }

    // TODO: @Test
    void testReceiveNoneAfterCancel() /* = runTest */ {
        // TODO: auto channel = BroadcastChannel<int>(10);
        // TODO: auto sub = channel.openSubscription();
        // generate into buffer & cancel
        // TODO: for (int x = 1; x <= 5; x++) channel.send(x);
        // TODO: channel.cancel();
        // TODO: assertTrue(channel.isClosedForSend);
        // TODO: assertTrue(sub.isClosedForReceive);
        // TODO: check(sub.receiveCatching().getOrNull() == nullptr);
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
