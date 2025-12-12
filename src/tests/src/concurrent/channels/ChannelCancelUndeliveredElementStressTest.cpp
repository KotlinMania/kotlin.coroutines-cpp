// Original: kotlinx-coroutines-core/concurrent/test/channels/ChannelCancelUndeliveredElementStressTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement atomic operations with std::atomic
// TODO: Implement Channel with onUndeliveredElement
// TODO: Implement Random, select, assertIs
// TODO: Implement BufferedChannel

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.atomicfu.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlinx.coroutines.selects.*
            // TODO: import kotlin.random.*
            // TODO: import kotlin.test.*

            class ChannelCancelUndeliveredElementStressTest : public TestBase {
            private:
                int repeat_times = (is_native ? 1'000 : 10'000) * stress_test_multiplier;

                // total counters
                int send_cnt = 0;
                int try_send_failed_cnt = 0;
                int received_cnt = 0;
                int undelivered_cnt = 0;

                // last operation
                int last_received = 0;
                int d_send_cnt = 0;
                int d_send_exception_cnt = 0;
                int d_try_send_failed_cnt = 0;
                int d_received_cnt = 0;
                std::atomic<int> d_undelivered_cnt{0};

            public:
                // @Test
                // TODO: Convert test annotation
                void test_stress() {
                    runTest([&]() {
                        // TODO: suspend function
                        for (int i = 0; i < repeat_times; ++i) {
                            auto channel = Channel<int>(1, [&](int) {
                                d_undelivered_cnt.fetch_add(1);
                            });
                            auto j1 = launch(Dispatchers::Default, [&]() {
                                // TODO: suspend function
                                send_one(channel); // send first
                                send_one(channel); // send second
                            });
                            auto j2 = launch(Dispatchers::Default, [&]() {
                                // TODO: suspend function
                                receive_one(channel); // receive one element from the channel
                                channel.cancel(); // cancel the channel
                            });

                            join_all(j1, j2);

                            // All elements must be either received or undelivered (IN every run)
                            if (d_send_cnt - d_try_send_failed_cnt != d_received_cnt + d_undelivered_cnt.load()) {
                                std::cout << "          Send: " << d_send_cnt << std::endl;
                                std::cout << "Send exception: " << d_send_exception_cnt << std::endl;
                                std::cout << "trySend failed: " << d_try_send_failed_cnt << std::endl;
                                std::cout << "      Received: " << d_received_cnt << std::endl;
                                std::cout << "   Undelivered: " << d_undelivered_cnt.load() << std::endl;
                                throw std::runtime_error("Failed");
                            }
                            auto *buffered_channel = dynamic_cast<BufferedChannel<int> *>(&channel);
                            if (buffered_channel) {
                                buffered_channel->check_segment_structure_invariants();
                            }
                            try_send_failed_cnt += d_try_send_failed_cnt;
                            received_cnt += d_received_cnt;
                            undelivered_cnt += d_undelivered_cnt.load();
                            // clear for next run
                            d_send_cnt = 0;
                            d_send_exception_cnt = 0;
                            d_try_send_failed_cnt = 0;
                            d_received_cnt = 0;
                            d_undelivered_cnt.store(0);
                        }
                        // Stats
                        std::cout << "          Send: " << send_cnt << std::endl;
                        std::cout << "trySend failed: " << try_send_failed_cnt << std::endl;
                        std::cout << "      Received: " << received_cnt << std::endl;
                        std::cout << "   Undelivered: " << undelivered_cnt << std::endl;
                        assertEquals(send_cnt - try_send_failed_cnt, received_cnt + undelivered_cnt);
                    });
                }

            private:
                void send_one(Channel<int> &channel) {
                    // TODO: suspend function
                    d_send_cnt++;
                    int i = ++send_cnt;
                    try {
                        switch (Random::next_int(2)) {
                            case 0:
                                channel.send(i);
                                break;
                            case 1:
                                if (!channel.try_send(i).is_success()) {
                                    d_try_send_failed_cnt++;
                                }
                                break;
                        }
                    } catch (const std::exception &e) {
                        // assertIs<CancellationException>(e) // the only exception possible in this test
                        d_send_exception_cnt++;
                        throw;
                    }
                }

                void receive_one(Channel<int> &channel) {
                    // TODO: suspend function
                    int received;
                    switch (Random::next_int(3)) {
                        case 0:
                            received = channel.receive();
                            break;
                        case 1:
                            received = channel.receive_catching().get_or_else([]() {
                                throw std::runtime_error("Cannot be closed yet");
                            });
                            break;
                        case 2:
                            received = select<int>([&](auto &builder) {
                                builder.on_receive(channel, [](int it) { return it; });
                            });
                            break;
                        default:
                            throw std::runtime_error("Cannot happen");
                    }
                    assertTrue(received > last_received);
                    d_received_cnt++;
                    last_received = received;
                }
            };
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx