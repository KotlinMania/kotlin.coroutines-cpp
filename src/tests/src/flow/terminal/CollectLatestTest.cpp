/**
 * Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/CollectLatestTest.kt
 */

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Collect.hpp"
#include "kotlinx/coroutines/Yield.hpp"
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
            struct YieldExpectStateMachine : public Continuation<void*>,
                                             public std::enable_shared_from_this<YieldExpectStateMachine> {
                TestBase* test_;
                Continuation<void*>* cont_;
                int state_ = 0;

                YieldExpectStateMachine(TestBase* test, Continuation<void*>* cont)
                    : test_(test), cont_(cont) {}

                std::shared_ptr<CoroutineContext> get_context() const override {
                    return cont_ ? cont_->get_context() : nullptr;
                }

                void resume_with(Result<void*> result) override {
                    if (!result.is_success()) {
                        if (cont_) cont_->resume_with(result);
                        return;
                    }
                    execute();
                }

                void* execute() {
                    if (state_ == 0) {
                        state_ = 1;
                        if (cont_ && cont_->get_context()) {
                            auto job = std::dynamic_pointer_cast<Job>(cont_->get_context()->get(Job::type_key));
                            if (job && !job->is_active()) {
                                if (cont_) cont_->resume_with(Result<void*>::failure(job->get_cancellation_exception()));
                                return nullptr;
                            }
                        }
                        auto self = shared_from_this();
                        void* res = yield(self);
                        if (intrinsics::is_coroutine_suspended(res)) {
                            return intrinsics::get_COROUTINE_SUSPENDED();
                        }
                    }
                    if (state_ == 1) {
                        state_ = 2;
                        if (cont_ && cont_->get_context()) {
                            auto job = std::dynamic_pointer_cast<Job>(cont_->get_context()->get(Job::type_key));
                            if (job && !job->is_active()) {
                                if (cont_) cont_->resume_with(Result<void*>::failure(job->get_cancellation_exception()));
                                return nullptr;
                            }
                        }
                        test_->expect(1);
                        if (cont_) cont_->resume_with(Result<void*>::success(nullptr));
                        return nullptr;
                    }
                    return nullptr;
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
                return sm->execute();
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
