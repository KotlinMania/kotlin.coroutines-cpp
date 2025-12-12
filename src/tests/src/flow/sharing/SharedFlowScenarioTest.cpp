// Transliterated from: kotlinx-coroutines-core/common/test/flow/sharing/SharedFlowScenarioTest.kt

// TODO: #include equivalent for kotlinx.coroutines.testing.*
// TODO: #include equivalent for kotlinx.coroutines.*
// TODO: #include equivalent for kotlinx.coroutines.channels.*
// TODO: #include equivalent for kotlin.coroutines.*
// TODO: #include equivalent for kotlin.test.*

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            /**
 * This test suit for [SharedFlow] has a dense framework that allows to test complex
 * suspend/resume scenarios while keeping the code readable. Each test here is for
 * one specific [SharedFlow] configuration, testing all the various corner cases in its
 * behavior.
 */
            class SharedFlowScenarioTest : public TestBase {
            public:
                // @Test
                void test_replay1_extra2() {
                    // TODO: implement coroutine suspension
                    test_shared_flow(MutableSharedFlow<int>(1, 2), [this]() {
                        // total buffer size == 3
                        expect_replay_of();
                        emit_right_now(1);
                        expect_replay_of(1);
                        emit_right_now(2);
                        expect_replay_of(2);
                        emit_right_now(3);
                        expect_replay_of(3);
                        emit_right_now(4);
                        expect_replay_of(4); // no prob - no subscribers
                        auto a = subscribe("a");
                        collect(a, 4);
                        emit_right_now(5);
                        expect_replay_of(5);
                        emit_right_now(6);
                        expect_replay_of(6);
                        emit_right_now(7);
                        expect_replay_of(7);
                        // suspend/collect sequentially
                        auto e8 = emit_suspends(8);
                        collect(a, 5);
                        emit_resumes(e8);
                        expect_replay_of(8);
                        auto e9 = emit_suspends(9);
                        collect(a, 6);
                        emit_resumes(e9);
                        expect_replay_of(9);
                        // buffer full, but parallel emitters can still suspend (queue up)
                        auto e10 = emit_suspends(10);
                        auto e11 = emit_suspends(11);
                        auto e12 = emit_suspends(12);
                        collect(a, 7);
                        emit_resumes(e10);
                        expect_replay_of(10); // buffer 8, 9 | 10
                        collect(a, 8);
                        emit_resumes(e11);
                        expect_replay_of(11); // buffer 9, 10 | 11
                        shared_flow.reset_replay_cache();
                        expect_replay_of(); // 9, 10 11 | no replay
                        collect(a, 9);
                        emit_resumes(e12);
                        expect_replay_of(12);
                        collect(a, 10, 11, 12);
                        expect_replay_of(12); // buffer empty | 12
                        emit_right_now(13);
                        expect_replay_of(13);
                        emit_right_now(14);
                        expect_replay_of(14);
                        emit_right_now(15);
                        expect_replay_of(15); // buffer 13, 14 | 15
                        auto e16 = emit_suspends(16);
                        auto e17 = emit_suspends(17);
                        auto e18 = emit_suspends(18);
                        cancel(e17);
                        expect_replay_of(15); // cancel in the middle of three emits; buffer 13, 14 | 15
                        collect(a, 13);
                        emit_resumes(e16);
                        expect_replay_of(16); // buffer 14, 15, | 16
                        collect(a, 14);
                        emit_resumes(e18);
                        expect_replay_of(18); // buffer 15, 16 | 18
                        auto e19 = emit_suspends(19);
                        auto e20 = emit_suspends(20);
                        auto e21 = emit_suspends(21);
                        cancel(e21);
                        expect_replay_of(18); // cancel last emit; buffer 15, 16, 18
                        collect(a, 15);
                        emit_resumes(e19);
                        expect_replay_of(19); // buffer 16, 18 | 19
                        collect(a, 16);
                        emit_resumes(e20);
                        expect_replay_of(20); // buffer 18, 19 | 20
                        collect(a, 18, 19, 20);
                        expect_replay_of(20); // buffer empty | 20
                        emit_right_now(22);
                        expect_replay_of(22);
                        emit_right_now(23);
                        expect_replay_of(23);
                        emit_right_now(24);
                        expect_replay_of(24); // buffer 22, 23 | 24
                        auto e25 = emit_suspends(25);
                        auto e26 = emit_suspends(26);
                        auto e27 = emit_suspends(27);
                        cancel(e25);
                        expect_replay_of(24); // cancel first emit, buffer 22, 23 | 24
                        shared_flow.reset_replay_cache();
                        expect_replay_of(); // buffer 22, 23, 24 | no replay
                        auto b = subscribe("b"); // new subscriber
                        collect(a, 22);
                        emit_resumes(e26);
                        expect_replay_of(26); // buffer 23, 24 | 26
                        collect(b, 26);
                        collect(a, 23);
                        emit_resumes(e27);
                        expect_replay_of(27); // buffer 24, 26 | 27
                        collect(a, 24, 26, 27); // buffer empty | 27
                        emit_right_now(28);
                        expect_replay_of(28);
                        emit_right_now(29);
                        expect_replay_of(29); // buffer 27, 28 | 29
                        collect(a, 28, 29); // but b is slow
                        auto e30 = emit_suspends(30);
                        auto e31 = emit_suspends(31);
                        auto e32 = emit_suspends(32);
                        auto e33 = emit_suspends(33);
                        auto e34 = emit_suspends(34);
                        auto e35 = emit_suspends(35);
                        auto e36 = emit_suspends(36);
                        auto e37 = emit_suspends(37);
                        auto e38 = emit_suspends(38);
                        auto e39 = emit_suspends(39);
                        cancel(e31); // cancel emitter in queue
                        cancel(b); // cancel slow subscriber -> 3 emitters resume
                        emit_resumes(e30);
                        emit_resumes(e32);
                        emit_resumes(e33);
                        expect_replay_of(33); // buffer 30, 32 | 33
                        auto c = subscribe("c");
                        collect(c, 33); // replays
                        cancel(e34);
                        collect(a, 30);
                        emit_resumes(e35);
                        expect_replay_of(35); // buffer 32, 33 | 35
                        cancel(e37);
                        cancel(a);
                        emit_resumes(e36);
                        emit_resumes(e38);
                        expect_replay_of(38); // buffer 35, 36 | 38
                        collect(c, 35);
                        emit_resumes(e39);
                        expect_replay_of(39); // buffer 36, 38 | 39
                        collect(c, 36, 38, 39);
                        expect_replay_of(39);
                        cancel(c);
                        expect_replay_of(39); // replay stays
                    });
                }

                // @Test
                void test_replay1() {
                    // TODO: implement coroutine suspension
                    test_shared_flow(MutableSharedFlow<int>(1), [this]() {
                        emit_right_now(0);
                        expect_replay_of(0);
                        emit_right_now(1);
                        expect_replay_of(1);
                        emit_right_now(2);
                        expect_replay_of(2);
                        shared_flow.reset_replay_cache();
                        expect_replay_of();
                        shared_flow.reset_replay_cache();
                        expect_replay_of();
                        emit_right_now(3);
                        expect_replay_of(3);
                        emit_right_now(4);
                        expect_replay_of(4);
                        auto a = subscribe("a");
                        collect(a, 4);
                        emit_right_now(5);
                        expect_replay_of(5);
                        collect(a, 5);
                        emit_right_now(6);
                        shared_flow.reset_replay_cache();
                        expect_replay_of();
                        shared_flow.reset_replay_cache();
                        expect_replay_of();
                        auto e7 = emit_suspends(7);
                        auto e8 = emit_suspends(8);
                        auto e9 = emit_suspends(9);
                        collect(a, 6);
                        emit_resumes(e7);
                        expect_replay_of(7);
                        shared_flow.reset_replay_cache();
                        expect_replay_of();
                        shared_flow.reset_replay_cache();
                        expect_replay_of(); // buffer 7 | -- no replay, but still buffered
                        auto b = subscribe("b");
                        collect(a, 7);
                        emit_resumes(e8);
                        expect_replay_of(8);
                        collect(b, 8); // buffer | 8 -- a is slow
                        auto e10 = emit_suspends(10);
                        auto e11 = emit_suspends(11);
                        auto e12 = emit_suspends(12);
                        cancel(e9);
                        collect(a, 8);
                        emit_resumes(e10);
                        expect_replay_of(10);
                        collect(a, 10); // now b's slow
                        cancel(e11);
                        collect(b, 10);
                        emit_resumes(e12);
                        expect_replay_of(12);
                        collect(a, 12);
                        collect(b, 12);
                        shared_flow.reset_replay_cache();
                        expect_replay_of();
                        shared_flow.reset_replay_cache();
                        expect_replay_of(); // nothing is buffered -- both collectors up to date
                        emit_right_now(13);
                        expect_replay_of(13);
                        collect(b, 13); // a is slow
                        auto e14 = emit_suspends(14);
                        auto e15 = emit_suspends(15);
                        auto e16 = emit_suspends(16);
                        cancel(e14);
                        cancel(a);
                        emit_resumes(e15);
                        expect_replay_of(15); // cancelling slow subscriber
                        collect(b, 15);
                        emit_resumes(e16);
                        expect_replay_of(16);
                        collect(b, 16);
                    });
                }

                // @Test
                void test_replay2_extra2_drop_oldest() {
                    // TODO: implement coroutine suspension
                    test_shared_flow<int>(MutableSharedFlow(2, 2, BufferOverflow::kDropOldest), [this]() {
                        emit_right_now(0);
                        expect_replay_of(0);
                        emit_right_now(1);
                        expect_replay_of(0, 1);
                        emit_right_now(2);
                        expect_replay_of(1, 2);
                        emit_right_now(3);
                        expect_replay_of(2, 3);
                        emit_right_now(4);
                        expect_replay_of(3, 4);
                        auto a = subscribe("a");
                        collect(a, 3);
                        emit_right_now(5);
                        expect_replay_of(4, 5);
                        emit_right_now(6);
                        expect_replay_of(5, 6);
                        emit_right_now(7);
                        expect_replay_of(6, 7); // buffer 4, 5 | 6, 7
                        emit_right_now(8);
                        expect_replay_of(7, 8); // buffer 5, 6 | 7, 8
                        emit_right_now(9);
                        expect_replay_of(8, 9); // buffer 6, 7 | 8, 9
                        collect(a, 6, 7);
                        auto b = subscribe("b");
                        collect(b, 8, 9); // buffer | 8, 9
                        emit_right_now(10);
                        expect_replay_of(9, 10); // buffer 8 | 9, 10
                        collect(a, 8, 9, 10); // buffer | 9, 10, note "b" had not collected 10 yet
                        emit_right_now(11);
                        expect_replay_of(10, 11); // buffer | 10, 11
                        emit_right_now(12);
                        expect_replay_of(11, 12); // buffer 10 | 11, 12
                        emit_right_now(13);
                        expect_replay_of(12, 13); // buffer 10, 11 | 12, 13
                        emit_right_now(14);
                        expect_replay_of(13, 14); // buffer 11, 12 | 13, 14, "b" missed 10
                        collect(b, 11, 12, 13, 14);
                        shared_flow.reset_replay_cache();
                        expect_replay_of(); // buffer 11, 12, 13, 14 |
                        shared_flow.reset_replay_cache();
                        expect_replay_of();
                        collect(a, 11, 12, 13, 14);
                        emit_right_now(15);
                        expect_replay_of(15);
                        collect(a, 15);
                        collect(b, 15);
                    });
                }

                // @Test // https://github.com/Kotlin/kotlinx.coroutines/issues/2320
                void test_resume_fast_subscriber_on_resumed_emitter() {
                    // TODO: implement coroutine suspension
                    test_shared_flow<int>(MutableSharedFlow(1), [this]() {
                        // create two subscribers and start collecting
                        auto s1 = subscribe("s1");
                        resume_collecting(s1);
                        auto s2 = subscribe("s2");
                        resume_collecting(s2);
                        // now emit 0, make sure it is collected
                        emit_right_now(0);
                        expect_replay_of(0);
                        await_collected(s1, 0);
                        await_collected(s2, 0);
                        // now emit 1, and only first subscriber continues and collects it
                        emit_right_now(1);
                        expect_replay_of(1);
                        collect(s1, 1);
                        // now emit 2, it suspend (s2 is blocking it)
                        auto e2 = emit_suspends(2);
                        resume_collecting(s1); // resume, but does not collect (e2 is still queued)
                        collect(s2, 1); // resume + collect next --> resumes emitter, thus resumes s1
                        await_collected(s1, 2); // <-- S1 collects value from the newly resumed emitter here !!!
                        emit_resumes(e2);
                        expect_replay_of(2);
                        // now emit 3, it suspends (s2 blocks it)
                        auto e3 = emit_suspends(3);
                        collect(s2, 2);
                        emit_resumes(e3);
                        expect_replay_of(3);
                    });
                }

                // @Test
                void test_suspended_concurrent_emit_and_cancel_subscriber_replay1() {
                    // TODO: implement coroutine suspension
                    test_shared_flow<int>(MutableSharedFlow(1), [this]() {
                        auto a = subscribe("a");
                        emit_right_now(0);
                        expect_replay_of(0);
                        collect(a, 0);
                        emit_right_now(1);
                        expect_replay_of(1);
                        auto e2 = emit_suspends(2); // suspends until 1 is collected
                        auto e3 = emit_suspends(3); // suspends until 1 is collected, too
                        cancel(a); // must resume emitters 2 & 3
                        emit_resumes(e2);
                        emit_resumes(e3);
                        expect_replay_of(3); // but replay size is 1 so only 3 should be kept
                        // Note: originally, SharedFlow was in a broken state here with 3 elements in the buffer
                        auto b = subscribe("b");
                        collect(b, 3);
                        emit_right_now(4);
                        expect_replay_of(4);
                        collect(b, 4);
                    });
                }

                // @Test
                void test_suspended_concurrent_emit_and_cancel_subscriber_replay1_extra_buffer1() {
                    // TODO: implement coroutine suspension
                    test_shared_flow<int>(MutableSharedFlow(/*replay=*/1, /*extraBufferCapacity=*/1), [this]() {
                        auto a = subscribe("a");
                        emit_right_now(0);
                        expect_replay_of(0);
                        collect(a, 0);
                        emit_right_now(1);
                        expect_replay_of(1);
                        emit_right_now(2);
                        expect_replay_of(2);
                        auto e3 = emit_suspends(3); // suspends until 1 is collected
                        auto e4 = emit_suspends(4); // suspends until 1 is collected, too
                        auto e5 = emit_suspends(5); // suspends until 1 is collected, too
                        cancel(a); // must resume emitters 3, 4, 5
                        emit_resumes(e3);
                        emit_resumes(e4);
                        emit_resumes(e5);
                        expect_replay_of(5);
                        auto b = subscribe("b");
                        collect(b, 5);
                        emit_right_now(6);
                        expect_replay_of(6);
                        collect(b, 6);
                    });
                }

            private:
                template<typename T>
                void test_shared_flow(
                    MutableSharedFlow<T> shared_flow,
                    std::function<void(ScenarioDsl<T> &)> scenario
                ) {
                    // TODO: implement coroutine suspension
                    run_test([&]() {
                        ScenarioDsl<T> *dsl = nullptr;
                        try {
                            coroutine_scope([&]() {
                                dsl = new ScenarioDsl<T>(shared_flow, coroutine_context);
                                dsl->scenario();
                                dsl->stop();
                            });
                        } catch (const std::exception &e) {
                            if (dsl) dsl->print_log();
                            throw;
                        }
                    });
                }

                struct TestJob {
                    Job *job;
                    std::string name;

                    std::string to_string() const { return name; }
                };

                class Action {
                public:
                    virtual ~Action() = default;
                };

                class EmitResumes : public Action {
                public:
                    TestJob job;

                    explicit EmitResumes(TestJob j) : job(j) {
                    }
                };

                class Collected : public Action {
                public:
                    TestJob job;
                    void *value; // TODO: proper type
                    Collected(TestJob j, void *v) : job(j), value(v) {
                    }
                };

                class ResumeCollecting : public Action {
                public:
                    TestJob job;

                    explicit ResumeCollecting(TestJob j) : job(j) {
                    }
                };

                class Cancelled : public Action {
                public:
                    TestJob job;

                    explicit Cancelled(TestJob j) : job(j) {
                    }
                };

                // @OptIn(ExperimentalStdlibApi::class)
                template<typename T>
                class ScenarioDsl {
                private:
                    MutableSharedFlow<T> shared_flow;
                    CoroutineContext coroutine_context;
                    std::vector<std::string> log;
                    int64_t timeout = 10000L;
                    CoroutineScope scope;
                    std::unordered_set<Action *> actions;
                    std::deque<Continuation<void> *> action_waiters;
                    std::vector<T> expected_replay;

                    void check_replay() {
                        assert_equals(expected_replay, shared_flow.replay_cache());
                    }

                    void wakeup_waiters() {
                        for (size_t i = 0; i < action_waiters.size(); ++i) {
                            action_waiters.front()->resume(/*Unit*/);
                            action_waiters.pop_front();
                        }
                    }

                    void add_action(Action *action) {
                        actions.insert(action);
                        wakeup_waiters();
                    }

                    // TODO: implement coroutine suspension
                    void await_action(Action *action) {
                        with_timeout_or_null(timeout, [&]() {
                            while (actions.find(action) == actions.end() || actions.erase(action) == 0) {
                                suspend_cancellable_coroutine<void>([&](Continuation<void> *cont) {
                                    action_waiters.push_back(cont);
                                });
                            }
                        }); // ?: error("Timed out waiting for action: $action")
                        wakeup_waiters();
                    }

                    TestJob launch_emit(T a) {
                        std::string name = "emit(" + std::to_string(a) + ")";
                        // TODO: implement coroutine suspension
                        Job *job = scope.launch(CoroutineStart::kUndispatched, [&]() {
                            TestJob job_obj{
                                /*coroutineContext[Job]!*/ nullptr, name
                            };
                            try {
                                log_msg(name);
                                shared_flow.emit(a);
                                log_msg(name + " resumes");
                                add_action(new EmitResumes(job_obj));
                            } catch (const CancellationException &e) {
                                log_msg(name + " cancelled");
                                add_action(new Cancelled(job_obj));
                            }
                        });
                        return TestJob{job, name};
                    }

                public:
                    ScenarioDsl(MutableSharedFlow<T> sf, CoroutineContext ctx)
                        : shared_flow(sf), coroutine_context(ctx), scope(ctx /*+ Job()*/) {
                        expected_replay = {};
                    }

                    void expect_replay_of(std::initializer_list<T> a) {
                        expected_replay = std::vector<T>(a);
                        check_replay();
                    }

                    void emit_right_now(T a) {
                        TestJob job = launch_emit(a);
                        assert_true(actions.erase(new EmitResumes(job)) > 0);
                    }

                    TestJob emit_suspends(T a) {
                        TestJob job = launch_emit(a);
                        assert_false(actions.find(new EmitResumes(job)) != actions.end());
                        check_replay();
                        return job;
                    }

                    // TODO: implement coroutine suspension
                    void emit_resumes(TestJob job) {
                        await_action(new EmitResumes(job));
                    }

                    // TODO: implement coroutine suspension
                    void cancel(TestJob job) {
                        log_msg("cancel(" + job.name + ")");
                        job.job->cancel();
                        await_action(new Cancelled(job));
                    }

                    TestJob subscribe(const std::string &id) {
                        std::string name = "collect(" + id + ")";
                        // TODO: implement coroutine suspension
                        Job *job = scope.launch(CoroutineStart::kUndispatched, [&]() {
                            TestJob job_obj{
                                /*coroutineContext[Job]!*/ nullptr, name
                            };
                            try {
                                await_action(new ResumeCollecting(job_obj));
                                log_msg(name + " start");
                                shared_flow.collect([&](T value) {
                                    log_msg(name + " -> " + std::to_string(value));
                                    add_action(new Collected(job_obj, &value));
                                    await_action(new ResumeCollecting(job_obj));
                                    log_msg(name + " -> " + std::to_string(value) + " resumes");
                                });
                                // error(name + " completed")
                            } catch (const CancellationException &e) {
                                log_msg(name + " cancelled");
                                add_action(new Cancelled(job_obj));
                            }
                        });
                        return TestJob{job, name};
                    }

                    // collect ~== resumeCollecting + awaitCollected (for each value)
                    // TODO: implement coroutine suspension
                    void collect(TestJob job, std::initializer_list<T> a) {
                        for (const T &value: a) {
                            check_replay(); // should not have changed
                            resume_collecting(job);
                            await_collected(job, value);
                        }
                    }

                    // TODO: implement coroutine suspension
                    void resume_collecting(TestJob job) {
                        add_action(new ResumeCollecting(job));
                    }

                    // TODO: implement coroutine suspension
                    void await_collected(TestJob job, T value) {
                        await_action(new Collected(job, &value));
                    }

                    void stop() {
                        log_msg("--- stop");
                        scope.cancel();
                    }

                    void log_msg(const std::string &text) {
                        log.push_back(text);
                    }

                    void print_log() {
                        std::cout << "--- The most recent log entries ---" << std::endl;
                        size_t start = log.size() > 30 ? log.size() - 30 : 0;
                        for (size_t i = start; i < log.size(); ++i) {
                            std::cout << log[i] << std::endl;
                        }
                        std::cout << "--- That's it ---" << std::endl;
                    }
                };
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement coroutine suspension for all suspend functions
// 2. Implement MutableSharedFlow template class
// 3. Implement BufferOverflow enumeration
// 4. Implement TestBase base class
// 5. Implement run_test, coroutine_scope functions
// 6. Implement CoroutineScope, CoroutineContext, Job classes
// 7. Implement CoroutineStart enumeration
// 8. Implement CancellationException class
// 9. Implement Continuation template
// 10. Implement suspend_cancellable_coroutine, with_timeout_or_null
// 11. Implement proper test assertions (assert_equals, assert_true, assert_false)
// 12. Implement variadic template for expect_replay_of and collect
// 13. Add proper includes for all dependencies
// 14. Handle memory management for Action objects
// 15. Implement proper equality and hashing for Action classes