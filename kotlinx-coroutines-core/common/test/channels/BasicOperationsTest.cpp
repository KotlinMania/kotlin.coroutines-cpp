// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/channels/BasicOperationsTest.kt
//
// TODO: Translate imports
// TODO: Translate suspend functions to C++ coroutines
// TODO: Translate test annotations to C++ test framework
// TODO: Handle Kotlin nullable types
// TODO: Translate exception handling

namespace kotlinx {
namespace coroutines {
namespace channels {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlin.test.*

class BasicOperationsTest : public TestBase {
public:
    // TODO: @Test
    void testSimpleSendReceive() /* = runTest */ {
        // Parametrized common test :(
        // TODO: TestChannelKind.values().forEach { kind -> testSendReceive(kind, 20) }
    }

    // TODO: @Test
    void testTrySendToFullChannel() /* = runTest */ {
        // TODO: TestChannelKind.values().forEach { kind -> testTrySendToFullChannel(kind) }
    }

    // TODO: @Test
    void testTrySendAfterClose() /* = runTest */ {
        // TODO: TestChannelKind.values().forEach { kind -> testTrySendAfterClose(kind) }
    }

    // TODO: @Test
    void testSendAfterClose() /* = runTest */ {
        // TODO: TestChannelKind.values().forEach { kind -> testSendAfterClose(kind) }
    }

    // TODO: @Test
    void testReceiveCatching() /* = runTest */ {
        // TODO: TestChannelKind.values().forEach { kind -> testReceiveCatching(kind) }
    }

    // TODO: @Test
    void testInvokeOnClose() /* = TestChannelKind.values().forEach */ {
        reset();
        // TODO: auto channel = kind.create<int>();
        // TODO: channel.invokeOnClose([](auto it) {
        //     if (/* it is AssertionError */) {
        //         expect(3);
        //     }
        // });
        expect(1);
        // TODO: channel.trySend(42);
        expect(2);
        // TODO: channel.close(AssertionError());
        finish(4);
    }

    // TODO: @Test
    void testInvokeOnClosed() /* = TestChannelKind.values().forEach */ {
        reset();
        expect(1);
        // TODO: auto channel = kind.create<int>();
        // TODO: channel.close();
        // TODO: channel.invokeOnClose([]() { expect(2); });
        // TODO: assertFailsWith<IllegalStateException>([&]() { channel.invokeOnClose([]() { expect(3); }); });
        finish(3);
    }

    // TODO: @Test
    void testMultipleInvokeOnClose() /* = TestChannelKind.values().forEach */ {
        reset();
        // TODO: auto channel = kind.create<int>();
        // TODO: channel.invokeOnClose([]() { expect(3); });
        expect(1);
        // TODO: assertFailsWith<IllegalStateException>([&]() { channel.invokeOnClose([]() { expect(4); }); });
        expect(2);
        // TODO: channel.close();
        finish(4);
    }

    // TODO: @Test
    void testIterator() /* = runTest */ {
        // TODO: TestChannelKind.values().forEach([](auto kind) {
        //     auto channel = kind.create<int>();
        //     auto iterator = channel.iterator();
        //     assertFailsWith<IllegalStateException>([&]() { iterator.next(); });
        //     channel.close();
        //     assertFailsWith<IllegalStateException>([&]() { iterator.next(); });
        //     assertFalse(iterator.hasNext());
        // });
    }

    // TODO: @Test
    void testCancelledChannelInvokeOnClose() {
        // TODO: auto ch = Channel<int>();
        // TODO: ch.invokeOnClose([](auto it) { assertIs<CancellationException>(it); });
        // TODO: ch.cancel();
    }

    // TODO: @Test
    void testCancelledChannelWithCauseInvokeOnClose() {
        // TODO: auto ch = Channel<int>();
        // TODO: ch.invokeOnClose([](auto it) { assertIs<TimeoutCancellationException>(it); });
        // TODO: ch.cancel(TimeoutCancellationException(""));
    }

