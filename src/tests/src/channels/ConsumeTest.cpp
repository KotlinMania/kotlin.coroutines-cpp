// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/channels/ConsumeTest.kt
//
// TODO: Translate file-level annotations (@file:OptIn)
// TODO: Translate imports
// TODO: Translate suspend functions to C++ coroutines
// TODO: Translate test annotations to C++ test framework

// TODO: @file:OptIn(DelicateCoroutinesApi::class)

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlin.test.*

            class ConsumeTest : public TestBase {
            public:
                /** Check that [ReceiveChannel.consume] does not suffer from KT-58685 */
                // TODO: @Test
                void testConsumeJsMiscompilation() /* = runTest */ {
                    // TODO: Implementation
                }

                /** Checks that [ReceiveChannel.consume] closes the channel when the block executes successfully. */
                // TODO: @Test
                void testConsumeClosesOnSuccess() /* = runTest */ {
                    // TODO: Implementation
                }

                /** Checks that [ReceiveChannel.consume] closes the channel when the block executes successfully. */
                // TODO: @Test
                void testConsumeClosesOnFailure() /* = runTest */ {
                    // TODO: Implementation
                }

                /** Checks that [ReceiveChannel.consume] closes the channel when the block does an early return. */
                // TODO: @Test
                void testConsumeClosesOnEarlyReturn() /* = runTest */ {
                    // TODO: Implementation
                }

                /** Checks that [ReceiveChannel.consume] closes the channel when the block executes successfully. */
                // TODO: @Test
                void testConsumeEachClosesOnSuccess() /* = runTest */ {
                    // TODO: Implementation
                }

                /** Checks that [ReceiveChannel.consume] closes the channel when the block executes successfully. */
                // TODO: @Test
                void testConsumeEachClosesOnFailure() /* = runTest */ {
                    // TODO: Implementation
                }

                /** Checks that [ReceiveChannel.consume] closes the channel when the block does an early return. */
                // TODO: @Test
                void testConsumeEachClosesOnEarlyReturn() /* = runTest */ {
                    // TODO: Implementation
                }

                /** Checks that [ReceiveChannel.consumeEach] reacts to cancellation, but processes the elements that are
     * readily available in the buffer. */
                // TODO: @Test
                void testConsumeEachExitsOnCancellation() /* = runTest */ {
                    // TODO: Implementation with undeliveredElements
                }

                // TODO: @Test
                void testConsumeEachThrowingOnChannelClosing() /* = runTest */ {
                    // TODO: Implementation
                }

                /** Check that [BroadcastChannel.consume] does not suffer from KT-58685 */
                // TODO: @Suppress("DEPRECATION", "DEPRECATION_ERROR")
                // TODO: @Test
                void testBroadcastChannelConsumeJsMiscompilation() /* = runTest */ {
                    // TODO: Implementation
                }
            };
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx