// Original: kotlinx-coroutines-core/concurrent/test/RunBlockingTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement runTest, runBlocking, launch, GlobalScope
// TODO: Implement withTimeout, withTimeoutOrNull, delay
// TODO: Implement EventLoop, ContinuationInterceptor
// TODO: Implement Duration types from kotlin.time
// TODO: Implement assertIs, assertSame

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.exceptions.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.test.*
// TODO: import kotlin.time.Duration.Companion.milliseconds
// TODO: import kotlin.time.Duration.Companion.seconds

class RunBlockingTest : public TestBase {
public:
    // @Test
    // TODO: Convert test annotation
    void test_with_timeout_busy_wait() {
        runTest([&]() {
            // TODO: suspend function
            auto value = with_timeout_or_null(10, [&]() -> std::optional<std::string> {
                // TODO: suspend function
                while (is_active()) {
                    // Busy wait
                }
                return "value";
            });

            assertEquals("value", value);
        });
    }

    // @Test
    // TODO: Convert test annotation
    void test_private_event_loop() {
        expect(1);
        runBlocking([&]() {
            // TODO: suspend function
            expect(2);
            auto* event_loop = coroutine_context()[ContinuationInterceptor];
            assertIs<EventLoop>(event_loop);
            yield(); // is supported!
            expect(3);
        });
        finish(4);
    }

    // @Test
    // TODO: Convert test annotation
    void test_outer_event_loop() {
        expect(1);
        runBlocking([&]() {
            // TODO: suspend function
            expect(2);
            auto* outer_event_loop = dynamic_cast<EventLoop*>(coroutine_context()[ContinuationInterceptor]);
            runBlocking(coroutine_context(), [&]() {
                // TODO: suspend function
                expect(3);
                // still same event loop
                assertSame(coroutine_context()[ContinuationInterceptor], outer_event_loop);
                yield(); // still works
                expect(4);
            });
            expect(5);
        });
        finish(6);
    }

    // @Test
    // TODO: Convert test annotation
    void test_other_dispatcher() {
        runTest([&]() {
            // TODO: suspend function
            expect(1);
            std::string name = "RunBlockingTest.testOtherDispatcher";
            auto thread = new_single_thread_context(name);
            runBlocking(thread, [&]() {
                // TODO: suspend function
                expect(2);
                assertSame(coroutine_context()[ContinuationInterceptor], &thread);
                assertTrue(current_thread_name().find(name) != std::string::npos);
                yield(); // should work
                expect(3);
            });
            finish(4);
            thread.close();
        });
    }

    // @Test
    // TODO: Convert test annotation
    void test_cancellation() {
        runTest([&]() {
            // TODO: suspend function
            auto context = new_fixed_thread_pool_context(2, "testCancellation");
            context.use([&]() {
                auto job = GlobalScope::launch(context, [&]() {
                    // TODO: suspend function
                    runBlocking(coroutine_context(), [&]() {
                        // TODO: suspend function
                        while (true) {
                            yield();
                        }
                    });
                });

                runBlocking([&]() {
                    // TODO: suspend function
                    job.cancel_and_join();
                });
            });
        });
    }

    // @Test
    // TODO: Convert test annotation
    void test_cancel_with_delay() {
        // see https://github.com/Kotlin/kotlinx.coroutines/issues/586
        try {
            runBlocking([&]() {
                // TODO: suspend function
                expect(1);
                coroutine_context().cancel();
                expect(2);
                try {
                    delay(1);
                    expectUnreached();
                } catch (...) {
                    expect(3);
                    throw;
                }
            });
            expectUnreached();
        } catch (const CancellationException& e) {
            finish(4);
        }
    }

    // @Test
    // TODO: Convert test annotation
    void test_dispatch_on_shutdown() {
        assert_fails_with<CancellationException>([&]() {
            runBlocking([&]() {
                // TODO: suspend function
                expect(1);
                auto job = launch(NonCancellable, [&]() {
                    // TODO: suspend function
                    try {
                        expect(2);
                        delay(LONG_MAX);
                    } catch (...) {
                        finish(4);
                        throw;
                    }
                });

                yield();
                expect(3);
                coroutine_context().cancel();
                job.cancel();
            });
        });
        // .let { }
    }

    // @Test
    // TODO: Convert test annotation
    void test_dispatch_on_shutdown2() {
        assert_fails_with<CancellationException>([&]() {
            runBlocking([&]() {
                // TODO: suspend function
                coroutine_context().cancel();
                expect(1);
                auto job = launch(NonCancellable, CoroutineStart::kUndispatched, [&]() {
                    // TODO: suspend function
                    try {
                        expect(2);
                        delay(LONG_MAX);
                    } catch (...) {
                        finish(4);
                        throw;
                    }
                });

                expect(3);
                job.cancel();
            });
        });
        // .let { }
    }

    // @Test
    // TODO: Convert test annotation
    void test_nested_run_blocking() {
        runBlocking([&]() {
            // TODO: suspend function
            delay(100);
            auto value = runBlocking([&]() {
                // TODO: suspend function
                delay(100);
                return runBlocking([&]() {
                    // TODO: suspend function
                    delay(100);
                    return 1;
                });
            });

            assertEquals(1, value);
        });
    }

    // @Test
    // TODO: Convert test annotation
    void test_incomplete_state() {
        auto handle = runBlocking([&]() {
            // TODO: suspend function
            // See #835
            return coroutine_context()[Job]->invoke_on_completion([]() { });
        });

        handle.dispose();
    }

    // @Test
    // TODO: Convert test annotation
    void test_cancelled_parent() {
        auto job = Job();
        job.cancel();
        assert_fails_with<CancellationException>([&]() {
            runBlocking(job, [&]() {
                // TODO: suspend function
                expectUnreached();
            });
        });
    }

    /** Tests that the delayed tasks scheduled on a closed `runBlocking` event loop get processed in reasonable time. */
    // @Test
    // TODO: Convert test annotation
    void test_rescheduling_delayed_tasks() {
        auto job = runBlocking([&]() {
            // TODO: suspend function
            auto* dispatcher = coroutine_context()[ContinuationInterceptor];
            return GlobalScope::launch(dispatcher, [&]() {
                // TODO: suspend function
                delay(1 /* milliseconds */);
            });
        });
        runBlocking([&]() {
            // TODO: suspend function
            with_timeout(10'000 /* seconds */, [&]() {
                // TODO: suspend function
                job.join();
            });
        });
    }
};

} // namespace coroutines
} // namespace kotlinx
