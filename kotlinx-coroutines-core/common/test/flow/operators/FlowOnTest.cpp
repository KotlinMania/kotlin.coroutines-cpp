// Original file: kotlinx-coroutines-core/common/test/flow/operators/FlowOnTest.kt
// TODO: handle imports (kotlinx.coroutines.testing, kotlinx.coroutines, kotlinx.coroutines.channels, kotlinx.coroutines.testing.flow, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle Flow types and operations
// TODO: translate inner classes

namespace kotlinx {
namespace coroutines {
namespace flow {

class FlowOnTest : public TestBase {
public:
    // @Test
    void test_flow_on() /* TODO: = runTest */ {
        Source source(42);
        Consumer consumer(42);

        auto flow = source.produce().as_flow();
        flow.flow_on(NamedDispatchers("ctx1")).launch_in(*this, [&](auto& builder) {
            builder.on_each([&consumer](int it) { consumer.consume(it); });
        }).join();

        assert_equals("ctx1", source.context_name);
        assert_equals("main", consumer.context_name);

        flow.flow_on(NamedDispatchers("ctx2")).launch_in(*this, [&](auto& builder) {
            builder.on_each([&consumer](int it) { consumer.consume(it); });
        }).join();

        assert_equals("ctx2", source.context_name);
        assert_equals("main", consumer.context_name);
    }

    // @Test
    void test_flow_on_and_operators() /* TODO: = runTest */ {
        Source source(42);
        Consumer consumer(42);
        std::vector<std::string> captured;
        auto mapper = [&captured](int it) /* TODO: suspend */ -> int {
            captured.push_back(NamedDispatchers::name_or("main"));
            return it;
        };

        auto flow = source.produce().as_flow();
        flow.map(mapper)
            .flow_on(NamedDispatchers("ctx1"))
            .map(mapper)
            .flow_on(NamedDispatchers("ctx2"))
            .map(mapper)
            .launch_in(*this, [&](auto& builder) {
                builder.on_each([&consumer](int it) { consumer.consume(it); });
            }).join();

        assert_equals(std::vector<std::string>{"ctx1", "ctx2", "main"}, captured);
        assert_equals("ctx1", source.context_name);
        assert_equals("main", consumer.context_name);
    }

    // @Test
    void test_flow_on_throwing_source() /* TODO: = runTest */ {
        auto flow = flow([]() /* TODO: suspend */ {
            expect(1);
            emit(NamedDispatchers::name());
            expect(3);
            throw TestException();
        }).map([](const std::string& it) {
            expect(2);
            assert_equals("throwing", it);
            return it;
        }).flow_on(NamedDispatchers("throwing"));

        assert_fails_with<TestException>([&]() { flow.single(); });
        ensure_active();
        finish(4);
    }

    // @Test
    void test_flow_on_throwing_operator() /* TODO: = runTest */ {
        auto flow = flow([]() /* TODO: suspend */ {
            expect(1);
            emit(NamedDispatchers::name());
            expect_unreached();
        }).map([](const std::string& it) {
            expect(2);
            assert_equals("throwing", it);
            throw TestException();
        }).flow_on(NamedDispatchers("throwing"));

        assert_fails_with<TestException>(flow);
        ensure_active();
        finish(3);
    }

    // @Test
    void test_flow_on_downstream_operator() /* TODO: = runTest */ {
        auto flow = flow([]() /* TODO: suspend */ {
            expect(2);
            emit(NamedDispatchers::name());
            hang([]() { expect(5); });
            delay(LONG_MAX);
        }).map([](const std::string& it) {
            expect(3);
            return it;
        }).flow_on(NamedDispatchers("throwing"))
            .map<std::string, std::string>([](const std::string&) {
                expect(4);
                throw TestException();
            });

        expect(1);
        assert_fails_with<TestException>([&]() { flow.single(); });
        ensure_active();
        finish(6);
    }

    // @Test
    void test_flow_on_throwing_consumer() /* TODO: = runTest */ {
        auto flow = flow([]() /* TODO: suspend */ {
            expect(2);
            emit(NamedDispatchers::name());
            hang([]() { expect(4); });
        });

        expect(1);
        flow.flow_on(NamedDispatchers("...")).launch_in(*this + NamedDispatchers("launch"), [](auto& builder) {
            builder
                .on_each([](const std::string&) {
                    expect(3);
                    throw TestException();
                })
                .catch_error<Throwable>([](const Throwable&) { expect(5); });
        }).join();

        ensure_active();
        finish(6);
    }

    // @Test
    void test_flow_on_with_job() /* TODO: = runTest({ it is IllegalArgumentException }) */ {
        flow([]() /* TODO: suspend */ {
            emit(1);
        }).flow_on(NamedDispatchers("foo") + Job());
    }

    // @Test
    void test_flow_on_cancellation() /* TODO: = runTest */ {
        Channel<Unit> latch;
        expect(1);
        auto job = launch(NamedDispatchers("launch"), [&]() /* TODO: suspend */ {
            flow<int>([&]() /* TODO: suspend */ {
                expect(2);
                latch.send(Unit{});
                expect(3);
                hang([&]() {
                    assert_equals("cancelled", NamedDispatchers::name());
                    expect(5);
                });
            }).flow_on(NamedDispatchers("cancelled")).single();
        });

        latch.receive();
        expect(4);
        job.cancel();
        job.join();
        ensure_active();
        finish(6);
    }

    // @Test
    void test_flow_on_cancellation_happens_before() /* TODO: = runTest */ {
        launch([&]() /* TODO: suspend */ {
            try {
                flow<int>([&]() /* TODO: suspend */ {
                    expect(1);
                    auto flow_job = current_context()[Job::key];
                    launch([&]() /* TODO: suspend */ {
                        expect(2);
                        flow_job->cancel();
                    });
                    hang([]() { expect(3); });
                }).flow_on(NamedDispatchers("upstream")).single();
            } catch (const CancellationException& e) {
                expect(4);
            }
        }).join();
        ensure_active();
        finish(5);
    }

    // @Test
    void test_independent_operator_context() /* TODO: = runTest */ {
        auto value = flow([]() /* TODO: suspend */ {
            assert_equals("base", NamedDispatchers::name_or("main"));
            expect(1);
            emit(-239);
        }).map([](int it) {
            assert_equals("base", NamedDispatchers::name_or("main"));
            expect(2);
            return it;
        }).flow_on(NamedDispatchers("base"))
            .map([](int it) {
                assert_equals("main", NamedDispatchers::name_or("main"));
                expect(3);
                return it;
            }).single();

        assert_equals(-239, value);
        finish(4);
    }

    // @Test
    void test_multiple_flow_on() /* TODO: = runTest */ {
        flow([]() /* TODO: suspend */ {
            assert_equals("ctx1", NamedDispatchers::name_or("main"));
            expect(1);
            emit(1);
        }).map([](int) {
            assert_equals("ctx1", NamedDispatchers::name_or("main"));
            expect(2);
        }).flow_on(NamedDispatchers("ctx1"))
            .map([](int) {
                assert_equals("ctx2", NamedDispatchers::name_or("main"));
                expect(3);
            }).flow_on(NamedDispatchers("ctx2"))
            .map([](int) {
                assert_equals("ctx3", NamedDispatchers::name_or("main"));
                expect(4);
            }).flow_on(NamedDispatchers("ctx3"))
            .map([](int) {
                assert_equals("main", NamedDispatchers::name_or("main"));
                expect(5);
            })
            .single();

        finish(6);
    }

    // @Test
    void test_timeout_exception_upstream() /* TODO: = runTest */ {
        auto flow = flow([]() /* TODO: suspend */ {
            emit(1);
            yield();
            with_timeout(-1, []() {});
            emit(42);
        }).flow_on(NamedDispatchers("foo")).on_each([](int) {
            expect(1);
        });
        assert_fails_with<TimeoutCancellationException>(flow);
        finish(2);
    }

    // @Test
    void test_timeout_exception_downstream() /* TODO: = runTest */ {
        auto flow = flow([]() /* TODO: suspend */ {
            emit(1);
            hang([]() { expect(2); });
        }).flow_on(NamedDispatchers("foo")).on_each([](int) {
            expect(1);
            with_timeout(-1, []() {});
        });
        assert_fails_with<TimeoutCancellationException>(flow);
        finish(3);
    }

    // @Test
    void test_cancellation() /* TODO: = runTest */ {
        auto result = flow([]() /* TODO: suspend */ {
            emit(1);
            emit(2);
            emit(3);
            expect_unreached();
            emit(4);
        }).flow_on(wrapper_dispatcher())
            .buffer(0)
            .take(2)
            .to_list();
        assert_equals(std::vector<int>{1, 2}, result);
    }

    // @Test
    void test_atomic_start() /* TODO: = runTest */ {
        try {
            coroutine_scope([&]() /* TODO: suspend */ {
                auto job = coroutine_context()[Job::key];
                auto flow = flow([]() /* TODO: suspend */ {
                    expect(3);
                    emit(1);
                })
                    .on_completion([]() { expect(4); })
                    .flow_on(wrapper_dispatcher())
                    .on_completion([]() { expect(5); });

                launch([&]() /* TODO: suspend */ {
                    expect(1);
                    flow.collect();
                });
                launch([&]() /* TODO: suspend */ {
                    expect(2);
                    job->cancel();
                });
            });
        } catch (const CancellationException& e) {
            finish(6);
        }
    }

    // @Test
    void test_exception() /* TODO: = runTest */ {
        auto flow = flow([]() /* TODO: suspend */ {
            emit(314);
            delay(LONG_MAX);
        }).flow_on(NamedDispatchers("upstream"))
            .map([](int) {
                throw TestException();
            });

        assert_fails_with<TestException>([&]() { flow.single(); });
        assert_fails_with<TestException>(flow);
        ensure_active();
    }

    // @Test
    void test_illegal_argument_exception() {
        auto flow = empty_flow<int>();
        assert_fails_with<IllegalArgumentException>([&]() { flow.flow_on(Job()); });
    }

    // @Test
    void test_cancelled_flow_on() /* TODO: = runTest */ {
        assert_fails_with<CancellationException>([&]() {
            coroutine_scope([&]() /* TODO: suspend */ {
                auto scope = this;
                flow([]() /* TODO: suspend */ {
                    emit(Unit{}); // emit to buffer
                    scope->cancel(); // now cancel outer scope
                }).flow_on(wrapper_dispatcher()).collect([](const Unit&) {
                    // should not be reached, because cancelled before it runs
                    expect_unreached();
                });
            });
        });
    }

private:
    class Source {
    public:
        std::string context_name = "unknown";

        Source(int value_) : value(value_) {}

        int produce() {
            context_name = NamedDispatchers::name_or("main");
            return value;
        }

    private:
        int value;
    };

    class Consumer {
    public:
        std::string context_name = "unknown";

        Consumer(int expected_) : expected(expected_) {}

        void consume(int value) {
            context_name = NamedDispatchers::name_or("main");
            assert_equals(expected, value);
        }

    private:
        int expected;
    };
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
