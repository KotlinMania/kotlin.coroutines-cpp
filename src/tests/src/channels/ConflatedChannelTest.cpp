// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/channels/ConflatedChannelTest.kt
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

            class ConflatedChannelTest : public TestBase {
            protected:
                template<typename T>
                /* Channel<T> */ void *createConflatedChannel() {
                    // TODO: return Channel<T>(Channel::CONFLATED);
                    return nullptr;
                }

            public:
                // TODO: @Test
                void testBasicConflationOfferTryReceive() {
                    // TODO: auto q = createConflatedChannel<int>();
                    // TODO: assertNull(q.tryReceive().getOrNull());
                    // TODO: assertTrue(q.trySend(1).isSuccess);
                    // TODO: assertTrue(q.trySend(2).isSuccess);
                    // TODO: assertTrue(q.trySend(3).isSuccess);
                    // TODO: assertEquals(3, q.tryReceive().getOrNull());
                    // TODO: assertNull(q.tryReceive().getOrNull());
                }

                // TODO: @Test
                void testConflatedSend() /* = runTest */ {
                    // TODO: auto q = createConflatedChannel<int>();
                    // TODO: q.send(1);
                    // TODO: q.send(2); // shall conflated previously sent
                    // TODO: assertEquals(2, q.receiveCatching().getOrNull());
                }

                // TODO: @Test
                void testConflatedClose() /* = runTest */ {
                    // TODO: Implementation
                }

                // TODO: @Test
                void testConflationSendReceive() /* = runTest */ {
                    // TODO: Implementation
                }

                // TODO: @Test
                void testConsumeAll() /* = runTest */ {
                    expect(1);
                    // TODO: Implementation
                    finish(2);
                }

                // TODO: @Test
                void testCancelWithCause() /* = runTest({ it is TestCancellationException }) */ {
                    // TODO: auto channel = createConflatedChannel<int>();
                    // TODO: channel.cancel(TestCancellationException());
                    // TODO: channel.receive();
                }
            };
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx