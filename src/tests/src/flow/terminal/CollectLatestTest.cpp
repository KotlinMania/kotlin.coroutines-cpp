/**
 * Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/CollectLatestTest.kt
 */

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Collect.hpp"
#include "kotlinx/coroutines/Yield.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/EventLoop.hpp"
#include <iostream>
#include <memory>
#include <atomic>

namespace kotlinx {
namespace coroutines {

using namespace flow;
using namespace testing;

class CollectLatestTest : public TestBase {
public:
    // Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/CollectLatestTest.kt:9-14
    // @Test
    void test_no_suspension() {
        run_test([this](CoroutineScope*) {
            auto f = flow_of<int>({1, 2, 3});
            collect_latest<int>(f, [this](int it) {
                expect(it);
            });
            finish(4);
        });
    }

    // Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/CollectLatestTest.kt:17-23
    // @Test
    void test_suspension() {
        run_test([this](CoroutineScope* scope) {
            // Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/CollectLatestTest.kt:18-21
            class YieldExpectStateMachine final : public ContinuationImpl {
                TestBase* test_;
                void* _label = nullptr;
            public:
                YieldExpectStateMachine(TestBase* test, Continuation<void*>* completion)
                    : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)), test_(test) {}

                void* invoke_suspend(Result<void*> result) override {
                    coroutine_begin(this)
                    coroutine_yield(this, yield(shared_from_this()));
                    test_->expect(1);
                    coroutine_end(this)
                }
            };

            auto f = flow_of<int>({1, 2, 3});
            std::atomic<bool> done{false};
            std::exception_ptr error = nullptr;
            auto cont = std::make_shared<FunctionalContinuation<void*>>(
                scope->get_coroutine_context(),
                [&done, &error](Result<void*> res) {
                    if (!res.is_success()) {
                        error = res.exception_or_null();
                    }
                    done.store(true);
                }
            );

            void* r = collect_latest<int>(f, [this](int, Continuation<void*>* c) -> void* {
                auto sm = std::make_shared<YieldExpectStateMachine>(this, c);
                return sm->start(Result<void*>::success(nullptr));
            }, cont.get());

            if (intrinsics::is_coroutine_suspended(r)) {
                auto loop = ThreadLocalEventLoop::current_or_null();
                while (!done.load()) {
                    if (loop && !loop->is_empty()) {
                        loop->process_next_event();
                    } else {
                        std::this_thread::yield();
                    }
                }
            }
            if (error) std::rethrow_exception(error);
            finish(2);
        });
    }

    // Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/CollectLatestTest.kt:26-36
    // @Test
    void test_upstream_error_suspension() {
        run_test(
            [](std::exception_ptr ex) {
                try {
                    if (ex) std::rethrow_exception(ex);
                    return false;
                } catch (const TestException&) {
                    return true;
                } catch (...) {
                    return false;
                }
            },
            [this](CoroutineScope* scope) {
                struct FinallyGuard {
                    TestBase* test;
                    ~FinallyGuard() { test->finish(2); }
                } guard{this};

                auto f = flow::flow<int>([](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
                    collector->emit(1, cont);
                    throw TestException();
                });

                std::atomic<bool> done{false};
                std::exception_ptr error = nullptr;
                auto cont = std::make_shared<FunctionalContinuation<void*>>(
                    scope->get_coroutine_context(),
                    [&done, &error](Result<void*> res) {
                        if (!res.is_success()) {
                            error = res.exception_or_null();
                        }
                        done.store(true);
                    }
                );

                void* r = collect_latest<int>(f, [this](int) {
                    expect(1);
                }, cont.get());

                if (intrinsics::is_coroutine_suspended(r)) {
                    auto loop = ThreadLocalEventLoop::current_or_null();
                    while (!done.load()) {
                        if (loop && !loop->is_empty()) {
                            loop->process_next_event();
                        } else {
                            std::this_thread::yield();
                        }
                    }
                }
                if (error) std::rethrow_exception(error);
                expect_unreached();
            }
        );
    }

    // Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/CollectLatestTest.kt:39-52
    // @Test
    void test_downstream_error() {
        run_test(
            [](std::exception_ptr ex) {
                try {
                    if (ex) std::rethrow_exception(ex);
                    return false;
                } catch (const TestException&) {
                    return true;
                } catch (...) {
                    return false;
                }
            },
            [this](CoroutineScope* scope) {
                struct FinallyGuard {
                    TestBase* test;
                    ~FinallyGuard() { test->finish(2); }
                } guard{this};

                auto f = flow::flow<int>([this](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
                    collector->emit(1, cont);
                    hang([this]() {
                        expect(1);
                    }, cont);
                    return nullptr;
                });

                std::atomic<bool> done{false};
                std::exception_ptr error = nullptr;
                auto cont = std::make_shared<FunctionalContinuation<void*>>(
                    scope->get_coroutine_context(),
                    [&done, &error](Result<void*> res) {
                        if (!res.is_success()) {
                            error = res.exception_or_null();
                        }
                        done.store(true);
                    }
                );

                void* r = collect_latest<int>(f, [](int) {
                    throw TestException();
                }, cont.get());

                if (intrinsics::is_coroutine_suspended(r)) {
                    auto loop = ThreadLocalEventLoop::current_or_null();
                    while (!done.load()) {
                        if (loop && !loop->is_empty()) {
                            loop->process_next_event();
                        } else {
                            std::this_thread::yield();
                        }
                    }
                }
                if (error) std::rethrow_exception(error);
                expect_unreached();
            }
        );
    }
};

} // namespace coroutines
} // namespace kotlinx

int main() {
    using namespace kotlinx::coroutines;
    CollectLatestTest test;
    int failed = 0;

    auto run = [&](const char* name, void (CollectLatestTest::*method)()) {
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

    std::cout << "=== CollectLatestTest ===" << std::endl;
    run("test_no_suspension", &CollectLatestTest::test_no_suspension);
    run("test_suspension", &CollectLatestTest::test_suspension);
    run("test_upstream_error_suspension", &CollectLatestTest::test_upstream_error_suspension);
    run("test_downstream_error", &CollectLatestTest::test_downstream_error);

    std::cout << "=== Results: " << (4 - failed) << "/4 passed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
