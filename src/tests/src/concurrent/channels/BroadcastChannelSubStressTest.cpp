// Original: kotlinx-coroutines-core/concurrent/test/channels/BroadcastChannelSubStressTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement atomic operations with std::atomic
// TODO: Implement TestBroadcastChannelKind, broadcast channels
// TODO: Implement launch, delay, withTimeout
// TODO: Implement CoroutineName

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.atomicfu.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlin.test.*

            /**
 * Creates a broadcast channel and repeatedly opens new subscription, receives event, closes it,
 * to stress test the logic of opening the subscription
 * to broadcast channel while events are being concurrently sent to it.
 */
            class BroadcastChannelSubStressTest : public TestBase {
            private:
                int n_seconds = std::max(5, stress_test_multiplier);
                std::atomic<int64_t> sent_total{0};
                std::atomic<int64_t> received_total{0};

            public:
                // @Test
                // TODO: Convert test annotation
                void test_stress() {
                    runTest([&]() {
                        // TODO: suspend function
                        for (auto kind: TestBroadcastChannelKind::entries) {
                            std::cout << "--- BroadcastChannelSubStressTest " << kind << std::endl;
                            auto broadcast = kind.create<int64_t>();
                            auto sender = launch(Dispatchers::Default + CoroutineName("Sender"), [&]() {
                                // TODO: suspend function
                                while (is_active()) {
                                    broadcast.send(sent_total.fetch_add(1) + 1);
                                }
                            });
                            auto receiver = launch(Dispatchers::Default + CoroutineName("Receiver"), [&]() {
                                // TODO: suspend function
                                int64_t last = -1;
                                while (is_active()) {
                                    auto channel = broadcast.open_subscription();
                                    auto i = channel.receive();
                                    if (i < last) {
                                        throw std::runtime_error(
                                            "Last was " + std::to_string(last) + ", got " + std::to_string(i));
                                    }
                                    if (!kind.is_conflated && i == last) {
                                        throw std::runtime_error("Last was " + std::to_string(last) + ", got it again");
                                    }
                                    received_total.fetch_add(1);
                                    last = i;
                                    channel.cancel();
                                }
                            });
                            int64_t prev_sent = -1;
                            for (int sec = 0; sec < n_seconds; ++sec) {
                                delay(1000);
                                int64_t cur_sent = sent_total.load();
                                std::cout << (sec + 1) << ": Sent " << cur_sent << ", received " << received_total.
                                        load() << std::endl;
                                if (cur_sent <= prev_sent) {
                                    throw std::runtime_error("Send stalled at " + std::to_string(cur_sent) + " events");
                                }
                                prev_sent = cur_sent;
                            }
                            with_timeout(5000, [&]() {
                                // TODO: suspend function
                                sender.cancel_and_join();
                                receiver.cancel_and_join();
                            });
                        }
                    });
                }
            };
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx