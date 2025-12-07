// Original file: kotlinx-coroutines-test/common/test/TestCoroutineSchedulerTest.kt
// TODO: Remove or convert import statements
// TODO: Convert @Test annotations to appropriate test framework
// TODO: Convert all suspend functions (runTest, delay, launch, etc.)
// TODO: Convert assertFailsWith test assertions
// TODO: Handle Duration companion object extensions

namespace kotlinx {
namespace coroutines {
namespace test {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.test.*
// TODO: import kotlin.test.assertFailsWith
// TODO: import kotlin.time.*
// TODO: import kotlin.time.Duration.Companion.seconds
// TODO: import kotlin.time.Duration.Companion.milliseconds

class TestCoroutineSchedulerTest {
public:
    /** Tests that `TestCoroutineScheduler` attempts to detect if there are several instances of it. */
    // TODO: @Test
    void test_context_element() {
        run_test([&]() {
            // TODO: assertFailsWith<IllegalStateException>
            try {
                with_context(StandardTestDispatcher(), [&]() {
                });
                assert(false && "should have thrown");
            } catch (const std::invalid_argument& e) {
                // expected
            }
        });
    }

    /** Tests that, as opposed to [DelayController.advanceTimeBy] or [TestCoroutineScope.advanceTimeBy],
     * [TestCoroutineScheduler.advanceTimeBy] doesn't run the tasks scheduled at the target moment. */
    // TODO: @Test
    void test_advance_time_by_does_not_run_current() {
        run_test([&]() {
            bool entered = false;
            launch([&]() {
                delay(15);
                entered = true;
            });
            test_scheduler.advance_time_by(std::chrono::milliseconds(15));
            assert(!entered);
            test_scheduler.run_current();
            assert(entered);
        });
    }

    /** Tests that [TestCoroutineScheduler.advanceTimeBy] doesn't accept negative delays. */
    // TODO: @Test
    void test_advance_time_by_with_negative_delay() {
        auto scheduler = TestCoroutineScheduler();
        // TODO: assertFailsWith<IllegalArgumentException>
        try {
            scheduler.advance_time_by(std::chrono::milliseconds(-1));
            assert(false && "should have thrown");
        } catch (const std::invalid_argument& e) {
            // expected
        }
    }

    /** Tests that if [TestCoroutineScheduler.advanceTimeBy] encounters an arithmetic overflow, all the tasks scheduled
     * until the moment [Long.MAX_VALUE] get run. */
    // TODO: @Test
    void test_advance_time_by_enormous_delays() {
        for_test_dispatchers([&](TestDispatcher* it) {
            assert_runs_fast([&]() {
                TestScope scope(it);
                scope.launch([&]() {
                    const int64_t initial_delay = 10;
                    delay(initial_delay);
                    assert(scope.current_time == initial_delay);
                    bool entered_infinity = false;
                    launch([&]() {
                        delay(LLONG_MAX - 1); // delay(Long.MAX_VALUE) does nothing
                        assert(scope.current_time == LLONG_MAX);
                        entered_infinity = true;
                    });
                    bool entered_near_infinity = false;
                    launch([&]() {
                        delay(LLONG_MAX - initial_delay - 1);
                        assert(scope.current_time == LLONG_MAX - 1);
                        entered_near_infinity = true;
                    });
                    test_scheduler.advance_time_by(Duration::kInfinite);
                    assert(!entered_infinity);
                    assert(entered_near_infinity);
                    assert(scope.current_time == LLONG_MAX);
                    test_scheduler.run_current();
                    assert(entered_infinity);
                });
                scope.test_scheduler.advance_until_idle();
            });
        });
    }

    /** Tests the basic functionality of [TestCoroutineScheduler.advanceTimeBy]. */
    // TODO: @Test
    void test_advance_time_by() {
        run_test([&]() {
            assert_runs_fast([&]() {
                int stage = 1;
                launch([&]() {
                    delay(1000);
                    assert(current_time == 1000);
                    stage = 2;
                    delay(500);
                    assert(current_time == 1500);
                    stage = 3;
                    delay(501);
                    assert(current_time == 2001);
                    stage = 4;
                });
                assert(stage == 1);
                assert(current_time == 0);
                advance_time_by(std::chrono::seconds(2));
                assert(stage == 3);
                assert(current_time == 2000);
                advance_time_by(std::chrono::milliseconds(2));
                assert(stage == 4);
                assert(current_time == 2002);
            });
        });
    }

    /** Tests the basic functionality of [TestCoroutineScheduler.runCurrent]. */
    // TODO: @Test
    void test_run_current() {
        run_test([&]() {
            int stage = 0;
            launch([&]() {
                delay(1);
                ++stage;
                delay(1);
                stage += 10;
            });
            launch([&]() {
                delay(1);
                ++stage;
                delay(1);
                stage += 10;
            });
            test_scheduler.advance_time_by(std::chrono::milliseconds(1));
            assert(stage == 0);
            run_current();
            assert(stage == 2);
            test_scheduler.advance_time_by(std::chrono::milliseconds(1));
            assert(stage == 2);
            run_current();
            assert(stage == 22);
        });
    }

    /** Tests that [TestCoroutineScheduler.runCurrent] will not run new tasks after the current time has advanced. */
    // TODO: @Test
    void test_run_current_not_draining_queue() {
        for_test_dispatchers([&](TestDispatcher* it) {
            assert_runs_fast([&]() {
                auto scheduler = it->scheduler;
                TestScope scope(it);
                int stage = 1;
                scope.launch([&]() {
                    delay(kSlow);
                    launch([&]() {
                        delay(kSlow);
                        stage = 3;
                    });
                    scheduler.advance_time_by(std::chrono::milliseconds(kSlow));
                    stage = 2;
                });
                scheduler.advance_time_by(std::chrono::milliseconds(kSlow));
                assert(stage == 1);
                scheduler.run_current();
                assert(stage == 2);
                scheduler.run_current();
                assert(stage == 3);
            });
        });
    }

    /** Tests that [TestCoroutineScheduler.advanceUntilIdle] doesn't hang when itself running in a scheduler task. */
    // TODO: @Test
    void test_nested_advance_until_idle() {
        for_test_dispatchers([&](TestDispatcher* it) {
            assert_runs_fast([&]() {
                auto scheduler = it->scheduler;
                TestScope scope(it);
                bool executed = false;
                scope.launch([&]() {
                    launch([&]() {
                        delay(kSlow);
                        executed = true;
                    });
                    scheduler.advance_until_idle();
                });
                scheduler.advance_until_idle();
                assert(executed);
            });
        });
    }

    /** Tests [yield] scheduling tasks for future execution and not executing immediately. */
    // TODO: @Test
    void test_yield() {
        for_test_dispatchers([&](TestDispatcher* it) {
            TestScope scope(it);
            int stage = 0;
            scope.launch([&]() {
                yield();
                assert(stage == 1);
                stage = 2;
            });
            scope.launch([&]() {
                yield();
                assert(stage == 2);
                stage = 3;
            });
            assert(stage == 0);
            stage = 1;
            scope.run_current();
        });
    }

