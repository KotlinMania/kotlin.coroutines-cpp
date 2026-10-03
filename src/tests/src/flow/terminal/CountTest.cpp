/**
 * Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/CountTest.kt
 */

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Count.hpp"

#include <iostream>
#include <stdexcept>

namespace kotlinx {
namespace coroutines {

using namespace flow;
using namespace testing;

namespace {

int unbox_count(void* ptr) {
    if (ptr == nullptr || intrinsics::is_coroutine_suspended(ptr)) {
        throw std::logic_error("count() returned null or suspended unexpectedly");
    }
    int val = *static_cast<int*>(ptr);
    delete static_cast<int*>(ptr);
    return val;
}

} // namespace

class CountTest : public TestBase {
private:
    class DeferredTestFlow : public Flow<int> {
    public:
        FlowCollector<int>* retained_collector = nullptr;
        Continuation<void*>* retained_completion = nullptr;

        void* collect(FlowCollector<int>* c, Continuation<void*>* cont) override {
            retained_collector = c;
            retained_completion = cont;
            return intrinsics::get_COROUTINE_SUSPENDED();
        }
    };

public:
    // @Test
    void test_count() {
        run_test([](CoroutineScope*) {
            auto f = flow_of({239, 240});
            assert_equals(2, unbox_count(count(f)));
            assert_equals(2, unbox_count(count(f, [](const int&) { return true; })));
            assert_equals(1, unbox_count(count(f, [](const int& it) { return it % 2 == 0; })));
            assert_equals(0, unbox_count(count(f, [](const int&) { return false; })));
        });
    }

    // @Test
    void test_no_values() {
        run_test([](CoroutineScope*) {
            assert_equals(0, unbox_count(count(flow_of<int>({}))));
            assert_equals(0, unbox_count(count(flow_of<int>({}), [](const int&) { return false; })));
            assert_equals(0, unbox_count(count(flow_of<int>({}), [](const int&) { return true; })));
        });
    }

    // @Test
    void test_exception() {
        run_test([](CoroutineScope*) {
            auto f = flow::flow<int>([](FlowCollector<int>*, Continuation<void*>*) -> void* {
                throw TestException();
            });

            assert_fails_with<TestException>([&]() {
                unbox_count(count(f));
            });
            assert_fails_with<TestException>([&]() {
                unbox_count(count(f, [](const int&) { return false; }));
            });
        });
    }

    // @Test
    void test_exception_after_value() {
        run_test([](CoroutineScope*) {
            auto f = flow::flow<int>([](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
                collector->emit(1, cont);
                throw TestException();
            });

            assert_fails_with<TestException>([&]() {
                unbox_count(count(f));
            });
            assert_fails_with<TestException>([&]() {
                unbox_count(count(f, [](const int&) { return false; }));
            });
        });
    }

    // @Test
    void test_suspended_count() {
        run_test([](CoroutineScope*) {
            auto deferred_impl = std::make_shared<DeferredTestFlow>();
            std::shared_ptr<Flow<int>> deferred = deferred_impl;
            int resumed_count = -1;
            bool resumed = false;

            auto cont = make_continuation<void*>(nullptr, [&resumed_count, &resumed](Result<void*> res) {
                resumed = true;
                if (res.is_success()) {
                    auto* p = static_cast<int*>(res.get_or_throw());
                    if (p) {
                        resumed_count = *p;
                        delete p;
                    }
                }
            });

            void* r = count(deferred, cont.get());
            assert_true(intrinsics::is_coroutine_suspended(r));
            assert_true(!resumed);

            assert_not_null(deferred_impl->retained_collector);
            assert_not_null(deferred_impl->retained_completion);

            deferred_impl->retained_collector->emit(10, nullptr);
            deferred_impl->retained_collector->emit(20, nullptr);
            deferred_impl->retained_collector->emit(30, nullptr);

            deferred_impl->retained_completion->resume_with(Result<void*>::success(nullptr));

            assert_true(resumed);
            assert_equals(3, resumed_count);
        });
    }

    // @Test
    void test_suspended_count_predicate() {
        run_test([](CoroutineScope*) {
            auto deferred_impl = std::make_shared<DeferredTestFlow>();
            std::shared_ptr<Flow<int>> deferred = deferred_impl;

            Continuation<void*>* retained_pred_cont = nullptr;
            auto susp_predicate = [&](int val, Continuation<void*>* cont) -> void* {
                if (val == 42) {
                    retained_pred_cont = cont;
                    return intrinsics::get_COROUTINE_SUSPENDED();
                }
                return new bool(false);
            };

            int resumed_count = -1;
            bool resumed = false;
            auto cont = make_continuation<void*>(nullptr, [&resumed_count, &resumed](Result<void*> res) {
                resumed = true;
                if (res.is_success()) {
                    auto* p = static_cast<int*>(res.get_or_throw());
                    if (p) {
                        resumed_count = *p;
                        delete p;
                    }
                }
            });

            void* r = count(deferred, susp_predicate, cont.get());
            assert_true(intrinsics::is_coroutine_suspended(r));
            assert_true(!resumed);

            assert_not_null(deferred_impl->retained_collector);
            assert_not_null(deferred_impl->retained_completion);

            bool emit_resumed = false;
            auto emit_cont = make_continuation<void*>(nullptr, [&emit_resumed](Result<void*>) {
                emit_resumed = true;
            });

            void* emit_r = deferred_impl->retained_collector->emit(42, emit_cont.get());
            assert_true(intrinsics::is_coroutine_suspended(emit_r));
            assert_true(!emit_resumed);
            assert_not_null(retained_pred_cont);

            // Resume the predicate with true
            retained_pred_cont->resume_with(Result<void*>::success(new bool(true)));

            // emit_cont should now have resumed
            assert_true(emit_resumed);

            // Flow now completes
            deferred_impl->retained_completion->resume_with(Result<void*>::success(nullptr));

            assert_true(resumed);
            assert_equals(1, resumed_count);
        });
    }

    // @Test
    void test_suspended_failure_cleanup() {
        run_test([](CoroutineScope*) {
            auto deferred_impl = std::make_shared<DeferredTestFlow>();
            std::shared_ptr<Flow<int>> deferred = deferred_impl;
            bool resumed = false;
            bool failed = false;

            auto cont = make_continuation<void*>(nullptr, [&resumed, &failed](Result<void*> res) {
                resumed = true;
                if (res.is_failure()) {
                    failed = true;
                }
            });

            void* r = count(deferred, cont.get());
            assert_true(intrinsics::is_coroutine_suspended(r));
            assert_true(!resumed);

            deferred_impl->retained_completion->resume_with(
                Result<void*>::failure(std::make_exception_ptr(std::runtime_error("flow error"))));

            assert_true(resumed);
            assert_true(failed);
        });
    }
};

} // namespace coroutines
} // namespace kotlinx

int main() {
    using namespace kotlinx::coroutines;
    CountTest test;
    int failed = 0;

    auto run = [&](const char* name, void (CountTest::*method)()) {
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

    std::cout << "=== CountTest ===" << std::endl;
    run("test_count", &CountTest::test_count);
    run("test_no_values", &CountTest::test_no_values);
    run("test_exception", &CountTest::test_exception);
    run("test_exception_after_value", &CountTest::test_exception_after_value);
    run("test_suspended_count", &CountTest::test_suspended_count);
    run("test_suspended_count_predicate", &CountTest::test_suspended_count_predicate);
    run("test_suspended_failure_cleanup", &CountTest::test_suspended_failure_cleanup);

    std::cout << "=== Results: " << (7 - failed) << "/7 passed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}