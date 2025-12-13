// Original: kotlinx-coroutines-core/concurrent/test/AtomicCancellationTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement runBlocking, launch, async
// TODO: Implement Channel, select, CoroutineStart
// TODO: Implement expect/finish/expectUnreached test utilities
// TODO: Handle @Suppress annotation

#include <future>

#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.coroutines.testing.*
        // TODO: import kotlinx.coroutines.channels.*
        // TODO: import kotlinx.coroutines.selects.*
        // TODO: import kotlin.test.*

        class AtomicCancellationTest : public TestBase {
        public:
            // @Test
            // TODO: Convert test annotation
            void test_send_cancellable() {
                runBlocking([&]() {
                    // TODO: suspend function
                    expect(1);
                    auto channel = channels::Channel<int>();
                    auto job = std::launch(CoroutineStart::UNDISPATCHED, [&]() {
                        // TODO: suspend function
                        expect(2);
                        channel.send(42); // suspends
                        expectUnreached(); // should NOT execute because of cancellation
                    });
                    expect(3);
                    assertEquals(42, channel.receive()); // will schedule sender for further execution
                    job.cancel(); // cancel the job next
                    std::this_thread::yield(); // now yield
                    finish(4);
                });
            }

            // @Suppress("UNUSED_VARIABLE")
            // @Test
            // TODO: Convert test annotation
            void test_select_send_cancellable() {
                runBlocking([&]() {
                    // TODO: suspend function
                    expect(1);
                    auto channel = Channel<int>();
                    auto job = launch(CoroutineStart::UNDISPATCHED, [&]() {
                        // TODO: suspend function
                        expect(2);
                        auto result = select<std::string>([&](auto &builder) {
                            // TODO: suspend function
                            builder.on_send(channel, 42, [&]() {
                                expect(4);
                                return "OK";
                            });
                        });
                        expectUnreached(); // should NOT execute because of cancellation
                    });
                    expect(3);
                    assertEquals(42, channel.receive()); // will schedule sender for further execution
                    job.cancel(); // cancel the job next
                    yield(); // now yield
                    finish(4);
                });
            }

            // @Test
            // TODO: Convert test annotation
            void test_receive_cancellable() {
                runBlocking([&]() {
                    // TODO: suspend function
                    expect(1);
                    auto channel = Channel<int>();
                    auto job = launch(CoroutineStart::UNDISPATCHED, [&]() {
                        // TODO: suspend function
                        expect(2);
                        assertEquals(42, channel.receive()); // suspends
                        expectUnreached(); // should NOT execute because of cancellation
                    });
                    expect(3);
                    channel.send(42); // will schedule receiver for further execution
                    job.cancel(); // cancel the job next
                    yield(); // now yield
                    finish(4);
                });
            }

            // @Test
            // TODO: Convert test annotation
            void test_select_receive_cancellable() {
                runBlocking([&]() {
                    // TODO: suspend function
                    expect(1);
                    auto channel = Channel<int>();
                    auto job = launch(CoroutineStart::UNDISPATCHED, [&]() {
                        // TODO: suspend function
                        expect(2);
                        auto result = select<std::string>([&](auto &builder) {
                            // TODO: suspend function
                            builder.on_receive(channel, [&](int it) {
                                assertEquals(42, it);
                                expect(4);
                                return "OK";
                            });
                        });
                        expectUnreached(); // should NOT execute because of cancellation
                    });
                    expect(3);
                    channel.send(42); // will schedule receiver for further execution
                    job.cancel(); // cancel the job next
                    yield(); // now yield
                    finish(4);
                });
            }

            // @Test
            // TODO: Convert test annotation
            void test_select_deferred_await_cancellable() {
                runBlocking([&]() {
                    // TODO: suspend function
                    expect(1);
                    auto deferred = async([&]() {
                        // TODO: suspend function (deferred, not yet complete)
                        expect(4);
                        return "OK";
                    });
                    assertEquals(false, deferred.is_completed());
                    Job *job = nullptr;
                    launch([&]() {
                        // TODO: suspend function (will cancel job as soon as deferred completes)
                        expect(5);
                        assertEquals(true, deferred.is_completed());
                        job->cancel();
                    });
                    job = launch(CoroutineStart::UNDISPATCHED, [&]() {
                        // TODO: suspend function
                        expect(2);
                        try {
                            select<void>([&](auto &builder) {
                                // TODO: suspend function
                                builder.on_await(deferred, [&]() {
                                    expectUnreached();
                                });
                            });
                            expectUnreached(); // will not execute -- cancelled while dispatched
                        } catch (...) {
                            finish(7); // but will execute finally blocks
                            throw;
                        }
                    });
                    expect(3); // continues to execute when the job suspends
                    yield(); // to deferred & canceller
                    expect(6);
                });
            }

            // @Test
            // TODO: Convert test annotation
            static void test_select_job_join_cancellable() {
                runBlocking([&]() {
                    // TODO: suspend function
                    expect(1);
                    auto job_to_join = launch([&]() {
                        // TODO: suspend function (not yet complete)
                        expect(4);
                    });
                    assertEquals(false, job_to_join.is_completed());
                    Job *job = nullptr;
                    launch([&]() {
                        // TODO: suspend function (will cancel job as soon as jobToJoin completes)
                        expect(5);
                        assertEquals(true, job_to_join.is_completed());
                        job->cancel();
                    });
                    job = launch(CoroutineStart::UNDISPATCHED, [&]() {
                        // TODO: suspend function
                        expect(2);
                        try {
                            select<void>([&](auto &builder) {
                                // TODO: suspend function
                                builder.on_join(job_to_join, [&]() {
                                    expectUnreached();
                                });
                            });
                            expectUnreached(); // will not execute -- cancelled while dispatched
                        } catch (...) {
                            finish(7); // but will execute finally blocks
                            throw;
                        }
                    });
                    expect(3); // continues to execute when the job suspends
                    std::this_thread::yield(); // to jobToJoin & canceller
                    expect(6);
                });
            }
        };
    } // namespace coroutines
} // namespace kotlinx