    /** Tests that dispatching the delayed tasks is ordered by their waking times. */
    // TODO: @Test
    void test_delays_priority() {
        for_test_dispatchers([&](TestDispatcher* it) {
            TestScope scope(it);
            int64_t last_measurement = 0;
            auto check_time = [&](int64_t time) {
                assert(last_measurement < time);
                assert(time == scope.current_time);
                last_measurement = scope.current_time;
            };
            scope.launch([&]() {
                launch([&]() {
                    delay(100);
                    check_time(100);
                    auto deferred = async_coro([&]() {
                        delay(70);
                        check_time(170);
                    });
                    delay(1);
                    check_time(101);
                    deferred.await();
                    delay(1);
                    check_time(171);
                });
                launch([&]() {
                    delay(200);
                    check_time(200);
                });
                launch([&]() {
                    delay(150);
                    check_time(150);
                    delay(22);
                    check_time(172);
                });
                delay(201);
            });
            scope.advance_until_idle();
            check_time(201);
        });
    }

private:
    void check_timeout(
        TestScope& scope,
        bool times_out,
        int64_t timeout_millis,
        std::function<void()> block
    ) {
        assert_runs_fast([&]() {
            bool caught_exception = false;
            scope.as_specific_implementation().enter();
            scope.launch([&]() {
                try {
                    with_timeout(timeout_millis, block);
                } catch (const TimeoutCancellationException& e) {
                    caught_exception = true;
                }
            });
            scope.advance_until_idle();
            throw_all(nullptr, scope.as_specific_implementation().legacy_leave());
            if (times_out) {
                assert(caught_exception);
            } else {
                assert(!caught_exception);
            }
        });
    }

public:
    /** Tests that timeouts get triggered. */
    // TODO: @Test
    void test_small_timeouts() {
        for_test_dispatchers([&](TestDispatcher* it) {
            TestScope scope(it);
            check_timeout(scope, true, kSlow, [&]() {
                int64_t half = kSlow / 2;
                delay(half);
                delay(kSlow - half);
            });
        });
    }

    /** Tests that timeouts don't get triggered if the code finishes in time. */
    // TODO: @Test
    void test_large_timeouts() {
        for_test_dispatchers([&](TestDispatcher* it) {
            TestScope scope(it);
            check_timeout(scope, false, kSlow, [&]() {
                int64_t half = kSlow / 2;
                delay(half);
                delay(kSlow - half - 1);
            });
        });
    }

    /** Tests that timeouts get triggered if the code fails to finish in time asynchronously. */
    // TODO: @Test
    void test_small_asynchronous_timeouts() {
        for_test_dispatchers([&](TestDispatcher* it) {
            TestScope scope(it);
            auto deferred = CompletableDeferred<void>();
            scope.launch([&]() {
                int64_t half = kSlow / 2;
                delay(half);
                delay(kSlow - half);
                deferred.complete();
            });
            check_timeout(scope, true, kSlow, [&]() {
                deferred.await();
            });
        });
    }

    /** Tests that timeouts don't get triggered if the code finishes in time, even if it does so asynchronously. */
    // TODO: @Test
    void test_large_asynchronous_timeouts() {
        for_test_dispatchers([&](TestDispatcher* it) {
            TestScope scope(it);
            auto deferred = CompletableDeferred<void>();
            scope.launch([&]() {
                int64_t half = kSlow / 2;
                delay(half);
                delay(kSlow - half - 1);
                deferred.complete();
            });
            check_timeout(scope, false, kSlow, [&]() {
                deferred.await();
            });
        });
    }

    // TODO: @Test
    void test_advance_time_source() {
        run_test([&]() {
            auto expected = std::chrono::seconds(1);
            auto before = test_time_source.mark_now();
            auto actual = test_time_source.measure_time([&]() {
                delay(expected);
            });
            assert(expected == actual);
            auto after = test_time_source.mark_now();
            assert(before < after);
            assert(expected == after - before);
        });
    }

private:
    void for_test_dispatchers(std::function<void(TestDispatcher*)> block) {
        // TODO: @Suppress("DEPRECATION")
        std::vector<TestDispatcher*> dispatchers = {
            new StandardTestDispatcher(),
            new UnconfinedTestDispatcher()
        };
        for (auto* it : dispatchers) {
            try {
                block(it);
            } catch (const std::exception& e) {
                std::stringstream ss;
                ss << "Test failed for dispatcher " << it;
                throw std::runtime_error(ss.str());
            }
        }
    }
};

} // namespace test
} // namespace coroutines
} // namespace kotlinx
