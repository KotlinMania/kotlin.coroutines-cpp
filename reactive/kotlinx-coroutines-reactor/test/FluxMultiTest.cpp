// Transliterated from: reactive/kotlinx-coroutines-reactor/test/FluxMultiTest.cpp

// TODO: #include <kotlinx/coroutines/testing.hpp>
// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <kotlinx/coroutines/reactive.hpp>
// TODO: #include <org/junit/junit.hpp>
// TODO: #include <org/junit/test.hpp>
// TODO: #include <reactor/core/publisher.hpp>
// TODO: #include <java/io/io_exception.hpp>
// TODO: #include <kotlin/test.hpp>

namespace kotlinx { namespace coroutines { namespace reactor {

class FluxMultiTest : public TestBase {
public:
    // @Test
    void test_numbers() {
        int n = 100 * stress_test_multiplier;
        Flux<int>* flux = flux<int>([n](ProducerScope<int>& producer) {
            for (int i = 0; i < n; ++i) {
                producer.send(i);
            }
        });
        check_mono_value(flux->collect_list(), [n](std::vector<int> list) {
            std::vector<int> expected(n);
            std::iota(expected.begin(), expected.end(), 0);
            assert_equals(expected, list);
        });
    }

    // @Test
    void test_concurrent_stress() {
        int n = 10000 * stress_test_multiplier;
        Flux<int>* flux = flux<int>([n](ProducerScope<int>& producer) {
            // concurrent emitters (many coroutines)
            std::vector<Job*> jobs;
            for (int i = 0; i < n; ++i) {
                jobs.push_back(launch([&producer, i]() {
                    producer.send(i);
                }));
            }
            for (Job* job : jobs) {
                job->join();
            }
        });
        check_mono_value(flux->collect_list(), [n](std::vector<int> list) {
            assert_equals(n, list.size());
            std::vector<int> expected(n);
            std::iota(expected.begin(), expected.end(), 0);
            std::vector<int> sorted_list = list;
            std::sort(sorted_list.begin(), sorted_list.end());
            assert_equals(expected, sorted_list);
        });
    }

    // @Test
    void test_iterator_resend_unconfined() {
        int n = 10000 * stress_test_multiplier;
        Flux<int>* flux = flux<int>(Dispatchers::Unconfined, [n](ProducerScope<int>& producer) {
            Flux<int>::range(0, n)->collect([&producer](int it) { producer.send(it); });
        });
        check_mono_value(flux->collect_list(), [n](std::vector<int> list) {
            std::vector<int> expected(n);
            std::iota(expected.begin(), expected.end(), 0);
            assert_equals(expected, list);
        });
    }

    // @Test
    void test_iterator_resend_pool() {
        int n = 10000 * stress_test_multiplier;
        Flux<int>* flux = flux<int>([n](ProducerScope<int>& producer) {
            Flux<int>::range(0, n)->collect([&producer](int it) { producer.send(it); });
        });
        check_mono_value(flux->collect_list(), [n](std::vector<int> list) {
            std::vector<int> expected(n);
            std::iota(expected.begin(), expected.end(), 0);
            assert_equals(expected, list);
        });
    }

    // @Test
    void test_send_and_crash() {
        Flux<std::string>* flux = flux<std::string>([](ProducerScope<std::string>& producer) {
            producer.send("O");
            throw IOException("K");
        });
        Mono<std::string>* mono = mono([]() {
            std::string result = "";
            try {
                flux->collect([&result](std::string it) { result += it; });
            } catch(IOException& e) {
                result += e.message();
            }
            return result;
        });
        check_mono_value(mono, [](std::string it) {
            assert_equals("OK", it);
        });
    }
};

} } } // namespace kotlinx::coroutines::reactor

// TODO: Semantic implementation tasks:
// 1. Implement std::iota equivalent or use <numeric>
// 2. Implement Flux::range() static method
// 3. Implement IOException class
// 4. Implement message() method on exceptions
// 5. Implement std::vector comparison
