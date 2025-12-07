// Transliterated from: reactive/kotlinx-coroutines-reactor/test/ConvertTest.cpp

// TODO: #include <kotlinx/coroutines/testing.hpp>
// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <kotlinx/coroutines/channels.hpp>
// TODO: #include <kotlinx/coroutines/flow.hpp>
// TODO: #include <kotlinx/coroutines/reactive.hpp>
// TODO: #include <org/junit/junit.hpp>
// TODO: #include <org/junit/test.hpp>
// TODO: #include <kotlin/test.hpp>

namespace kotlinx { namespace coroutines { namespace reactor {

class ConvertTest : public TestBase {
public:
    // @Test
    // TODO: implement coroutine suspension
    void test_job_to_mono_success() {
        run_blocking([]() {
            expect(1);
            Job* job = launch([]() {
                expect(3);
            });
            Mono<Unit>* mono = job->as_mono(coroutine_context().minus_key(Job{}));
            mono->subscribe([](Unit) {
                expect(4);
            });
            expect(2);
            yield();
            finish(5);
        });
    }

    // @Test
    // TODO: implement coroutine suspension
    void test_job_to_mono_fail() {
        run_blocking([]() {
            expect(1);
            Deferred<void>* job = async(NonCancellable, []() {
                expect(3);
                throw std::runtime_error("OK");
            });
            Mono<Unit>* mono = job->as_mono(coroutine_context().minus_key(Job{}));
            mono->subscribe(
                    [](Unit) { fail("no item should be emitted"); },
                    [](Throwable*) { expect(4); }
            );
            expect(2);
            yield();
            finish(5);
        });
    }

    // @Test
    void test_deferred_to_mono() {
        Deferred<std::string>* d = GlobalScope->async_([]() {
            delay(50);
            return "OK";
        });
        Mono<std::string>* mono1 = d->as_mono(Dispatchers::Unconfined);
        check_mono_value(mono1, [](std::string it) {
            assert_equals("OK", it);
        });
        Mono<std::string>* mono2 = d->as_mono(Dispatchers::Unconfined);
        check_mono_value(mono2, [](std::string it) {
            assert_equals("OK", it);
        });
    }

    // @Test
    void test_deferred_to_mono_empty() {
        Deferred<std::string*>* d = GlobalScope->async_([]() -> std::string* {
            delay(50);
            return nullptr;
        });
        Mono<std::string>* mono1 = d->as_mono(Dispatchers::Unconfined);
        check_mono_value(mono1, Assert::assert_null);
        Mono<std::string>* mono2 = d->as_mono(Dispatchers::Unconfined);
        check_mono_value(mono2, Assert::assert_null);
    }

    // @Test
    void test_deferred_to_mono_fail() {
        Deferred<void>* d = GlobalScope->async_([]() {
            delay(50);
            throw TestRuntimeException("OK");
        });
        Mono<void>* mono1 = d->as_mono(Dispatchers::Unconfined);
        check_erroneous(mono1, [](Throwable* it) {
            TestRuntimeException* ex = dynamic_cast<TestRuntimeException*>(it);
            if (ex == nullptr || ex->message() != "OK") {
                throw std::runtime_error(it->to_string());
            }
        });
        Mono<void>* mono2 = d->as_mono(Dispatchers::Unconfined);
        check_erroneous(mono2, [](Throwable* it) {
            TestRuntimeException* ex = dynamic_cast<TestRuntimeException*>(it);
            if (ex == nullptr || ex->message() != "OK") {
                throw std::runtime_error(it->to_string());
            }
        });
    }

    // @Test
    void test_to_flux() {
        ReceiveChannel<std::string>* c = GlobalScope->produce([]() {
            delay(50);
            send("O");
            delay(50);
            send("K");
        });
        Flux<std::string>* flux = c->consume_as_flow()->as_flux(Dispatchers::Unconfined);
        check_mono_value(flux->reduce([](std::string t1, std::string t2) { return t1 + t2; }), [](std::string it) {
            assert_equals("OK", it);
        });
    }

    // @Test
    void test_to_flux_fail() {
        ReceiveChannel<std::string>* c = GlobalScope->produce([]() {
            delay(50);
            send("O");
            delay(50);
            throw TestException("K");
        });
        Flux<std::string>* flux = c->consume_as_flow()->as_flux(Dispatchers::Unconfined);
        Mono<std::string>* mono = mono(Dispatchers::Unconfined, []() {
            std::string result = "";
            try {
                flux->collect([&result](std::string it) { result += it; });
            } catch(Throwable* e) {
                TestException* ex = dynamic_cast<TestException*>(e);
                if (ex == nullptr) throw e;
                result += ex->message();
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
// 1. Implement run_blocking() function
// 2. Implement launch() function
// 3. Implement async() function
// 4. Implement Job and Deferred<T> types
// 5. Implement as_mono() extension methods
// 6. Implement coroutine_context() function
// 7. Implement minus_key() method
// 8. Implement subscribe() methods for Mono
// 9. Implement yield() function
// 10. Implement NonCancellable constant
// 11. Implement GlobalScope singleton
// 12. Implement delay() function
// 13. Implement send() function
// 14. Implement consume_as_flow() method
// 15. Implement reduce() method for Flux
// 16. Implement TestException and TestRuntimeException types
// 17. Implement Assert::assert_null function
// 18. Implement fail() function
// 19. Implement Unit type
// 20. Handle nullable return types properly
