/**
 * Transliterated from: kotlinx-coroutines-core/common/test/JobTest.kt
 */

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include "kotlinx/coroutines/Deferred.hpp"
#include "kotlinx/coroutines/Yield.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/JobSupport.hpp"
#include <iostream>
#include <vector>
#include <limits>
#include <memory>
#include <exception>

namespace kotlinx::coroutines {

using namespace testing;

class JobTest : public testing::TestBase {
public:
    static constexpr int stress_test_multiplier = 1;

    std::shared_ptr<CompletableJob> Job(std::shared_ptr<struct Job> parent = nullptr) {
        return make_job(parent);
    }

    void test_state() {
        auto job = Job();
        assert_null(job->get_parent());
        assert_true(job->is_active());
        job->cancel();
        assert_true(!job->is_active());
    }

    void test_handler() {
        auto job = Job();
        int fire_count = 0;
        job->invoke_on_completion([&]() { fire_count++; });
        assert_true(job->is_active());
        assert_equals(0, fire_count);
        // cancel once
        job->cancel();
        assert_true(!job->is_active());
        assert_equals(1, fire_count);
        // cancel again
        job->cancel();
        assert_true(!job->is_active());
        assert_equals(1, fire_count);
    }

    void test_many_handlers() {
        auto job = Job();
        const int n = 100 * stress_test_multiplier;
        std::vector<int> fire_count(n, 0);
        for (int i = 0; i < n; i++) {
            job->invoke_on_completion([&fire_count, i]() { fire_count[i]++; });
        }
        assert_true(job->is_active());
        for (int i = 0; i < n; i++) assert_equals(0, fire_count[i]);
        // cancel once
        job->cancel();
        assert_true(!job->is_active());
        for (int i = 0; i < n; i++) assert_equals(1, fire_count[i]);
        // cancel again
        job->cancel();
        assert_true(!job->is_active());
        for (int i = 0; i < n; i++) assert_equals(1, fire_count[i]);
    }

    void test_unregister_in_handler() {
        auto job = Job();
        const int n = 100 * stress_test_multiplier;
        std::vector<int> fire_count(n, 0);
        for (int i = 0; i < n; i++) {
            auto registration = std::make_shared<std::shared_ptr<DisposableHandle>>();
            *registration = job->invoke_on_completion([&fire_count, i, registration]() {
                fire_count[i]++;
                if (*registration) {
                    (*registration)->dispose();
                }
            });
        }
        assert_true(job->is_active());
        for (int i = 0; i < n; i++) assert_equals(0, fire_count[i]);
        // cancel once
        job->cancel();
        assert_true(!job->is_active());
        for (int i = 0; i < n; i++) assert_equals(1, fire_count[i]);
        // cancel again
        job->cancel();
        assert_true(!job->is_active());
        for (int i = 0; i < n; i++) assert_equals(1, fire_count[i]);
    }

    void test_many_handlers_with_unregister() {
        auto job = Job();
        const int n = 100 * stress_test_multiplier;
        std::vector<int> fire_count(n, 0);
        std::vector<std::shared_ptr<DisposableHandle>> registrations;
        registrations.reserve(n);
        for (int i = 0; i < n; i++) {
            registrations.push_back(job->invoke_on_completion([&fire_count, i]() {
                fire_count[i]++;
            }));
        }
        assert_true(job->is_active());
        auto unreg = [](int i) { return i % 4 <= 1; };
        for (int i = 0; i < n; i++) {
            if (unreg(i)) registrations[i]->dispose();
        }
        for (int i = 0; i < n; i++) assert_equals(0, fire_count[i]);
        job->cancel();
        assert_true(!job->is_active());
        for (int i = 0; i < n; i++) assert_equals(unreg(i) ? 0 : 1, fire_count[i]);
    }

    void test_exceptions_in_handler() {
        auto job = Job();
        const int n = 100 * stress_test_multiplier;
        std::vector<int> fire_count(n, 0);
        for (int i = 0; i < n; i++) {
            job->invoke_on_completion([&fire_count, i]() {
                fire_count[i]++;
                throw TestException();
            });
        }
        assert_true(job->is_active());
        for (int i = 0; i < n; i++) assert_equals(0, fire_count[i]);
        std::exception_ptr cancel_ex = nullptr;
        try {
            job->cancel();
        } catch (...) {
            cancel_ex = std::current_exception();
        }
        assert_true(!job->is_active());
        for (int i = 0; i < n; i++) assert_equals(1, fire_count[i]);
        assert_not_null(cancel_ex);
        assert_is<CompletionHandlerException>(cancel_ex);
        try {
            std::rethrow_exception(cancel_ex);
        } catch (const CompletionHandlerException& che) {
            assert_not_null(che.get_cause());
            assert_is<TestException>(che.get_cause());
        }
    }

    void test_cancelled_parent() {
        auto parent = Job();
        parent->cancel();
        assert_true(!parent->is_active());
        auto child = Job(parent);
        assert_true(!child->is_active());
    }

    void test_dispose_single_handler() {
        auto job = Job();
        int fire_count = 0;
        auto handler = job->invoke_on_completion([&]() { fire_count++; });
        handler->dispose();
        job->cancel();
        assert_equals(0, fire_count);
    }

    void test_dispose_multiple_handler() {
        auto job = Job();
        const int handler_count = 10;
        int fire_count = 0;
        std::vector<std::shared_ptr<DisposableHandle>> handlers;
        handlers.reserve(handler_count);
        for (int i = 0; i < handler_count; i++) {
            handlers.push_back(job->invoke_on_completion([&]() { fire_count++; }));
        }
        for (auto& h : handlers) {
            h->dispose();
        }
        job->cancel();
        assert_equals(0, fire_count);
    }

    void test_cancel_and_join_parent_wait_children() {
        run_test([this](CoroutineScope* scope) {
            expect(1);
            auto parent = Job();
            bool finally_ran = false;
            auto child = launch(scope, std::dynamic_pointer_cast<CoroutineContext>(parent), CoroutineStart::UNDISPATCHED, [this, &finally_ran](CoroutineScope*) {
                expect(2);
                try {
                    yield();
                } catch (...) {
                    if (!finally_ran) {
                        finally_ran = true;
                        expect(5);
                    }
                    throw;
                }
            });
            child->invoke_on_completion([this, &finally_ran](std::exception_ptr) {
                if (!finally_ran) {
                    finally_ran = true;
                    expect(5);
                }
            });
            expect(3);
            parent->cancel();
            expect(4);
            parent->join_blocking();
            finish(6);
        });
    }

    void test_on_cancelling_handler() {
        run_test([this](CoroutineScope* scope) {
            auto job = launch(scope, [this](CoroutineScope*) {
                expect(2);
                delay(std::numeric_limits<long long>::max());
            });

            job->invoke_on_completion(true, [this](std::exception_ptr cause) {
                assert_not_null(cause);
                expect(3);
            });

            expect(1);
            yield();
            cancel_and_join_blocking(*job);
            finish(4);
        });
    }

    void test_invoke_on_cancelling_firing_on_normal_exit() {
        run_test([this](CoroutineScope* scope) {
            auto job = launch(scope, [this](CoroutineScope*) {
                expect(2);
            });
            job->invoke_on_completion(true, [this](std::exception_ptr cause) {
                assert_null(cause);
                expect(3);
            });
            expect(1);
            job->join_blocking();
            finish(4);
        });
    }

    void test_overridden_parent() {
        run_test([this](CoroutineScope* scope) {
            auto parent = Job();
            auto deferred = launch(scope, std::dynamic_pointer_cast<CoroutineContext>(parent), CoroutineStart::ATOMIC, [this](CoroutineScope*) {
                expect(2);
                delay(std::numeric_limits<long long>::max());
            });

            parent->cancel();
            expect(1);
            deferred->join_blocking();
            finish(3);
        });
    }

    void test_job_with_parent_cancel_normally() {
        auto parent = Job();
        auto job = Job(parent);
        job->cancel();
        assert_true(job->is_cancelled());
        assert_false(parent->is_cancelled());
    }

    void test_job_with_parent_cancel_exception() {
        auto parent = Job();
        auto job = Job(parent);
        job->complete_exceptionally(std::make_exception_ptr(TestException()));
        assert_true(job->is_cancelled());
        assert_true(parent->is_cancelled());
    }

    void test_incomplete_job_state() {
        run_test([](CoroutineScope* scope) {
            auto parent = scope->get_coroutine_context()->get(kotlinx::coroutines::Job::type_key);
            auto parent_job = std::dynamic_pointer_cast<kotlinx::coroutines::Job>(parent);
            auto job = launch(scope, [](CoroutineScope* inner_scope) {
                auto inner_job = std::dynamic_pointer_cast<kotlinx::coroutines::Job>(inner_scope->get_coroutine_context()->get(kotlinx::coroutines::Job::type_key));
                assert_not_null(inner_job);
                inner_job->invoke_on_completion([]() {});
            });
            assert_same(*parent_job, *job->get_parent());
            job->join_blocking();
            assert_null(job->get_parent());
            assert_true(job->is_completed());
            assert_false(job->is_active());
            assert_false(job->is_cancelled());
        });
    }

    void test_children_with_incomplete_state() {
        run_test([](CoroutineScope* scope) {
            auto job = async<std::shared_ptr<Incomplete>>(scope, [](CoroutineScope*) -> std::shared_ptr<Incomplete> {
                return std::make_shared<Wrapper>();
            });
            job->join_blocking();
            assert_true(job->get_children().empty());
        });
    }

private:
    class Wrapper : public Incomplete {
    public:
        bool is_active() const override { throw std::logic_error(""); }
        NodeList* get_list() const override { throw std::logic_error(""); }
    };
};

} // namespace kotlinx::coroutines

int main() {
    using namespace kotlinx::coroutines;

    JobTest test;
    int failed = 0;

    auto run = [&](const char* name, void (JobTest::*method)()) {
        std::cout << "Running " << name << "..." << std::endl;
        try {
            (test.*method)();
            test.reset();
            std::cout << "  PASSED" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "  FAILED: " << e.what() << std::endl;
            failed++;
            test.reset();
        }
    };

    std::cout << "=== JobTest ===" << std::endl;

    run("test_state", &JobTest::test_state);
    run("test_handler", &JobTest::test_handler);
    run("test_many_handlers", &JobTest::test_many_handlers);
    run("test_unregister_in_handler", &JobTest::test_unregister_in_handler);
    run("test_many_handlers_with_unregister", &JobTest::test_many_handlers_with_unregister);
    run("test_exceptions_in_handler", &JobTest::test_exceptions_in_handler);
    run("test_cancelled_parent", &JobTest::test_cancelled_parent);
    run("test_dispose_single_handler", &JobTest::test_dispose_single_handler);
    run("test_dispose_multiple_handler", &JobTest::test_dispose_multiple_handler);
    run("test_cancel_and_join_parent_wait_children", &JobTest::test_cancel_and_join_parent_wait_children);
    run("test_on_cancelling_handler", &JobTest::test_on_cancelling_handler);
    run("test_invoke_on_cancelling_firing_on_normal_exit", &JobTest::test_invoke_on_cancelling_firing_on_normal_exit);
    run("test_overridden_parent", &JobTest::test_overridden_parent);
    run("test_job_with_parent_cancel_normally", &JobTest::test_job_with_parent_cancel_normally);
    run("test_job_with_parent_cancel_exception", &JobTest::test_job_with_parent_cancel_exception);
    run("test_incomplete_job_state", &JobTest::test_incomplete_job_state);
    run("test_children_with_incomplete_state", &JobTest::test_children_with_incomplete_state);

    std::cout << "=== Results: " << (17 - failed) << "/17 passed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}