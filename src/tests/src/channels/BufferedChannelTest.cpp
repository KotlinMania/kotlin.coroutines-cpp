// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/channels/BufferedChannelTest.kt
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

            class BufferedChannelTest : public TestBase {
            public:
                /** Tests that a buffered channel does not consume enough memory to fail with an OOM. */
                // TODO: @Test
                void testMemoryConsumption() /* = runTest */ {
                    // TODO: auto largeChannel = Channel<int>(INT_MAX / 2);
                    // TODO: repeat(10000, [&](int it) {
                    //     largeChannel.send(it);
                    // });
                    // TODO: repeat(10000, [&](int it) {
                    //     auto element = largeChannel.receive();
                    //     assertEquals(it, element);
                    // });
                }

                // TODO: @Test
                void testIteratorHasNextIsIdempotent() /* = runTest */ {
                    // TODO: auto q = Channel<int>();
                    // TODO: check(q.isEmpty);
                    // TODO: auto iter = q.iterator();
                    expect(1);
                    // TODO: auto sender = launch([&]() {
                    //     expect(4);
                    //     q.send(1); // sent
                    //     expect(10);
                    //     q.close();
                    //     expect(11);
                    // });
                    expect(2);
                    // TODO: auto receiver = launch([&]() {
                    //     expect(5);
                    //     check(iter.hasNext());
                    //     expect(6);
                    //     check(iter.hasNext());
                    //     expect(7);
                    //     check(iter.hasNext());
                    //     expect(8);
                    //     check(iter.next() == 1);
                    //     expect(9);
                    //     check(!iter.hasNext());
                    //     expect(12);
                    // });
                    expect(3);
                    // TODO: sender.join();
                    // TODO: receiver.join();
                    // TODO: check(q.isClosedForReceive);
                    finish(13);
                }

                // TODO: @Test
                void testSimple() /* = runTest */ {
                    // TODO: auto q = Channel<int>(1);
                    // TODO: check(q.isEmpty);
                    expect(1);
                    // TODO: auto sender = launch([&]() {
                    //     expect(4);
                    //     q.send(1); // success -- buffered
                    //     check(!q.isEmpty);
                    //     expect(5);
                    //     q.send(2); // suspends (buffer full)
                    //     expect(9);
                    // });
                    expect(2);
                    // TODO: auto receiver = launch([&]() {
                    //     expect(6);
                    //     check(q.receive() == 1); // does not suspend -- took from buffer
                    //     check(!q.isEmpty); // waiting sender's element moved to buffer
                    //     expect(7);
                    //     check(q.receive() == 2); // does not suspend (takes from sender)
                    //     expect(8);
                    // });
                    expect(3);
                    // TODO: sender.join();
                    // TODO: receiver.join();
                    // TODO: check(q.isEmpty);
                    // TODO: static_cast<BufferedChannel<int>*>(&q)->checkSegmentStructureInvariants();
                    finish(10);
                }

                // TODO: @Test
                void testClosedBufferedReceiveCatching() /* = runTest */ {
                    // TODO: auto q = Channel<int>(1);
                    // TODO: check(q.isEmpty && !q.isClosedForSend && !q.isClosedForReceive);
                    expect(1);
                    // TODO: launch([&]() {
                    //     expect(5);
                    //     check(!q.isEmpty && q.isClosedForSend && !q.isClosedForReceive);
                    //     assertEquals(42, q.receiveCatching().getOrNull());
                    //     expect(6);
                    //     check(!q.isEmpty && q.isClosedForSend && q.isClosedForReceive);
                    //     assertNull(q.receiveCatching().getOrNull());
                    //     expect(7);
                    // });
                    expect(2);
                    // TODO: q.send(42); // buffers
                    expect(3);
                    // TODO: q.close(); // goes on
                    expect(4);
                    // TODO: check(!q.isEmpty && q.isClosedForSend && !q.isClosedForReceive);
                    // TODO: yield();
                    // TODO: check(!q.isEmpty && q.isClosedForSend && q.isClosedForReceive);
                    // TODO: static_cast<BufferedChannel<int>*>(&q)->checkSegmentStructureInvariants();
                    finish(8);
                }

                // TODO: @Test
                void testClosedExceptions() /* = runTest */ {
                    // TODO: auto q = Channel<int>(1);
                    expect(1);
                    // TODO: launch([&]() {
                    //     expect(4);
                    //     try { q.receive(); }
                    //     catch (const ClosedReceiveChannelException& e) {
                    //         expect(5);
                    //     }
                    // });
                    expect(2);

                    // TODO: require(q.close());
                    expect(3);
                    // TODO: yield();
                    expect(6);
                    // TODO: try { q.send(42); }
                    // TODO: catch (const ClosedSendChannelException& e) {
                    //     static_cast<BufferedChannel<int>*>(&q)->checkSegmentStructureInvariants();
                    //     finish(7);
                    // }
                }

                // TODO: @Test
                void testTryOp() /* = runTest */ {
                    // TODO: auto q = Channel<int>(1);
                    // TODO: assertTrue(q.trySend(1).isSuccess);
                    expect(1);
                    // TODO: launch([&]() {
                    //     expect(3);
                    //     assertEquals(1, q.tryReceive().getOrNull());
                    //     expect(4);
                    //     assertNull(q.tryReceive().getOrNull());
                    //     expect(5);
                    //     assertEquals(2, q.receive()); // suspends
                    //     expect(9);
                    //     assertEquals(3, q.tryReceive().getOrNull());
                    //     expect(10);
                    //     assertNull(q.tryReceive().getOrNull());
                    //     expect(11);
                    // });
                    expect(2);
                    // TODO: yield();
                    expect(6);
                    // TODO: assertTrue(q.trySend(2).isSuccess);
                    expect(7);
                    // TODO: assertTrue(q.trySend(3).isSuccess);
                    expect(8);
                    // TODO: assertFalse(q.trySend(4).isSuccess);
                    // TODO: yield();
                    // TODO: static_cast<BufferedChannel<int>*>(&q)->checkSegmentStructureInvariants();
                    finish(12);
                }

                // TODO: @Test
                void testConsumeAll() /* = runTest */ {
                    // TODO: auto q = Channel<int>(5);
                    // TODO: for (int i = 1; i <= 10; i++) {
                    //     if (i <= 5) {
                    //         expect(i);
                    //         q.send(i); // shall buffer
                    //     } else {
                    //         launch(CoroutineStart::UNDISPATCHED, [&, i]() {
                    //             expect(i);
                    //             q.send(i); // suspends
                    //             expectUnreached(); // will get cancelled by cancel
                    //         });
                    //     }
                    // }
                    expect(11);
                    // TODO: q.cancel();
                    // TODO: check(q.isClosedForSend);
                    // TODO: check(q.isClosedForReceive);
                    // TODO: assertFailsWith<CancellationException>([&]() { q.receiveCatching().getOrThrow(); });
                    // TODO: static_cast<BufferedChannel<int>*>(&q)->checkSegmentStructureInvariants();
                    finish(12);
                }

                // TODO: @Test
                void testCancelWithCause() /* = runTest({ it is TestCancellationException }) */ {
                    // TODO: auto channel = Channel<int>(5);
                    // TODO: channel.cancel(TestCancellationException());
                    // TODO: channel.receive();
                }

                // TODO: @Test
                void testBufferSize() /* = runTest */ {
                    int capacity = 42;
                    // TODO: auto channel = Channel<int>(capacity);
                    // TODO: checkBufferChannel(channel, capacity);
                }

                // TODO: @Test
                void testBufferSizeFromTheMiddle() /* = runTest */ {
                    int capacity = 42;
                    // TODO: auto channel = Channel<int>(capacity);
                    // TODO: repeat(4, [&](int) {
                    //     channel.trySend(-1);
                    // });
                    // TODO: repeat(4, [&](int) {
                    //     channel.receiveCatching().getOrNull();
                    // });
                    // TODO: checkBufferChannel(channel, capacity);
                }

                // TODO: @Test
                void testBufferIsNotPreallocated() {
                    // TODO: for (int i = 0; i <= 100000; i++) {
                    //     Channel<int>(INT_MAX / 2);
                    // }
                }

            private:
                /* suspend */
                void checkBufferChannel(
                    /* Channel<int>& channel, */
                    int capacity
                ) {
                    // TODO: launch([&]() {
                    //     expect(2);
                    //     repeat(42, [&](int it) {
                    //         channel.send(it);
                    //     });
                    //     expect(3);
                    //     channel.send(42);
                    //     expect(5);
                    //     channel.close();
                    // });

                    expect(1);
                    // TODO: yield();

                    expect(4);
                    // TODO: std::vector<int> result;
                    // TODO: result.reserve(42);
                    // TODO: channel.consumeEach([&](int it) {
                    //     result.push_back(it);
                    // });
                    // TODO: assertEquals((0..capacity).toList(), result);
                    // TODO: static_cast<BufferedChannel<int>*>(&channel)->checkSegmentStructureInvariants();
                    finish(6);
                }
            };
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx