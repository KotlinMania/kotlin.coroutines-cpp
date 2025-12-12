// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/channels/ChannelBufferOverflowTest.kt
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

            class ChannelBufferOverflowTest : public TestBase {
            public:
                // TODO: @Test
                void testDropLatest() /* = runTest */ {
                    // TODO: auto c = Channel<int>(2, BufferOverflow::DROP_LATEST);
                    // TODO: assertTrue(c.trySend(1).isSuccess);
                    // TODO: assertTrue(c.trySend(2).isSuccess);
                    // TODO: assertTrue(c.trySend(3).isSuccess); // overflows, dropped
                    // TODO: c.send(4); // overflows dropped
                    // TODO: assertEquals(1, c.receive());
                    // TODO: assertTrue(c.trySend(5).isSuccess);
                    // TODO: assertTrue(c.trySend(6).isSuccess); // overflows, dropped
                    // TODO: assertEquals(2, c.receive());
                    // TODO: assertEquals(5, c.receive());
                    // TODO: assertEquals(nullptr, c.tryReceive().getOrNull());
                }

                // TODO: @Test
                void testDropOldest() /* = runTest */ {
                    // TODO: auto c = Channel<int>(2, BufferOverflow::DROP_OLDEST);
                    // TODO: assertTrue(c.trySend(1).isSuccess);
                    // TODO: assertTrue(c.trySend(2).isSuccess);
                    // TODO: assertTrue(c.trySend(3).isSuccess); // overflows, keeps 2, 3
                    // TODO: c.send(4); // overflows, keeps 3, 4
                    // TODO: assertEquals(3, c.receive());
                    // TODO: assertTrue(c.trySend(5).isSuccess);
                    // TODO: assertTrue(c.trySend(6).isSuccess); // overflows, keeps 5, 6
                    // TODO: assertEquals(5, c.receive());
                    // TODO: assertEquals(6, c.receive());
                    // TODO: assertEquals(nullptr, c.tryReceive().getOrNull());
                }
            };
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx