// Transliterated from: reactive/kotlinx-coroutines-reactor/test/FluxCompletionStressTest.cpp

// TODO: #include <kotlinx/coroutines/testing.hpp>
// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <kotlinx/coroutines/reactive.hpp>
// TODO: #include <org/junit/junit.hpp>
// TODO: #include <java/util/random.hpp>
// TODO: #include <kotlin/coroutines/coroutine_context.hpp>

namespace kotlinx { namespace coroutines { namespace reactor {

class FluxCompletionStressTest : public TestBase {
private:
    static const int kNRepeats = 10000 * stress_test_multiplier;

    // TODO: implement coroutine suspension
    Flux<int> range(CoroutineScope* scope, CoroutineContext context, int start, int count) {
        return flux(context, [start, count](ProducerScope<int>& producer) {
            for (int x = start; x < start + count; ++x) {
                producer.send(x);
            }
        });
    }

public:
    // @Test
    void test_completion() {
        Random rnd;
        for (int i = 0; i < kNRepeats; ++i) {
            int count = rnd.next_int(5);
            run_blocking([]() {
                with_timeout(5000, []() {
                    int received = 0;
                    range(this, Dispatchers::Default, 1, count)->collect([&received](int x) {
                        received++;
                        if (x != received) {
                            throw std::runtime_error(std::to_string(x) + " != " + std::to_string(received));
                        }
                    });
                    if (received != count) {
                        throw std::runtime_error(std::to_string(received) + " != " + std::to_string(count));
                    }
                });
            });
        }
    }
};

} } } // namespace kotlinx::coroutines::reactor

// TODO: Semantic implementation tasks:
// 1. Implement stress_test_multiplier constant
// 2. Implement Random class
// 3. Implement next_int() method
// 4. Implement with_timeout() function
// 5. Implement ProducerScope<T> and send()
// 6. Implement const static member initialization
// 7. Handle range member function vs flux builder