    // TODO: @Test
    void testThrowingInvokeOnClose() /* = runTest */ {
        // TODO: auto channel = Channel<int>();
        // TODO: channel.invokeOnClose([](auto it) {
        //     assertNull(it);
        //     expect(3);
        //     throw TestException();
        // });

        // TODO: launch([&]() {
        //     try {
        //         expect(2);
        //         channel.close();
        //     } catch (const TestException& e) {
        //         expect(4);
        //     }
        // });
        expect(1);
        // TODO: yield();
        // TODO: assertTrue(channel.isClosedForReceive);
        // TODO: assertTrue(channel.isClosedForSend);
        // TODO: assertFalse(channel.close());
        finish(5);
    }

    // TODO: @Suppress("ReplaceAssertBooleanWithAssertEquality")
private:
    /* suspend */ void testReceiveCatching(/* kind: TestChannelKind */) /* = coroutineScope */ {
        reset();
        // TODO: auto channel = kind.create<int>();
        // TODO: launch([&]() {
        //     expect(2);
        //     channel.send(1);
        // });

        expect(1);
        // TODO: auto result = channel.receiveCatching();
        // TODO: assertEquals(1, result.getOrThrow());
        // TODO: assertEquals(1, result.getOrNull());
        // TODO: assertTrue(ChannelResult.success(1) == result);

        expect(3);
        // TODO: launch([&]() {
        //     expect(4);
        //     channel.close();
        // });
        // TODO: auto closed = channel.receiveCatching();
        expect(5);
        // TODO: assertNull(closed.getOrNull());
        // TODO: assertTrue(closed.isClosed);
        // TODO: assertNull(closed.exceptionOrNull());
        // TODO: assertTrue(ChannelResult.closed<int>(closed.exceptionOrNull()) == closed);
        finish(6);
    }

    /* suspend */ void testTrySendAfterClose(/* kind: TestChannelKind */) /* = coroutineScope */ {
        // TODO: auto channel = kind.create<int>();
        // TODO: auto d = async([&]() { channel.send(42); });
        // TODO: yield();
        // TODO: channel.close();

        // TODO: assertTrue(channel.isClosedForSend);
        // TODO: channel.trySend(2)
        //     .onSuccess([]() { expectUnreached(); })
        //     .onClosed([&](auto it) {
        //         assertTrue(/* it is ClosedSendChannelException */);
        //         if (!kind.isConflated) {
        //             assertEquals(42, channel.receive());
        //         }
        //     });
        // TODO: d.await();
    }

    /* suspend */ void testTrySendToFullChannel(/* kind: TestChannelKind */) /* = coroutineScope */ {
        // TODO: if (kind.isConflated || kind.capacity == INT_MAX) return;
        // TODO: auto channel = kind.create<int>();
        // Make it full
        // TODO: repeat(11, [&](int) {
        //     channel.trySend(42);
        // });
        // TODO: channel.trySend(1)
        //     .onSuccess([]() { expectUnreached(); })
        //     .onFailure([](auto it) { assertNull(it); })
        //     .onClosed([]() {
        //         expectUnreached();
        //     });
    }

    /**
     * [ClosedSendChannelException] should not be eaten.
     * See [https://github.com/Kotlin/kotlinx.coroutines/issues/957]
     */
    /* suspend */ void testSendAfterClose(/* kind: TestChannelKind */) {
        // TODO: assertFailsWith<ClosedSendChannelException>([&]() {
        //     coroutineScope([&]() {
        //         auto channel = kind.create<int>();
        //         channel.close();
        //
        //         launch([&]() {
        //             channel.send(1);
        //         });
        //     });
        // });
    }

    /* suspend */ void testSendReceive(/* kind: TestChannelKind, */ int iterations) /* = coroutineScope */ {
        // TODO: auto channel = kind.create<int>();
        // TODO: launch([&]() {
        //     repeat(iterations, [&](int it) { channel.send(it); });
        //     channel.close();
        // });
        int expected = 0;
        // TODO: for (auto x : channel) {
        //     if (!kind.isConflated) {
        //         assertEquals(expected++, x);
        //     } else {
        //         assertTrue(x >= expected);
        //         expected = x + 1;
        //     }
        // }
        // TODO: if (!kind.isConflated) {
        //     assertEquals(iterations, expected);
        // }
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
