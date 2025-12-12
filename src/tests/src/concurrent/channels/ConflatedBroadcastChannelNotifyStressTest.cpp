// Original: kotlinx-coroutines-core/concurrent/test/channels/ConflatedBroadcastChannelNotifyStressTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement atomic operations with std::atomic
// TODO: Implement ConflatedBroadcastChannel (deprecated)
// TODO: Handle @Suppress("DEPRECATION_ERROR") annotation

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.atomicfu.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlin.test.*

            // @Suppress("DEPRECATION_ERROR")
            class ConflatedBroadcastChannelNotifyStressTest : public TestBase {
            private:
                static constexpr int n_senders = 2;
                static constexpr int n_receivers = 3;
                int n_events = (is_native ? 5'000 : 500'000) * stress_test_multiplier;
                int64_t time_limit = 30'000L * stress_test_multiplier; // 30 sec

                ConflatedBroadcastChannel<int> broadcast;

                std::atomic<int> senders_completed{0};
                std::atomic<int> receivers_completed{0};
                std::atomic<int> sent_total{0};
                std::atomic<int> received_total{0};

            public:
                // @Test
                // TODO: Convert test annotation
                void test_stress_notify() {
                    runTest([&]() {
                        // TODO: suspend function
                        std::cout << "--- ConflatedBroadcastChannelNotifyStressTest" << std::endl;
                        std::vector<Job> senders;
                        for (int sender_id = 0; sender_id < n_senders; ++sender_id) {
                            senders.push_back(launch(
                                Dispatchers::Default + CoroutineName("Sender" + std::to_string(sender_id)),
                                [&, sender_id]() {
                                    // TODO: suspend function
                                    for (int i = 0; i < n_events; ++i) {
                                        if (i % n_senders == sender_id) {
                                            broadcast.try_send(i);
                                            sent_total.fetch_add(1);
                                            yield();
                                        }
                                    }
                                    senders_completed.fetch_add(1);
                                }));
                        }
                        std::vector<Job> receivers;
                        for (int receiver_id = 0; receiver_id < n_receivers; ++receiver_id) {
                            receivers.push_back(launch(
                                Dispatchers::Default + CoroutineName("Receiver" + std::to_string(receiver_id)), [&]() {
                                    // TODO: suspend function
                                    int last = -1;
                                    while (is_active()) {
                                        int i = wait_for_event();
                                        if (i > last) {
                                            received_total.fetch_add(1);
                                            last = i;
                                        }
                                        if (i >= n_events) break;
                                        yield();
                                    }
                                    receivers_completed.fetch_add(1);
                                }));
                        }
                        // print progress
                        auto progress_job = launch([&]() {
                            // TODO: suspend function
                            int seconds = 0;
                            while (true) {
                                delay(1000);
                                std::cout << (++seconds) << ": Sent " << sent_total.load() << ", received " <<
                                        received_total.load() << std::endl;
                            }
                        });
                        try {
                            with_timeout(time_limit, [&]() {
                                // TODO: suspend function
                                for (auto &sender: senders) {
                                    sender.join();
                                }
                                broadcast.try_send(n_events); // last event to signal receivers termination
                                for (auto &receiver: receivers) {
                                    receiver.join();
                                }
                            });
                        } catch (const CancellationException &e) {
                            std::cout << "!!! Test timed out " << e.what() << std::endl;
                        }
                        progress_job.cancel();
                        std::cout << "Tested with nSenders=" << n_senders << ", nReceivers=" << n_receivers <<
                                std::endl;
                        std::cout << "Completed successfully " << senders_completed.load() << " sender coroutines" <<
                                std::endl;
                        std::cout << "Completed successfully " << receivers_completed.load() << " receiver coroutines"
                                << std::endl;
                        std::cout << "                  Sent " << sent_total.load() << " events" << std::endl;
                        std::cout << "              Received " << received_total.load() << " events" << std::endl;
                        assertEquals(n_senders, senders_completed.load());
                        assertEquals(n_receivers, receivers_completed.load());
                        assertEquals(n_events, sent_total.load());
                    });
                }

            private:
                int wait_for_event() {
                    // TODO: suspend function
                    auto subscription = broadcast.open_subscription();
                    auto value = subscription.receive();
                    subscription.cancel();
                    return value;
                }
            };
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx