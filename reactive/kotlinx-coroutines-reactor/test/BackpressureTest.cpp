// Transliterated from: reactive/kotlinx-coroutines-reactor/test/BackpressureTest.cpp

// TODO: #include <kotlinx/coroutines/testing.hpp>
// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <kotlinx/coroutines/channels.hpp>
// TODO: #include <kotlinx/coroutines/flow.hpp>
// TODO: #include <kotlinx/coroutines/reactive.hpp>
// TODO: #include <org/junit/test.hpp>
// TODO: #include <reactor/core/publisher.hpp>
// TODO: #include <kotlin/test.hpp>

namespace kotlinx { namespace coroutines { namespace reactor {

class BackpressureTest : public TestBase {
public:
    // @Test
    // TODO: implement coroutine suspension
    void test_backpressure_drop_direct() {
        run_test([]() {
            expect(1);
            Flux<int>::from_array({1})
                .on_backpressure_drop()
                .collect([](int it) {
                    assert_equals(1, it);
                    expect(2);
                });
            finish(3);
        });
    }

    // @Test
    // TODO: implement coroutine suspension
    void test_backpressure_drop_flow() {
        run_test([]() {
            expect(1);
            Flux<int>::from_array({1})
                .on_backpressure_drop()
                .as_flow()
                .collect([](int it) {
                    assert_equals(1, it);
                    expect(2);
                });
            finish(3);
        });
    }

    // @Test
    // TODO: implement coroutine suspension
    void test_cooperative_cancellation() {
        run_test([]() {
            Flow<int64_t>* flow = Flux<int64_t>::from_iterable(range(0, INT64_MAX)).as_flow();
            flow->on_each([](int64_t it) {
                if (it > 10) {
                    current_coroutine_context().cancel();
                }
            }).launch_in(*this + Dispatchers::Default).join();
        });
    }

    // @Test
    // TODO: implement coroutine suspension
    void test_cooperative_cancellation_for_buffered() {
        run_test([](auto expected) {
            // expected = { it is CancellationException }
            Flow<int64_t>* flow = Flux<int64_t>::from_iterable(range(0, INT64_MAX)).as_flow();
            ReceiveChannel<int64_t>* channel = flow->on_each([](int64_t it) {
                if (it > 10) {
                    current_coroutine_context().cancel();
                }
            }).produce_in(*this + Dispatchers::Default);
            channel->consume_each([](int64_t) {
                /* Do nothing, just consume elements */
            });
        });
    }
};

} } } // namespace kotlinx::coroutines::reactor

// TODO: Semantic implementation tasks:
// 1. Implement TestBase class
// 2. Implement run_test() function
// 3. Implement expect() and finish() test helpers
// 4. Implement assert_equals() function
// 5. Implement Flux<T>::from_array() and from_iterable()
// 6. Implement on_backpressure_drop() method
// 7. Implement collect() method
// 8. Implement as_flow() conversion
// 9. Implement Flow<T> type
// 10. Implement on_each() method for Flow
// 11. Implement current_coroutine_context() function
// 12. Implement cancel() on CoroutineContext
// 13. Implement launch_in() method
// 14. Implement join() method
// 15. Implement Dispatchers::Default
// 16. Implement produce_in() method
// 17. Implement consume_each() method
// 18. Implement ReceiveChannel<T> type
// 19. Implement range() utility function
// 20. Handle Test annotation
// 21. Implement CancellationException type
