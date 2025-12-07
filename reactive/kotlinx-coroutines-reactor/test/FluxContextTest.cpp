// Transliterated from: reactive/kotlinx-coroutines-reactor/test/FluxContextTest.cpp

// TODO: #include <kotlinx/coroutines/testing.hpp>
// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <kotlinx/coroutines/flow.hpp>
// TODO: #include <kotlinx/coroutines/reactive.hpp>
// TODO: #include <org/junit/junit.hpp>
// TODO: #include <org/junit/test.hpp>
// TODO: #include <reactor/core/publisher.hpp>
// TODO: #include <kotlin/test.hpp>

namespace kotlinx { namespace coroutines { namespace reactor {

class FluxContextTest : public TestBase {
private:
    CoroutineDispatcher* dispatcher = new_single_thread_context("FluxContextTest");

public:
    // @After
    void tear_down() {
        dispatcher->close();
    }

    // @Test
    // TODO: implement coroutine suspension
    void test_flux_create_as_flow_thread() {
        run_test([]() {
            expect(1);
            Thread* main_thread = Thread::current_thread();
            Thread* dispatcher_thread = with_context(dispatcher, []() { return Thread::current_thread(); });
            assert_true(dispatcher_thread != main_thread);
            Flux<std::string>::create([dispatcher_thread](FluxSink<std::string>* it) {
                assert_equals(dispatcher_thread, Thread::current_thread());
                it->next("OK");
                it->complete();
            })
                .as_flow()
                .flow_on(dispatcher)
                .collect([main_thread](std::string it) {
                    expect(2);
                    assert_equals("OK", it);
                    assert_equals(main_thread, Thread::current_thread());
                });
            finish(3);
        });
    }
};

} } } // namespace kotlinx::coroutines::reactor

// TODO: Semantic implementation tasks:
// 1. Implement After annotation handling
// 2. Implement tear_down() hook
// 3. Implement assert_true() function
// 4. Implement FluxSink<T>::next() and complete()
