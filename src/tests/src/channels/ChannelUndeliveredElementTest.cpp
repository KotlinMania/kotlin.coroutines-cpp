// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/channels/ChannelUndeliveredElementTest.kt
//
// TODO: Translate imports
// TODO: Translate suspend functions to C++ coroutines
// TODO: Translate test annotations to C++ test framework
// TODO: Translate atomicfu operations

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.atomicfu.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlin.test.*

            class ChannelUndeliveredElementTest : public TestBase {
            private:
                class Resource {
                private:
                    std::string value_;
                    // TODO: atomic<bool> _cancelled{false};

                public:
                    explicit Resource(const std::string &value) : value_(value) {
                    }

                    const std::string &get_value() const { return value_; }

                    bool is_cancelled() const {
                        // TODO: return _cancelled.value;
                        return false;
                    }

                    void cancel() {
                        // TODO: check(!_cancelled.getAndSet(true)) { "Already cancelled" };
                    }
                };

                /* suspend */
                void runAllKindsTest(/* std::function<void(TestChannelKind)> test */) {
                    // TODO: for (auto kind : TestChannelKind::values()) {
                    //     if (kind.viaBroadcast) continue; // does not support onUndeliveredElement
                    //     try {
                    //         withContext(Job(), [&]() {
                    //             test(kind);
                    //         });
                    //     } catch(const std::exception& e) {
                    //         error(std::string(kind.toString()) + ": " + e.what(), e);
                    //     }
                    // }
                }

            public:
                // TODO: @Test
                void testSendSuccessfully() /* = runTest */ {
                    // TODO: Implementation
                }

                // TODO: @Test
                void testRendezvousSendCancelled() /* = runTest */ {
                    // TODO: Implementation
                }

                // TODO: @Test
                void testBufferedSendCancelled() /* = runTest */ {
                    // TODO: Implementation
                }

                // TODO: @Test
                void testUnlimitedChannelCancelled() /* = runTest */ {
                    // TODO: Implementation
                }

                // TODO: @Test
                void testConflatedResourceCancelled() /* = runTest */ {
                    // TODO: Implementation
                }

                // TODO: @Test
                void testSendToClosedChannel() /* = runTest */ {
                    // TODO: Implementation
                }

                // TODO: @Test
                void testHandlerIsNotInvoked() /* = runTest */ {
                    // #2826
                    // TODO: Implementation
                }

                // TODO: @Test
                void testChannelBufferOverflow() /* = runTest */ {
                    // TODO: testBufferOverflowStrategy({1, 2}, BufferOverflow::DROP_OLDEST);
                    // TODO: testBufferOverflowStrategy({3}, BufferOverflow::DROP_LATEST);
                }

            private:
                /* suspend */
                void testBufferOverflowStrategy(
                    /* std::vector<int> expectedDroppedElements, */ BufferOverflow /* strategy */) {
                    // TODO: Implementation
                }

            public:
                // TODO: @Test
                void testTrySendDoesNotInvokeHandlerOnClosedConflatedChannel() /* = runTest */ {
                    // TODO: Implementation
                }

                // TODO: @Test
                void testTrySendDoesNotInvokeHandlerOnClosedChannel() /* = runTest */ {
                    // TODO: Implementation
                }

                // TODO: @Test
                void testTrySendDoesNotInvokeHandler() {
                    // TODO: for (int capacity = 0; capacity <= 2; capacity++) {
                    //     testTrySendDoesNotInvokeHandler(capacity);
                    // }
                }

            private:
                void testTrySendDoesNotInvokeHandler(int /* capacity */) {
                    // TODO: Implementation
                }
            };
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx