// Original file: kotlinx-coroutines-test/common/test/RunTestTest.kt
// TODO: Remove or convert import statements
// TODO: Convert @Test annotations to appropriate test framework
// TODO: Convert all suspend functions (runTest, withContext, delay, etc.)
// TODO: Handle TestResult type and testResultMap helper
// TODO: Convert @NoJs, @NoNative, @NoWasmWasi, @NoWasmJs, @Ignore annotations
// TODO: Convert assertFailsWith, assertIs, assertContains, assertSame test assertions
// TODO: Handle suppressedExceptions (platform-specific)

namespace kotlinx {
    namespace coroutines {
        namespace test {
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlinx.coroutines.internal.*
            // TODO: import kotlinx.coroutines.flow.*
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlin.coroutines.*
            // TODO: import kotlin.test.*
            // TODO: import kotlin.test.assertFailsWith
            // TODO: import kotlin.time.*
            // TODO: import kotlin.time.Duration.Companion.milliseconds

            class RunTestTest {
            public:
                /** Tests that [withContext] that sends work to other threads works in [runTest]. */
                // TODO: @Test
                void test_with_context_dispatching() {
                    // TODO: runTest is a suspend function
                    run_test([&]() {
                        int counter = 0;
                        with_context(Dispatchers::default_dispatcher(), [&]() {
                            counter += 1;
                        });
                        assert(counter == 1);
                    });
                }

                /** Tests that joining [GlobalScope.launch] works in [runTest]. */
                // TODO: @Test
                void test_joining_forked_job() {
                    run_test([&]() {
                        int counter = 0;
                        auto job = GlobalScope::launch([&]() {
                            counter += 1;
                        });
                        job.join();
                        assert(counter == 1);
                    });
                }

                /** Tests [suspendCoroutine] not failing [runTest]. */
                // TODO: @Test
                void test_suspend_coroutine() {
                    run_test([&]() {
                        auto answer = suspend_coroutine<int>([&](auto it) {
                            it.resume(42);
                        });
                        assert(answer == 42);
                    });
                }

                /** Tests that [runTest] attempts to detect it being run inside another [runTest] and failing in such scenarios. */
                // TODO: @Test
                void test_nested_run_test_forbidden() {
                    run_test([&]() {
                        // TODO: assertFailsWith<IllegalStateException>
                        try {
                            run_test([]() {
                            });
                            assert(false && "should have thrown");
                        } catch (const std::invalid_argument &e) {
                            // expected
                        }
                    });
                }

                /** Tests that even the dispatch timeout of `0` is fine if all the dispatches go through the same scheduler. */
                // TODO: @Test
                void test_run_test_with_zero_dispatch_timeout_with_controlled_dispatches() {
                    run_test(0, [&]() {
                        // dispatchTimeoutMs = 0
                        // below is some arbitrary concurrent code where all dispatches go through the same scheduler.
                        launch([&]() {
                            delay(2000);
                        });
                        auto deferred = async_coro([&]() {
                            auto job = launch(StandardTestDispatcher(test_scheduler), [&]() {
                                launch([&]() {
                                    delay(500);
                                });
                                delay(1000);
                            });
                            job.join();
                        });
                        deferred.await();
                    });
                }

                /** Tests that too low of a dispatch timeout causes crashes. */
                // TODO: @Test
                void test_run_test_with_small_dispatch_timeout() {
                    test_result_map([](auto fn) {
                                        try {
                                            fn();
                                            fail("shouldn't be reached");
                                        } catch (const UncompletedCoroutinesError &e) {
                                            // expected
                                        }
                                    }, []() {
                                        return run_test(100, [&]() {
                                            // dispatchTimeoutMs = 100
                                            with_context(Dispatchers::default_dispatcher(), [&]() {
                                                delay(10000);
                                                return 3;
                                            });
                                            fail("shouldn't be reached");
                                        });
                                    });
                }

                /**
     * Tests that [runTest] times out after the specified time.
     */
                // TODO: @Test
                void test_run_test_with_small_timeout() {
                    test_result_map([](auto fn) {
                                        try {
                                            fn();
                                            fail("shouldn't be reached");
                                        } catch (const UncompletedCoroutinesError &e) {
                                            // expected
                                        }
                                    }, []() {
                                        return run_test(std::chrono::milliseconds(100), [&]() {
                                            // timeout
                                            with_context(Dispatchers::default_dispatcher(), [&]() {
                                                delay(10000);
                                                return 3;
                                            });
                                            fail("shouldn't be reached");
                                        });
                                    });
                }

                /** Tests that [runTest] times out after the specified time, even if the test framework always knows the test is
     * still doing something. */
                // TODO: @Test
                void test_run_test_with_small_timeout_and_many_dispatches() {
                    test_result_map([](auto fn) {
                                        try {
                                            fn();
                                            fail("shouldn't be reached");
                                        } catch (const UncompletedCoroutinesError &e) {
                                            // expected
                                        }
                                    }, []() {
                                        return run_test(std::chrono::milliseconds(100), [&]() {
                                            while (true) {
                                                with_context(Dispatchers::default_dispatcher(), [&]() {
                                                    delay(10);
                                                    return 3;
                                                });
                                            }
                                        });
                                    });
                }

                /** Tests that, on timeout, the names of the active coroutines are listed,
     * whereas the names of the completed ones are not. */
                // TODO: @Test
                // TODO: @NoJs
                // TODO: @NoNative
                // TODO: @NoWasmWasi
                // TODO: @NoWasmJs
                TestResult test_listing_active_coroutines_on_timeout() {
                    std::string name1 = "GoodUniqueName";
                    std::string name2 = "BadUniqueName";
                    return test_result_map([&](auto it) {
                                               try {
                                                   it();
                                                   fail("unreached");
                                               } catch (const UncompletedCoroutinesError &e) {
                                                   // TODO: assertContains
                                                   assert(std::string(e.what()).find(name1) != std::string::npos);
                                                   assert(std::string(e.what()).find(name2) == std::string::npos);
                                               }
                                           }, [&]() {
                                               return run_test(10, [&]() {
                                                   // dispatchTimeoutMs = 10
                                                   launch(CoroutineName(name1), [&]() {
                                                       CompletableDeferred<void>().await();
                                                   });
                                                   launch(CoroutineName(name2), [&]() {
                                                   });
                                               });
                                           });
                }

                /** Tests that the [UncompletedCoroutinesError] suppresses an exception with which the coroutine is completing. */
                // TODO: @Test
                void test_failure_with_pending_coroutine() {
                    test_result_map([](auto it) {
                                        try {
                                            it();
                                            fail("unreached");
                                        } catch (const UncompletedCoroutinesError &e) {
                                            // TODO: @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
                                            auto suppressed = unwrap(e).suppressed_exceptions();
                                            assert(suppressed.size() == 1);
                                            auto *test_ex = dynamic_cast<TestException *>(suppressed[0]);
                                            assert(test_ex != nullptr);
                                            assert(std::string(test_ex->what()) == "A");
                                        }
                                    }, []() {
                                        return run_test(std::chrono::milliseconds(10), [&]() {
                                            launch(CoroutineStart::kUndispatched, [&]() {
                                                with_context(NonCancellable + Dispatchers::default_dispatcher(), [&]() {
                                                    delay(std::chrono::milliseconds(100));
                                                });
                                            });
                                            throw TestException("A");
                                        });
                                    });
                }

                /** Tests that real delays can be accounted for with a large enough dispatch timeout. */
                // TODO: @Test
                void test_run_test_with_large_dispatch_timeout() {
                    run_test(5000, [&]() {
                        // dispatchTimeoutMs = 5000
                        with_context(Dispatchers::default_dispatcher(), [&]() {
                            delay(50);
                        });
                    });
                }

                /** Tests that delays can be accounted for with a large enough timeout. */
                // TODO: @Test
                void test_run_test_with_large_timeout() {
                    run_test(std::chrono::milliseconds(5000), [&]() {
                        with_context(Dispatchers::default_dispatcher(), [&]() {
                            delay(50);
                        });
                    });
                }

                /** Tests uncaught exceptions being suppressed by the dispatch timeout error. */
                // TODO: @Test
                void test_run_test_timing_out_and_throwing() {
                    test_result_map([](auto fn) {
                                        try {
                                            fn();
                                            fail("unreached");
                                        } catch (const UncompletedCoroutinesError &e) {
                                            // TODO: @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
                                            auto suppressed = unwrap(e).suppressed_exceptions();
                                            assert(suppressed.size() == 1);
                                            auto *test_ex = dynamic_cast<TestException *>(suppressed[0]);
                                            assert(test_ex != nullptr);
                                            assert(std::string(test_ex->what()) == "A");
                                        }
                                    }, []() {
                                        return run_test(std::chrono::milliseconds(100), [&]() {
                                            coroutine_context[CoroutineExceptionHandler].handle_exception(
                                                coroutine_context, TestException("A")
                                            );
                                            with_context(Dispatchers::default_dispatcher(), [&]() {
                                                delay(10000);
                                                return 3;
                                            });
                                            fail("shouldn't be reached");
                                        });
                                    });
                }

                /** Tests that passing invalid contexts to [runTest] causes it to fail (on JS, without forking). */
                // TODO: @Test
                void test_run_test_with_illegal_context() {
                    for (auto &ctx: TestScopeTest::invalid_contexts) {
                        // TODO: assertFailsWith<IllegalArgumentException>
                        try {
                            run_test(ctx, []() {
                            });
                            assert(false && "should have thrown");
                        } catch (const std::invalid_argument &e) {
                            // expected
                        }
                    }
                }

                /** Tests that throwing exceptions in [runTest] fails the test with them. */
                // TODO: @Test
                void test_throwing_in_run_test_body() {
                    test_result_map([](auto it) {
                                        // TODO: assertFailsWith<RuntimeException>
                                        try {
                                            it();
                                            assert(false && "should have thrown");
                                        } catch (const std::runtime_error &e) {
                                            // expected
                                        }
                                    }, []() {
                                        return run_test([&]() {
                                            throw std::runtime_error("");
                                        });
                                    });
                }

                /** Tests that throwing exceptions in pending tasks [runTest] fails the test with them. */
                // TODO: @Test
                void test_throwing_in_run_test_pending_task() {
                    test_result_map([](auto it) {
                                        try {
                                            it();
                                            assert(false && "should have thrown");
                                        } catch (const std::runtime_error &e) {
                                            // expected
                                        }
                                    }, []() {
                                        return run_test([&]() {
                                            launch([&]() {
                                                delay(kSlow);
                                                throw std::runtime_error("");
                                            });
                                        });
                                    });
                }

                // TODO: @Test
                void reproducer2405() {
                    run_test([&]() {
                        auto dispatcher = StandardTestDispatcher(test_scheduler);
                        bool collected_error = false;
                        with_context(dispatcher, [&]() {
                            auto f = flow([&]() { emit(1); })
                                    .combine(
                                        flow<std::string>([&]() { throw std::invalid_argument(""); }),
                                        [](int int_val, std::string str_val) {
                                            return std::to_string(int_val) + str_val;
                                        }
                                    )
                                    .catch_exception([&](auto e) { emit("error"); })
                                    .collect([&](auto it) {
                                        assert(it == "error");
                                        collected_error = true;
                                    });
                        });
                        assert(collected_error);
                    });
                }

                /** Tests that, once the test body has thrown, the child coroutines are cancelled. */
                // TODO: @Test
                TestResult test_children_cancellation_on_test_body_failure() {
                    Job *job = nullptr;
                    return test_result_map([&](auto it) {
                                               // TODO: assertFailsWith<AssertionError>
                                               try {
                                                   it();
                                                   assert(false && "should have thrown");
                                               } catch (const std::exception &e) {
                                                   // expected
                                               }
                                               assert(job->is_cancelled());
                                           }, [&]() {
                                               return run_test([&]() {
                                                   job = launch([&]() {
                                                       while (true) {
                                                           delay(1000);
                                                       }
                                                   });
                                                   throw std::runtime_error("assertion error");
                                               });
                                           });
                }

                /** Tests that [runTest] reports [TimeoutCancellationException]. */
                // TODO: @Test
                void test_timeout() {
                    test_result_map([](auto it) {
                                        // TODO: assertFailsWith<TimeoutCancellationException>
                                        try {
                                            it();
                                            assert(false && "should have thrown");
                                        } catch (const TimeoutCancellationException &e) {
                                            // expected
                                        }
                                    }, []() {
                                        return run_test([&]() {
                                            with_timeout(50, [&]() {
                                                launch([&]() {
                                                    delay(1000);
                                                });
                                            });
                                        });
                                    });
                }

                /** Checks that [runTest] throws the root cause and not [JobCancellationException] when a child coroutine throws. */
                // TODO: @Test
                void test_run_test_throws_root_cause() {
                    test_result_map([](auto it) {
                                        // TODO: assertFailsWith<TestException>
                                        try {
                                            it();
                                            assert(false && "should have thrown");
                                        } catch (const TestException &e) {
                                            // expected
                                        }
                                    }, []() {
                                        return run_test([&]() {
                                            launch([&]() {
                                                throw TestException();
                                            });
                                        });
                                    });
                }

                /** Tests that [runTest] completes its job. */
                // TODO: @Test
                TestResult test_completes_own_job() {
                    bool handler_called = false;
                    return test_result_map([&](auto it) {
                                               it();
                                               assert(handler_called);
                                           }, [&]() {
                                               return run_test([&]() {
                                                   coroutine_context.job.invoke_on_completion([&]() {
                                                       handler_called = true;
                                                   });
                                               });
                                           });
                }

                /** Tests that [runTest] doesn't complete the job that was passed to it as an argument. */
                // TODO: @Test
                TestResult test_does_not_complete_given_job() {
                    bool handler_called = false;
                    auto job = Job();
                    job.invoke_on_completion([&]() {
                        handler_called = true;
                    });
                    return test_result_map([&](auto it) {
                                               it();
                                               assert(!handler_called);
                                               auto active_count = std::count_if(
                                                   job.children.begin(), job.children.end(),
                                                   [](auto &child) { return child.is_active(); }
                                               );
                                               assert(active_count == 0);
                                           }, [&]() {
                                               return run_test(job, [&]() {
                                                   // TODO: assertTrue(coroutineContext.job in job.children)
                                                   assert(std::find(job.children.begin(), job.children.end(),
                                                                    coroutine_context.job) != job.children.end());
                                               });
                                           });
                }

                /** Tests that, when the test body fails, the reported exceptions are suppressed. */
                // TODO: @Test
                void test_suppressed_exceptions() {
                    test_result_map([](auto it) {
                                        try {
                                            it();
                                            fail("should not be reached");
                                        } catch (const TestException &e) {
                                            assert(std::string(e.what()) == "w");
                                            auto suppressed = e.suppressed_exceptions();
                                            auto all_suppressed = suppressed;
                                            if (!suppressed.empty() && suppressed[0]->suppressed_exceptions().size() >
                                                0) {
                                                auto nested = suppressed[0]->suppressed_exceptions();
                                                all_suppressed.insert(all_suppressed.end(), nested.begin(),
                                                                      nested.end());
                                            }
                                            assert(all_suppressed.size() == 3);
                                            assert(std::string(all_suppressed[0]->what()) == "x");
                                            assert(std::string(all_suppressed[1]->what()) == "y");
                                            assert(std::string(all_suppressed[2]->what()) == "z");
                                        }
                                    }, []() {
                                        return run_test([&]() {
                                            launch(SupervisorJob(), [&]() { throw TestException("x"); });
                                            launch(SupervisorJob(), [&]() { throw TestException("y"); });
                                            launch(SupervisorJob(), [&]() { throw TestException("z"); });
                                            throw TestException("w");
                                        });
                                    });
                }

                /** Tests that [TestScope.runTest] does not inherit the exception handler and works. */
                // TODO: @Test
                TestResult test_scope_run_test_exception_handler() {
                    auto scope = TestScope();
                    return test_result_map([](auto it) {
                                               try {
                                                   it();
                                                   fail("should not be reached");
                                               } catch (const TestException &e) {
                                                   // expected
                                               }
                                           }, [&]() {
                                               return scope.run_test([&]() {
                                                   launch(SupervisorJob(), [&]() { throw TestException("x"); });
                                               });
                                           });
                }

                /**
     * Tests that if the main coroutine is completed without a dispatch, [runTest] will not consider this to be
     * inactivity.
     *
     * The test will hang if this is not case.
     */
                // TODO: @Test
                void test_coroutine_completing_without_dispatch() {
                    run_test(Duration::kInfinite, [&]() {
                        launch(Dispatchers::default_dispatcher(), [&]() { delay(100); });
                    });
                }

                /**
     * Tests that [runTest] cleans up the exception handler even if it threw on initialization.
     *
     * This test must be run manually, because it writes garbage to the log.
     *
     * The JVM-only source set contains a test equivalent to this one that isn't ignored.
     */
                // TODO: @Test
                // TODO: @Ignore
                TestResult test_exception_captor_cleaned_up_on_preliminary_exit() {
                    return test_result_chain(
                        []() {
                            // step 1: installing the exception handler
                            std::cout << "step 1" << std::endl;
                            return run_test([]() {
                            });
                        },
                        [](auto it) {
                            it.get_or_throw();
                            // step 2: throwing an uncaught exception to be caught by the exception-handling system
                            std::cout << "step 2" << std::endl;
                            return create_test_result([&]() {
                                launch(NonCancellable, [&]() { throw TestException("A"); });
                            });
                        },
                        [](auto it) {
                            it.get_or_throw();
                            // step 3: trying to run a test should immediately fail, even before entering the test body
                            std::cout << "step 3" << std::endl;
                            try {
                                run_test([&]() {
                                    fail("unreached");
                                });
                                fail("unreached");
                            } catch (const UncaughtExceptionsBeforeTest &e) {
                                auto cause = e.suppressed_exceptions()[0];
                                auto *test_ex = dynamic_cast<TestException *>(cause);
                                assert(test_ex != nullptr);
                                assert(std::string(test_ex->what()) == "A");
                            }
                            // step 4: trying to run a test again should not fail with an exception
                            std::cout << "step 4" << std::endl;
                            return run_test([]() {
                            });
                        },
                        [](auto it) {
                            it.get_or_throw();
                            // step 5: throwing an uncaught exception to be caught by the exception-handling system, again
                            std::cout << "step 5" << std::endl;
                            return create_test_result([&]() {
                                launch(NonCancellable, [&]() { throw TestException("B"); });
                            });
                        },
                        [](auto it) {
                            it.get_or_throw();
                            // step 6: trying to run a test should immediately fail, again
                            std::cout << "step 6" << std::endl;
                            try {
                                run_test([&]() {
                                    fail("unreached");
                                });
                                fail("unreached");
                            } catch (const std::exception &e) {
                                auto cause = e.suppressed_exceptions()[0];
                                auto *test_ex = dynamic_cast<TestException *>(cause);
                                assert(test_ex != nullptr);
                                assert(std::string(test_ex->what()) == "B");
                            }
                            // step 7: trying to run a test again should not fail with an exception, again
                            std::cout << "step 7" << std::endl;
                            return run_test([]() {
                            });
                        }
                    );
                }

                // TODO: @Test
                void test_cancelling_test_scope() {
                    test_result_map([](auto it) {
                                        try {
                                            it();
                                            fail("unreached");
                                        } catch (const CancellationException &e) {
                                            // expected
                                        }
                                    }, []() {
                                        return run_test([&]() {
                                            cancel(CancellationException("Oh no", TestException()));
                                        });
                                    });
                }
            };
        } // namespace test
    } // namespace coroutines
} // namespace kotlinx