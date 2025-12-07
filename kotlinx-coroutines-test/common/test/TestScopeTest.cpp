// Original file: kotlinx-coroutines-test/common/test/TestScopeTest.kt
// TODO: Remove or convert import statements
// TODO: Convert @Test, @Ignore annotations to appropriate test framework
// TODO: Convert all suspend functions (runTest, delay, launch, etc.)
// TODO: Handle TestResult type and testResultMap helper
// TODO: Convert companion object with invalidContexts list
// TODO: Convert Flow operations (stateIn, SharingStarted, produce, etc.)
// TODO: Convert assertFailsWith, assertIs, assertContains, assertSame test assertions

namespace kotlinx {
namespace coroutines {
namespace test {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.channels.*
// TODO: import kotlinx.coroutines.flow.*
// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.test.*
// TODO: import kotlin.test.assertFailsWith
// TODO: import kotlin.time.Duration.Companion.milliseconds

class TestScopeTest {
public:
    /** Tests failing to create a [TestScope] with incorrect contexts. */
    // TODO: @Test
    void test_create_throws_on_invalid_arguments() {
        for (auto& ctx : invalid_contexts) {
            // TODO: assertFailsWith<IllegalArgumentException>
            try {
                TestScope(ctx);
                assert(false && "should have thrown");
            } catch (const std::invalid_argument& e) {
                // expected
            }
        }
    }

    /** Tests that a newly-created [TestScope] provides the correct scheduler. */
    // TODO: @Test
    void test_create_provides_scheduler() {
        // Creates a new scheduler.
        {
            auto scope = TestScope();
            assert(scope.coroutine_context[TestCoroutineScheduler] != nullptr);
        }
        // Reuses the scheduler that the dispatcher is linked to.
        {
            auto dispatcher = StandardTestDispatcher();
            auto scope = TestScope(dispatcher);
            // TODO: assertSame
            assert(dispatcher.scheduler == scope.coroutine_context[TestCoroutineScheduler]);
        }
        // Uses the scheduler passed to it.
        {
            auto scheduler = TestCoroutineScheduler();
            auto scope = TestScope(scheduler);
            // TODO: assertSame
            assert(scheduler == scope.coroutine_context[TestCoroutineScheduler]);
            auto* test_dispatcher = dynamic_cast<TestDispatcher*>(
                scope.coroutine_context[ContinuationInterceptor]
            );
            assert(scheduler == test_dispatcher->scheduler);
        }
        // Doesn't touch the passed dispatcher and the scheduler if they match.
        {
            auto scheduler = TestCoroutineScheduler();
            auto dispatcher = StandardTestDispatcher(scheduler);
            auto scope = TestScope(scheduler + dispatcher);
            // TODO: assertSame
            assert(scheduler == scope.coroutine_context[TestCoroutineScheduler]);
            assert(dispatcher == scope.coroutine_context[ContinuationInterceptor]);
        }
    }

    /** Part of [testCreateProvidesScheduler], disabled for Native */
    // TODO: @Test
    void test_create_reuses_scheduler() {
        // Reuses the scheduler of `Dispatchers.Main`
        {
            auto scheduler = TestCoroutineScheduler();
            auto main_dispatcher = StandardTestDispatcher(scheduler);
            Dispatchers::set_main(main_dispatcher);
            try {
                auto scope = TestScope();
                // TODO: assertSame
                assert(scheduler == scope.coroutine_context[TestCoroutineScheduler]);
                // TODO: assertNotSame
                assert(main_dispatcher != scope.coroutine_context[ContinuationInterceptor]);
            } finally {
                Dispatchers::reset_main();
            }
        }
        // Does not reuse the scheduler of `Dispatchers.Main` if one is explicitly passed
        {
            auto main_dispatcher = StandardTestDispatcher();
            Dispatchers::set_main(main_dispatcher);
            try {
                auto scheduler = TestCoroutineScheduler();
                auto scope = TestScope(scheduler);
                // TODO: assertSame
                assert(scheduler == scope.coroutine_context[TestCoroutineScheduler]);
                // TODO: assertNotSame
                assert(main_dispatcher.scheduler != scope.coroutine_context[TestCoroutineScheduler]);
                assert(main_dispatcher != scope.coroutine_context[ContinuationInterceptor]);
            } finally {
                Dispatchers::reset_main();
            }
        }
    }

    /** Tests that the cleanup procedure throws if there were uncompleted delays by the end. */
    // TODO: @Test
    void test_present_delays_throwing() {
        auto scope = TestScope();
        bool result = false;
        scope.launch([&]() {
            delay(5);
            result = true;
        });
        assert(!result);
        scope.as_specific_implementation().enter();
        // TODO: assertFailsWith<UncompletedCoroutinesError>
        try {
            scope.as_specific_implementation().legacy_leave();
            assert(false && "should have thrown");
        } catch (const UncompletedCoroutinesError& e) {
            // expected
        }
        assert(!result);
    }

    /** Tests that the cleanup procedure throws if there were active jobs by the end. */
    // TODO: @Test
    void test_active_jobs_throwing() {
        auto scope = TestScope();
        bool result = false;
        auto deferred = CompletableDeferred<std::string>();
        scope.launch([&]() {
            deferred.await();
            result = true;
        });
        assert(!result);
        scope.as_specific_implementation().enter();
        // TODO: assertFailsWith<UncompletedCoroutinesError>
        try {
            scope.as_specific_implementation().legacy_leave();
            assert(false && "should have thrown");
        } catch (const UncompletedCoroutinesError& e) {
            // expected
        }
        assert(!result);
    }

    /** Tests that the cleanup procedure throws even if it detects that the job is already cancelled. */
    // TODO: @Test
    void test_cancelled_delays_throwing() {
        auto scope = TestScope();
        bool result = false;
        auto deferred = CompletableDeferred<std::string>();
        auto job = scope.launch([&]() {
            deferred.await();
            result = true;
        });
        job.cancel();
        assert(!result);
        scope.as_specific_implementation().enter();
        // TODO: assertFailsWith<UncompletedCoroutinesError>
        try {
            scope.as_specific_implementation().legacy_leave();
            assert(false && "should have thrown");
        } catch (const UncompletedCoroutinesError& e) {
            // expected
        }
        assert(!result);
    }

    /** Tests that uncaught exceptions are thrown at the cleanup. */
    // TODO: @Test
    TestResult test_gets_cancelled_on_child_failure() {
        auto scope = TestScope();
        auto exception = TestException("test");
        scope.launch([&]() {
            throw exception;
        });
        return test_result_map([](auto it) {
            try {
                it();
                fail("should not reach");
            } catch (const TestException& e) {
                // expected
            }
        }, [&]() {
            return scope.run_test([]() {
            });
        });
    }

    /** Tests that, when reporting several exceptions, the first one is thrown, with the rest suppressed. */
    // TODO: @Test
    void test_suppressed_exceptions() {
        TestScope scope;
        scope.as_specific_implementation().enter();
        scope.launch(SupervisorJob(), [&]() { throw TestException("x"); });
        scope.launch(SupervisorJob(), [&]() { throw TestException("y"); });
        scope.launch(SupervisorJob(), [&]() { throw TestException("z"); });
        scope.run_current();
        auto e = scope.as_specific_implementation().legacy_leave();
        assert(e.size() == 3);
        assert(std::string(e[0]->what()) == "x");
        assert(std::string(e[1]->what()) == "y");
        assert(std::string(e[2]->what()) == "z");
    }

    /** Tests that the background work is being run at all. */
    // TODO: @Test
    TestResult test_background_work_being_run() {
        return run_test([&]() {
            int i = 0;
            int j = 0;
            background_scope.launch([&]() {
                ++i;
            });
            background_scope.launch([&]() {
                delay(10);
                ++j;
            });
            assert(i == 0);
            assert(j == 0);
            delay(1);
            assert(i == 1);
            assert(j == 0);
            delay(10);
            assert(i == 1);
            assert(j == 1);
        });
    }

    /**
     * Tests that the background work gets cancelled after the test body finishes.
     */
    // TODO: @Test
    TestResult test_background_work_cancelled() {
        bool cancelled = false;
        return test_result_map([&](auto it) {
            it();
            assert(cancelled);
        }, [&]() {
            return run_test([&]() {
                int i = 0;
                background_scope.launch([&]() {
                    try {
                        while (is_active) {
                            ++i;
                            yield();
                        }
                    } catch (const CancellationException& e) {
                        cancelled = true;
                    }
                });
                for (int it = 0; it < 5; ++it) {
                    assert(i == it);
                    yield();
                }
            });
        });
    }

    /** Tests the interactions between the time-control commands and the background work. */
    // TODO: @Test
    TestResult test_background_work_time_control() {
        return run_test([&]() {
            int i = 0;
            int j = 0;
            background_scope.launch([&]() {
                while (true) {
                    ++i;
                    delay(100);
                }
            });
            background_scope.launch([&]() {
                while (true) {
                    ++j;
                    delay(50);
                }
            });
            advance_until_idle(); // should do nothing, as only background work is left.
            assert(i == 0);
            assert(j == 0);
            auto job = launch([&]() {
                delay(1);
                // the background work scheduled for earlier gets executed before the normal work scheduled for later does
                assert(i == 1);
                assert(j == 1);
            });
            job.join();
            advance_time_by(std::chrono::milliseconds(199)); // should work the same for the background tasks
            assert(i == 2);
            assert(j == 4);
            advance_until_idle(); // once again, should do nothing
            assert(i == 2);
            assert(j == 4);
            run_current(); // should behave the same way as for the normal work
            assert(i == 3);
            assert(j == 5);
            launch([&]() {
                delay(1001);
                assert(i == 13);
                assert(j == 25);
            });
            advance_until_idle(); // should execute the normal work, and with that, the background one, too
        });
    }

    /**
     * Tests that an error in a background coroutine does not cancel the test, but is reported at the end.
     */
    // TODO: @Test
    TestResult test_background_work_error_reporting() {
        bool test_finished = false;
        auto exception = std::runtime_error("x");
        return test_result_map([&](auto it) {
            try {
                it();
                fail("unreached");
            } catch (const std::exception& e) {
                // TODO: assertSame
                // assert(&e == &exception);
                assert(test_finished);
            }
        }, [&]() {
            return run_test([&]() {
                background_scope.launch([&]() {
                    throw exception;
                });
                delay(1000);
                test_finished = true;
            });
        });
    }

    /**
     * Tests that the background work gets to finish what it's doing after the test is completed.
     */
    // TODO: @Test
    TestResult test_background_work_finalizing() {
        int task_ended = 0;
        const int n_tasks = 10;
        return test_result_map([&](auto it) {
            try {
                it();
                fail("unreached");
            } catch (const TestException& e) {
                assert(e.suppressed_exceptions().size() == 2);
                assert(task_ended == n_tasks);
            }
        }, [&]() {
            return run_test([&]() {
                for (int i = 0; i < n_tasks; ++i) {
                    background_scope.launch([&]() {
                        try {
                            while (true) {
                                delay(1);
                            }
                        } finally {
                            ++task_ended;
                            if (task_ended <= 2) {
                                throw TestException();
                            }
                        }
                    });
                }
                delay(100);
                throw TestException();
            });
        });
    }

    /**
     * Tests using [Flow.stateIn] as a background job.
     */
    // TODO: @Test
    TestResult test_example_background_job1() {
        return run_test([&]() {
            auto my_flow = flow([&]() {
                int i = 0;
                while (true) {
                    emit(++i);
                    delay(1);
                }
            });
            auto state_flow = my_flow.state_in(background_scope, SharingStarted::kEagerly, 0);
            int j = 0;
            for (int it = 0; it < 100; ++it) {
                assert(j++ == state_flow.value());
                delay(1);
            }
        });
    }

    /**
     * A test from the documentation of [TestScope.backgroundScope].
     */
    // TODO: @Test
    TestResult test_example_background_job2() {
        return run_test([&]() {
            Channel<int> channel;
            background_scope.launch([&]() {
                int i = 0;
                while (true) {
                    channel.send(i++);
                }
            });
            for (int it = 0; it < 100; ++it) {
                assert(it == channel.receive());
            }
        });
    }

    /**
     * Tests that the test will timeout due to idleness even if some background tasks are running.
     */
    // TODO: @Test
    TestResult test_background_work_not_preventing_timeout() {
        return test_result_map([](auto it) {
            try {
                it();
                fail("unreached");
            } catch (const UncompletedCoroutinesError& e) {
                // expected
            }
        }, []() {
            return run_test(std::chrono::milliseconds(100), [&]() { // timeout
                background_scope.launch([&]() {
                    while (true) {
                        yield();
                    }
                });
                background_scope.launch([&]() {
                    while (true) {
                        delay(1);
                    }
                });
                auto deferred = CompletableDeferred<void>();
                deferred.await();
            });
        });
    }

    /**
     * Tests that the background work will not prevent the test from timing out even in some cases
     * when the unconfined dispatcher is used.
     */
    // TODO: @Test
    TestResult test_unconfined_background_work_not_preventing_timeout() {
        return test_result_map([](auto it) {
            try {
                it();
                fail("unreached");
            } catch (const UncompletedCoroutinesError& e) {
                // expected
            }
        }, []() {
            return run_test(UnconfinedTestDispatcher(), std::chrono::milliseconds(100), [&]() {
                /**
                 * Having a coroutine like this will still cause the test to hang:
                     background_scope.launch {
                         while (true) {
                             yield()
                         }
                     }
                 * The reason is that even the initial [advanceUntilIdle] will never return in this case.
                 */
                background_scope.launch([&]() {
                    while (true) {
                        delay(1);
                    }
                });
                auto deferred = CompletableDeferred<void>();
                deferred.await();
            });
        });
    }

    /**
     * Tests that even the exceptions in the background scope that don't typically get reported and need to be queried
     * (like failures in [async]) will still surface in some simple scenarios.
     */
    // TODO: @Test
    TestResult test_async_failure_in_background_reported() {
        return test_result_map([](auto it) {
            try {
                it();
                fail("unreached");
            } catch (const TestException& e) {
                assert(std::string(e.what()) == "z");
                auto suppressed = e.suppressed_exceptions();
                std::set<std::string> messages;
                for (auto* ex : suppressed) {
                    messages.insert(ex->what());
                }
                assert(messages == std::set<std::string>{"x", "y"});
            }
        }, []() {
            return run_test([&]() {
                background_scope.async_coro([&]() {
                    throw TestException("x");
                });
                background_scope.produce<void>([&]() {
                    throw TestException("y");
                });
                delay(1);
                throw TestException("z");
            });
        });
    }

    /**
     * Tests that, if an exception reaches the [TestScope] exception reporting mechanism via several
     * channels, it will only be reported once.
     */
    // TODO: @Test
    TestResult test_no_duplicate_exceptions() {
        return test_result_map([](auto it) {
            try {
                it();
                fail("unreached");
            } catch (const TestException& e) {
                assert(std::string(e.what()) == "y");
                auto suppressed = e.suppressed_exceptions();
                std::vector<std::string> messages;
                for (auto* ex : suppressed) {
                    messages.push_back(ex->what());
                }
                assert(messages == std::vector<std::string>{"x"});
            }
        }, []() {
            return run_test([&]() {
                background_scope.launch([&]() {
                    throw TestException("x");
                });
                delay(1);
                throw TestException("y");
            });
        });
    }

    /**
     * Tests that [TestScope.withTimeout] notifies the programmer about using the virtual time.
     */
    // TODO: @Test
    TestResult test_timing_out_with_virtual_time_message() {
        return run_test([&]() {
            try {
                with_timeout(1000000, [&]() {
                    Channel<void>().receive();
                });
            } catch (const TimeoutCancellationException& e) {
                // TODO: assertContains
                assert(std::string(e.what()).find("virtual") != std::string::npos);
            }
        });
    }

    /*
     * Tests that the [TestScope] exception reporting mechanism will report the exceptions that happen between
     * different tests.
     *
     * This test must be run manually, because such exceptions still go through the global exception handler
     * (as there's no guarantee that another test will happen), and the global exception handler will
     * log the exceptions or, on Native, crash the test suite.
     *
     * The JVM-only source set contains a test equivalent to this one that isn't ignored.
     */
    // TODO: @Test
    // TODO: @Ignore
    void test_reporting_stray_uncaught_exceptions_between_tests() {
        auto thrown = TestException("x");
        test_result_chain(
            []() {
                // register a handler for uncaught exceptions
                return run_test([]() {});
            },
            [&](auto it) {
                GlobalScope::launch(CoroutineStart::kUndispatched, [&]() {
                    throw thrown;
                });
                return run_test([&]() {
                    fail("unreached");
                });
            },
            [&](auto it) {
                // this `runTest` will not report the exception
                return run_test([&]() {
                    auto exception = it.exception_or_null();
                    if (auto* uncaught = dynamic_cast<UncaughtExceptionsBeforeTest*>(exception)) {
                        assert(uncaught->suppressed_exceptions().size() == 1);
                        // TODO: assertSame
                        // assert(uncaught->suppressed_exceptions()[0] == &thrown);
                    } else {
                        fail(std::string("unexpected exception: ") + (exception ? exception->what() : "null"));
                    }
                });
            }
        );
    }

    /**
     * Tests that the uncaught exceptions that happen during the test are reported.
     */
    // TODO: @Test
    TestResult test_reporting_stray_uncaught_exceptions_during_test() {
        auto thrown = TestException("x");
        return test_result_chain(
            [&](auto _) {
                return run_test([&]() {
                    auto job = launch(Dispatchers::default_dispatcher() + NonCancellable, [&]() {
                        throw thrown;
                    });
                    job.join();
                });
            },
            [&](auto it) {
                return run_test([&]() {
                    // TODO: assertEquals
                    assert(thrown == it.exception_or_null());
                });
            }
        );
    }

    // TODO: companion object
    static std::vector<CoroutineContext> invalid_contexts;
};

// Static member initialization
std::vector<CoroutineContext> TestScopeTest::invalid_contexts = {
    Dispatchers::default_dispatcher(), // not a [TestDispatcher]
    CoroutineExceptionHandler([](auto, auto) {}), // exception handlers can't be overridden
    StandardTestDispatcher() + TestCoroutineScheduler(), // the dispatcher is not linked to the scheduler
};

} // namespace test
} // namespace coroutines
} // namespace kotlinx
