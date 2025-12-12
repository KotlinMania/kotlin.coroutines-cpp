// Original: kotlinx-coroutines-core/concurrent/test/channels/TrySendBlockingTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement Channel, GlobalScope, async
// TODO: Implement trySendBlocking, consumeEach
// TODO: Implement TestException, TestCancellationException

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlin.test.*

            class TrySendBlockingTest : public TestBase {
            public:
                // @Test
                // TODO: Convert test annotation
                void test_try_send_blocking() {
                    runBlocking<void>([&]() {
                        // TODO: suspend function (For old MM)
                        auto ch = Channel<int>();
                        auto sum = GlobalScope::async([&]() {
                            // TODO: suspend function
                            int sum = 0;
                            ch.consume_each([&](int value) {
                                sum += value;
                            });
                            return sum;
                        });
                        for (int i = 0; i < 10; ++i) {
                            assertTrue(ch.try_send_blocking(i).is_success());
                        }
                        ch.close();
                        assertEquals(45, runBlocking([&]() {
                            // TODO: suspend function
                            return sum.await();
                        }));
                    });
                }

                // @Test
                // TODO: Convert test annotation
                void test_try_send_blocking_closed_channel() {
                    {
                        auto channel = Channel<void>();
                        channel.close();
                        channel.try_send_blocking(Unit)
                                .on_success([&]() { expectUnreached(); })
                                .on_failure([&](auto *it) { assertIs<ClosedSendChannelException>(it); })
                                .also([&](auto &it) { assertTrue(it.is_closed()); });
                    }

                    {
                        auto channel = Channel<void>();
                        channel.close(TestException());
                        channel.try_send_blocking(Unit)
                                .on_success([&]() { expectUnreached(); })
                                .on_failure([&](auto *it) { assertIs<TestException>(it); })
                                .also([&](auto &it) { assertTrue(it.is_closed()); });
                    }

                    {
                        auto channel = Channel<void>();
                        channel.cancel(TestCancellationException());
                        channel.try_send_blocking(Unit)
                                .on_success([&]() { expectUnreached(); })
                                .on_failure([&](auto *it) { assertIs<TestCancellationException>(it); })
                                .also([&](auto &it) { assertTrue(it.is_closed()); });
                    }
                }
            };
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx