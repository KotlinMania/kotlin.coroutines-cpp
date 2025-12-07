// Transliterated from: reactive/kotlinx-coroutines-reactor/test/FlowAsFluxTest.cpp

// TODO: #include <kotlinx/coroutines/testing.hpp>
// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <kotlinx/coroutines/flow.hpp>
// TODO: #include <kotlinx/coroutines/reactive.hpp>
// TODO: #include <org/junit/test.hpp>
// TODO: #include <org/reactivestreams/publisher.hpp>
// TODO: #include <reactor/core/publisher.hpp>
// TODO: #include <reactor/util/context.hpp>
// TODO: #include <java/util/concurrent.hpp>
// TODO: #include <kotlin/test.hpp>

// @Suppress("ReactiveStreamsSubscriberImplementation")
namespace kotlinx { namespace coroutines { namespace reactor {

class FlowAsFluxTest : public TestBase {
public:
    // @Test
    void test_flow_as_flux_context_propagation() {
        Flux<std::string>* flux = flow<std::string>([]() {
            for (int i = 1; i <= 4; ++i) {
                emit(create_mono(i)->await_single());
            }
        })
            .as_flux()
            .context_write(Context::of(1, "1"))
            .context_write(Context::of(2, "2", 3, "3", 4, "4"));
        std::vector<std::string> list = flux->collect_list()->block();
        assert_equals(std::vector<std::string>({"1", "2", "3", "4"}), list);
    }

private:
    // TODO: implement coroutine suspension
    Mono<std::string>* create_mono(int i) {
        return mono([i]() {
            Context ctx = coroutine_context()[ReactorContext::Key]->context;
            return ctx.get_or_default(i, "noValue");
        });
    }

public:
    // @Test
    // TODO: implement coroutine suspension
    void test_flux_as_flow_context_propagation_with_flow_on() {
        run_test([]() {
            expect(1);
            Flux<std::string>::create([](FluxSink<std::string>* it) {
                it->next("OK");
                it->complete();
            })
                .context_write([](Context ctx) {
                    expect(2);
                    assert_equals("CTX", ctx.get(1));
                    return ctx;
                })
                .as_flow()
                .flow_on(ReactorContext(Context::of(1, "CTX")))
                .collect([](std::string it) {
                    expect(3);
                    assert_equals("OK", it);
                });
            finish(4);
        });
    }

    // @Test
    // TODO: implement coroutine suspension
    void test_flux_as_flow_context_propagation_from_scope() {
        run_test([]() {
            expect(1);
            with_context(ReactorContext(Context::of(1, "CTX")), []() {
                Flux<std::string>::create([](FluxSink<std::string>* it) {
                        it->next("OK");
                        it->complete();
                    })
                .context_write([](Context ctx) {
                    expect(2);
                    assert_equals("CTX", ctx.get(1));
                    return ctx;
                })
                .as_flow()
                .collect([](std::string it) {
                    expect(3);
                    assert_equals("OK", it);
                });
            });
            finish(4);
        });
    }

    // @Test
    void test_unconfined_default_context() {
        expect(1);
        Thread* thread = Thread::current_thread();
        auto check_thread = [thread]() {
            assert_same(thread, Thread::current_thread());
        };
        flow_of(42)->as_flux()->subscribe(new class : public Subscriber<int> {
        private:
            Subscription* subscription;

        public:
            void on_subscribe(Subscription* s) override {
                expect(2);
                subscription = s;
                subscription->request(2);
            }

            void on_next(int t) override {
                check_thread();
                expect(3);
                assert_equals(42, t);
            }

            void on_complete() override {
                check_thread();
                expect(4);
            }

            void on_error(Throwable* t) override {
                expect_unreached();
            }
        });
        finish(5);
    }

    // @Test
    void test_confined_context() {
        expect(1);
        std::string thread_name = "FlowAsFluxTest.testConfinedContext";
        auto check_thread = [thread_name]() {
            Thread* current_thread = Thread::current_thread();
            assert_true(current_thread->name().starts_with(thread_name), "Unexpected thread " + current_thread->to_string());
        };
        CountDownLatch* completed = new CountDownLatch(1);
        CoroutineDispatcher* dispatcher = new_single_thread_context(thread_name);
        flow_of(42)->as_flux(dispatcher)->subscribe(new class : public Subscriber<int> {
        private:
            Subscription* subscription;

        public:
            void on_subscribe(Subscription* s) override {
                expect(2);
                subscription = s;
                subscription->request(2);
            }

            void on_next(int t) override {
                check_thread();
                expect(3);
                assert_equals(42, t);
            }

            void on_complete() override {
                check_thread();
                expect(4);
                completed->count_down();
            }

            void on_error(Throwable* t) override {
                expect_unreached();
            }
        });
        completed->await();
        dispatcher->close();
        finish(5);
    }
};

} } } // namespace kotlinx::coroutines::reactor

// TODO: Semantic implementation tasks:
// 1. Implement flow() builder function
// 2. Implement emit() function
// 3. Implement as_flux() conversion
// 4. Implement context_write() for Flux
// 5. Implement Context::of() with multiple parameters
// 6. Implement collect_list() method
// 7. Implement block() method
// 8. Implement Flux::create() method
// 9. Implement FluxSink<T> interface
// 10. Implement flow_on() method
// 11. Implement with_context() function
// 12. Implement flow_of() function
// 13. Implement Subscriber<T> interface
// 14. Implement Subscription interface
// 15. Implement Thread class and current_thread()
// 16. Implement assert_same() function
// 17. Implement expect_unreached() function
// 18. Implement CountDownLatch class
// 19. Implement new_single_thread_context() function
// 20. Implement CoroutineDispatcher::close()
// 21. Handle Suppress annotation
