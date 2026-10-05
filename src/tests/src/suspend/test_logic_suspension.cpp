// port-lint: tests flow/operators/BooleanTerminationTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/BooleanTerminationTest.kt
/** Continuation ABI regressions for Logic.hpp any, all, and none operators. */

#include "kotlinx/coroutines/flow/Logic.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::flow;
using namespace kotlinx::coroutines::testing;

namespace {

struct Completion final : Continuation<void*> {
    std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance();
    int resumes = 0;
    Result<void*> result = Result<void*>::success(nullptr);

    std::shared_ptr<CoroutineContext> get_context() const override { return context; }
    void resume_with(Result<void*> outcome) override {
        ++resumes;
        result = std::move(outcome);
    }
};

struct Pending {
    Continuation<void*>* continuation = nullptr;
    std::weak_ptr<BaseContinuationImpl> frame;

    void* suspend(Continuation<void*>* value) {
        assert_true(continuation == nullptr);
        continuation = value;
        if (auto* base = dynamic_cast<BaseContinuationImpl*>(value)) {
            frame = base->weak_from_this();
        }
        return intrinsics::get_COROUTINE_SUSPENDED();
    }

    void resume(Result<void*> result = Result<void*>::success(nullptr)) {
        auto* target = std::exchange(continuation, nullptr);
        assert_true(target != nullptr);
        target->resume_with(std::move(result));
    }
};

struct Trace {
    int completed_emits = 0;
    int finally_calls = 0;
    std::exception_ptr failure;
    bool pause_before_first = false;
    bool pause_after_first = false;
    Pending upstream_before;
    Pending upstream_after;
    std::weak_ptr<BaseContinuationImpl> frame;
};

std::shared_ptr<Flow<int>> source(Trace& trace, int count = 3) {
    return kotlinx::coroutines::flow::flow<int>([&trace, count](FlowCollector<int>* collector, Continuation<void*>* completion) -> void* {
        class Frame final : public ContinuationImpl {
        public:
            Frame(Trace& trace, int count, FlowCollector<int>* collector, Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  trace_(trace), count_(count), collector_(collector) {}
            void retain() { self_ref_ = shared_from_this(); }
            void* invoke_suspend(Result<void*> result) override {
                try {
                    coroutine_begin(this)
                    if (trace_.pause_before_first) {
                        coroutine_yield(this, trace_.upstream_before.suspend(this));
                    }
                    for (value_ = 1; value_ <= count_; ++value_) {
                        coroutine_yield(this, collector_->emit(value_, this));
                        ++trace_.completed_emits;
                        if (trace_.pause_after_first && value_ == 1 && value_ < count_) {
                            coroutine_yield(this, trace_.upstream_after.suspend(this));
                        }
                    }
                    ++trace_.finally_calls;
                    coroutine_end(this)
                } catch (...) {
                    trace_.failure = std::current_exception();
                    ++trace_.finally_calls;
                    throw;
                }
            }
        protected:
            void release_intercepted() override {
                ContinuationImpl::release_intercepted();
                self_ref_.reset();
            }
        private:
            void* _label = nullptr;
            Trace& trace_;
            int count_;
            FlowCollector<int>* collector_;
            int value_ = 0;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        auto frame = std::make_shared<Frame>(trace, count, collector, completion);
        trace.frame = frame;
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

std::shared_ptr<Flow<int>> infinite_flow(int val = 5) {
    return kotlinx::coroutines::flow::flow<int>([val](FlowCollector<int>* collector, Continuation<void*>* completion) -> void* {
        class Frame final : public ContinuationImpl {
        public:
            Frame(FlowCollector<int>* collector, int val, Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  collector_(collector), val_(val) {}
            void retain() { self_ref_ = shared_from_this(); }
            void* invoke_suspend(Result<void*> result) override {
                coroutine_begin(this)
                while (true) {
                    coroutine_yield(this, collector_->emit(val_, this));
                }
                coroutine_end(this)
            }
        protected:
            void release_intercepted() override {
                ContinuationImpl::release_intercepted();
                self_ref_.reset();
            }
        private:
            void* _label = nullptr;
            FlowCollector<int>* collector_;
            int val_;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        auto frame = std::make_shared<Frame>(collector, val, completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

std::shared_ptr<Flow<int>> short_circuit_source(bool& unreached_hit) {
    return kotlinx::coroutines::flow::flow<int>([&unreached_hit](FlowCollector<int>* collector, Continuation<void*>* completion) -> void* {
        class Frame final : public ContinuationImpl {
        public:
            Frame(FlowCollector<int>* collector, bool& unreached, Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  collector_(collector), unreached_(unreached) {}
            void retain() { self_ref_ = shared_from_this(); }
            void* invoke_suspend(Result<void*> result) override {
                coroutine_begin(this)
                coroutine_yield(this, collector_->emit(1, this));
                coroutine_yield(this, collector_->emit(2, this));
                unreached_ = true;
                assert_true(false); // expectUnreached()
                coroutine_end(this)
            }
        protected:
            void release_intercepted() override {
                ContinuationImpl::release_intercepted();
                self_ref_.reset();
            }
        private:
            void* _label = nullptr;
            FlowCollector<int>* collector_;
            bool& unreached_;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        auto frame = std::make_shared<Frame>(collector, unreached_hit, completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

bool await_result(void* res, Completion& comp) {
    if (intrinsics::is_coroutine_suspended(res)) {
        assert_equals(1, comp.resumes);
        assert_true(comp.result.is_success());
        void* ptr = comp.result.get_or_throw();
        assert_true(ptr != nullptr);
        bool val = *static_cast<bool*>(ptr);
        delete static_cast<bool*>(ptr);
        return val;
    } else {
        assert_equals(0, comp.resumes);
        assert_true(res != nullptr);
        bool val = *static_cast<bool*>(res);
        delete static_cast<bool*>(res);
        return val;
    }
}

template <typename T, typename Pred>
bool call_any(std::shared_ptr<Flow<T>> f, Pred p) {
    Completion comp;
    void* r = any<T>(f, std::move(p), &comp);
    return await_result(r, comp);
}

template <typename T, typename Pred>
bool call_all(std::shared_ptr<Flow<T>> f, Pred p) {
    Completion comp;
    void* r = all<T>(f, std::move(p), &comp);
    return await_result(r, comp);
}

template <typename T, typename Pred>
bool call_none(std::shared_ptr<Flow<T>> f, Pred p) {
    Completion comp;
    void* r = none<T>(f, std::move(p), &comp);
    return await_result(r, comp);
}

// =============================================================================
// Ported from BooleanTerminationTest.kt (all 12 tests)
// =============================================================================

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/BooleanTerminationTest.kt:8-17
void test_any_nominal() {
    auto f = as_flow(std::vector<int>{1, 2});
    assert_true(call_any(f, [](int it) { return it > 0; }));
    assert_true(call_any(f, [](int it) { return it % 2 == 0; }));
    assert_false(call_any(f, [](int it) { return it > 5; }));
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/BooleanTerminationTest.kt:20-22
void test_any_empty() {
    assert_false(call_any(empty_flow<int>(), [](int it) { return it > 0; }));
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/BooleanTerminationTest.kt:25-27
void test_any_infinite() {
    assert_true(call_any(infinite_flow(5), [](int it) { return it == 5; }));
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/BooleanTerminationTest.kt:30-38
void test_any_short_circuit() {
    bool unreached = false;
    assert_true(call_any(short_circuit_source(unreached), [](int it) { return it == 2; }));
    assert_false(unreached);
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/BooleanTerminationTest.kt:41-50
void test_all_nominal() {
    auto f = as_flow(std::vector<int>{1, 2});
    assert_true(call_all(f, [](int it) { return it > 0; }));
    assert_false(call_all(f, [](int it) { return it % 2 == 0; }));
    assert_false(call_all(f, [](int it) { return it > 5; }));
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/BooleanTerminationTest.kt:53-55
void test_all_empty() {
    assert_true(call_all(empty_flow<int>(), [](int it) { return it > 0; }));
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/BooleanTerminationTest.kt:58-60
void test_all_infinite() {
    assert_false(call_all(infinite_flow(5), [](int it) { return it == 0; }));
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/BooleanTerminationTest.kt:63-71
void test_all_short_circuit() {
    bool unreached = false;
    assert_false(call_all(short_circuit_source(unreached), [](int it) { return it <= 1; }));
    assert_false(unreached);
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/BooleanTerminationTest.kt:74-83
void test_none_nominal() {
    auto f = as_flow(std::vector<int>{1, 2});
    assert_false(call_none(f, [](int it) { return it > 0; }));
    assert_false(call_none(f, [](int it) { return it % 2 == 0; }));
    assert_true(call_none(f, [](int it) { return it > 5; }));
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/BooleanTerminationTest.kt:86-88
void test_none_empty() {
    assert_true(call_none(empty_flow<int>(), [](int it) { return it > 0; }));
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/BooleanTerminationTest.kt:91-93
void test_none_infinite() {
    assert_false(call_none(infinite_flow(5), [](int it) { return it == 5; }));
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/BooleanTerminationTest.kt:96-104
void test_none_short_circuit() {
    bool unreached = false;
    assert_false(call_none(short_circuit_source(unreached), [](int it) { return it == 2; }));
    assert_false(unreached);
}

// =============================================================================
// Deterministic Continuation Regressions
// =============================================================================

void test_immediate_results_and_no_callback() {
    auto f = as_flow(std::vector<int>{1, 2, 3});
    for (int op = 0; op < 3; ++op) {
        Completion comp;
        void* r = op == 0 ? any<int>(f, [](int x) { return x == 2; }, &comp)
                : op == 1 ? all<int>(f, [](int x) { return x > 0; }, &comp)
                          : none<int>(f, [](int x) { return x > 5; }, &comp);
        assert_false(intrinsics::is_coroutine_suspended(r));
        assert_equals(0, comp.resumes);
        assert_true(r != nullptr);
        bool val = *static_cast<bool*>(r);
        delete static_cast<bool*>(r);
        assert_true(val);
    }
}

void test_delayed_predicate_suspensions() {
    for (int op = 0; op < 3; ++op) {
        for (int outcome = 0; outcome < 2; ++outcome) {
            bool target_flag = (outcome == 1);
            // Flow with 2 elements to exercise multiple sequential delayed predicates
            auto f = as_flow(std::vector<int>{10, 20});
            Pending p1;
            Pending p2;
            Completion comp;

            int invocation = 0;
            void* r = op == 0 ? any<int>(f, [&](int, Continuation<void*>* cont) -> void* {
                                    ++invocation;
                                    return invocation == 1 ? p1.suspend(cont) : p2.suspend(cont);
                                }, &comp)
                    : op == 1 ? all<int>(f, [&](int, Continuation<void*>* cont) -> void* {
                                    ++invocation;
                                    return invocation == 1 ? p1.suspend(cont) : p2.suspend(cont);
                                }, &comp)
                              : none<int>(f, [&](int, Continuation<void*>* cont) -> void* {
                                    ++invocation;
                                    return invocation == 1 ? p1.suspend(cont) : p2.suspend(cont);
                                }, &comp);

            assert_true(intrinsics::is_coroutine_suspended(r));
            assert_equals(0, comp.resumes);
            assert_equals(1, invocation);
            assert_false(p1.frame.expired());

            // Resume first predicate:
            // For op == 0 (any): resume false -> non-matching, continues to second element
            // For op == 1 (all): resume true -> matching, continues to second element
            // For op == 2 (none): resume false -> non-matching, continues to second element
            bool r1 = (op == 1) ? true : false;
            p1.resume(Result<void*>::success(new bool(r1)));

            // Now second predicate should be reached and suspended
            assert_equals(0, comp.resumes);
            assert_equals(2, invocation);
            assert_false(p2.frame.expired());

            // Resume second predicate with value corresponding to target_flag:
            // For op == 0 (any): target_flag==true -> resume true (found=true); target_flag==false -> resume false (found=false)
            // For op == 1 (all): target_flag==true -> resume true (counter=false, returns true); target_flag==false -> resume false (counter=true, returns false)
            // For op == 2 (none): target_flag==true (none=true) -> resume false (any=false); target_flag==false (none=false) -> resume true (any=true)
            bool r2 = false;
            if (op == 0) r2 = target_flag;
            else if (op == 1) r2 = target_flag;
            else r2 = !target_flag;

            p2.resume(Result<void*>::success(new bool(r2)));

            assert_equals(1, comp.resumes);
            assert_true(comp.result.is_success());
            void* ptr = comp.result.get_or_throw();
            assert_true(ptr != nullptr);
            bool val = *static_cast<bool*>(ptr);
            delete static_cast<bool*>(ptr);
            assert_equals(target_flag, val);

            assert_true(p1.frame.expired());
            assert_true(p2.frame.expired());
        }
    }
}

void test_short_circuit_at_exact_value() {
    for (int op = 0; op < 3; ++op) {
        int predicate_calls = 0;
        auto f = as_flow(std::vector<int>{1, 2, 3, 4, 5});
        bool val = false;
        if (op == 0) {
            val = call_any(f, [&](int x) {
                ++predicate_calls;
                return x == 2;
            });
            assert_true(val);
            assert_equals(2, predicate_calls);
        } else if (op == 1) {
            val = call_all(f, [&](int x) {
                ++predicate_calls;
                return x < 3;
            });
            assert_false(val);
            assert_equals(3, predicate_calls);
        } else {
            val = call_none(f, [&](int x) {
                ++predicate_calls;
                return x == 3;
            });
            assert_false(val);
            assert_equals(3, predicate_calls);
        }
    }
}

void test_upstream_suspension_and_finally_ordering() {
    for (int op = 0; op < 3; ++op) {
        // 1. Upstream suspension BEFORE the first emit
        {
            Trace trace;
            trace.pause_before_first = true;
            Completion comp;
            auto f = source(trace, 3);
            void* r = op == 0 ? any<int>(f, [](int x) { return x == 1; }, &comp)
                    : op == 1 ? all<int>(f, [](int x) { return x > 0; }, &comp)
                              : none<int>(f, [](int x) { return x == 1; }, &comp);
            assert_true(intrinsics::is_coroutine_suspended(r));
            assert_equals(0, comp.resumes);
            assert_equals(0, trace.completed_emits);
            assert_equals(0, trace.finally_calls);

            trace.upstream_before.resume();
            assert_equals(1, comp.resumes);
            assert_equals(1, trace.finally_calls);
            void* ptr = comp.result.get_or_throw();
            delete static_cast<bool*>(ptr);
            assert_true(trace.frame.expired());
        }

        // 2. Upstream suspension AFTER a successful non-terminating emission
        {
            Trace trace;
            trace.pause_after_first = true;
            Completion comp;
            auto f = source(trace, 2);
            // Non-terminating on value 1:
            // op 0 (any): x > 5 (neither value terminates)
            // op 1 (all): x > 0 (1 satisfies, continues)
            // op 2 (none): x > 5 (neither value terminates)
            void* r = op == 0 ? any<int>(f, [](int x) { return x > 5; }, &comp)
                    : op == 1 ? all<int>(f, [](int x) { return x > 0; }, &comp)
                              : none<int>(f, [](int x) { return x > 5; }, &comp);
            assert_true(intrinsics::is_coroutine_suspended(r));
            assert_equals(0, comp.resumes);
            assert_equals(1, trace.completed_emits);
            assert_equals(0, trace.finally_calls);

            trace.upstream_after.resume();
            assert_equals(1, comp.resumes);
            assert_equals(2, trace.completed_emits);
            assert_equals(1, trace.finally_calls);
            void* ptr = comp.result.get_or_throw();
            delete static_cast<bool*>(ptr);
            assert_true(trace.frame.expired());
        }
    }
}

void test_resumed_failures_propagation() {
    for (int op = 0; op < 3; ++op) {
        // 1. Resumed predicate failure
        {
            Pending pending;
            Completion comp;
            auto f = as_flow(std::vector<int>{1});
            auto failure = std::make_exception_ptr(TestException("predicate error"));
            void* r = op == 0 ? any<int>(f, [&](int, Continuation<void*>* cont) { return pending.suspend(cont); }, &comp)
                    : op == 1 ? all<int>(f, [&](int, Continuation<void*>* cont) { return pending.suspend(cont); }, &comp)
                              : none<int>(f, [&](int, Continuation<void*>* cont) { return pending.suspend(cont); }, &comp);
            assert_true(intrinsics::is_coroutine_suspended(r));
            pending.resume(Result<void*>::failure(failure));
            assert_equals(1, comp.resumes);
            assert_true(comp.result.is_failure());
            assert_true(comp.result.exception_or_null() == failure);
            assert_true(pending.frame.expired());
        }

        // 2. Resumed upstream failure BEFORE first emit
        {
            Trace trace;
            trace.pause_before_first = true;
            Completion comp;
            auto f = source(trace, 2);
            auto failure = std::make_exception_ptr(TestException("upstream error before"));
            void* r = op == 0 ? any<int>(f, [](int) { return false; }, &comp)
                    : op == 1 ? all<int>(f, [](int) { return true; }, &comp)
                              : none<int>(f, [](int) { return false; }, &comp);
            assert_true(intrinsics::is_coroutine_suspended(r));
            trace.upstream_before.resume(Result<void*>::failure(failure));
            assert_equals(1, comp.resumes);
            assert_true(comp.result.is_failure());
            assert_true(comp.result.exception_or_null() == failure);
            assert_equals(1, trace.finally_calls);
            assert_true(trace.failure == failure);
            assert_true(trace.frame.expired());
        }

        // 3. Resumed upstream failure AFTER successful non-terminating emission
        {
            Trace trace;
            trace.pause_after_first = true;
            Completion comp;
            auto f = source(trace, 2);
            auto failure = std::make_exception_ptr(TestException("upstream error after"));
            void* r = op == 0 ? any<int>(f, [](int x) { return x > 5; }, &comp)
                    : op == 1 ? all<int>(f, [](int x) { return x > 0; }, &comp)
                              : none<int>(f, [](int x) { return x > 5; }, &comp);
            assert_true(intrinsics::is_coroutine_suspended(r));
            assert_equals(1, trace.completed_emits);
            trace.upstream_after.resume(Result<void*>::failure(failure));
            assert_equals(1, comp.resumes);
            assert_true(comp.result.is_failure());
            assert_true(comp.result.exception_or_null() == failure);
            assert_equals(1, trace.finally_calls);
            assert_true(trace.failure == failure);
            assert_true(trace.frame.expired());
        }
    }
}

void test_foreign_abort_propagation() {
    int foreign_owner = 42;
    for (int op = 0; op < 3; ++op) {
        auto abort_ex = std::make_exception_ptr(kotlinx::coroutines::flow::internal::AbortFlowException(&foreign_owner));

        // 1. Synchronous foreign abort (call inside try block)
        try {
            Completion comp;
            auto f = as_flow(std::vector<int>{1, 2});
            void* r = op == 0 ? any<int>(f, [&](int, Continuation<void*>*) -> void* {
                                    std::rethrow_exception(abort_ex);
                                }, &comp)
                    : op == 1 ? all<int>(f, [&](int, Continuation<void*>*) -> void* {
                                    std::rethrow_exception(abort_ex);
                                }, &comp)
                              : none<int>(f, [&](int, Continuation<void*>*) -> void* {
                                    std::rethrow_exception(abort_ex);
                                }, &comp);
            if (intrinsics::is_coroutine_suspended(r)) {
                comp.result.get_or_throw();
            }
            assert_true(false);
        } catch (const kotlinx::coroutines::flow::internal::AbortFlowException& e) {
            assert_true(e.owner == &foreign_owner);
        }

        // 2. Resumed foreign abort (Pending.resume failure)
        {
            Pending pending;
            Completion comp;
            auto f = as_flow(std::vector<int>{1, 2});
            void* r = op == 0 ? any<int>(f, [&](int, Continuation<void*>* cont) { return pending.suspend(cont); }, &comp)
                    : op == 1 ? all<int>(f, [&](int, Continuation<void*>* cont) { return pending.suspend(cont); }, &comp)
                              : none<int>(f, [&](int, Continuation<void*>* cont) { return pending.suspend(cont); }, &comp);
            assert_true(intrinsics::is_coroutine_suspended(r));
            pending.resume(Result<void*>::failure(abort_ex));
            assert_equals(1, comp.resumes);
            assert_true(comp.result.is_failure());
            try {
                comp.result.get_or_throw();
                assert_true(false);
            } catch (const kotlinx::coroutines::flow::internal::AbortFlowException& e) {
                assert_true(e.owner == &foreign_owner);
            }
            assert_true(pending.frame.expired());
        }
    }
}

void test_cancellation_before_owned_abort_ensure_active() {
    for (int op = 0; op < 3; ++op) {
        auto job = JobImpl::create(nullptr);
        Completion comp;
        comp.context = job;
        Pending pending;
        auto f = as_flow(std::vector<int>{1, 2});
        void* r = op == 0 ? any<int>(f, [&](int, Continuation<void*>* cont) { return pending.suspend(cont); }, &comp)
                : op == 1 ? all<int>(f, [&](int, Continuation<void*>* cont) { return pending.suspend(cont); }, &comp)
                          : none<int>(f, [&](int, Continuation<void*>* cont) { return pending.suspend(cont); }, &comp);
        assert_true(intrinsics::is_coroutine_suspended(r));
        assert_equals(0, comp.resumes);

        // Cancel job while predicate is suspended
        job->cancel();

        // Resume true for any AND none, false for all: new bool(op != 1)
        // This triggers owned short-circuit, which invokes context_ensure_active(*get_context())
        pending.resume(Result<void*>::success(new bool(op != 1)));
        assert_equals(1, comp.resumes);
        assert_true(comp.result.is_failure());
        try {
            comp.result.get_or_throw();
            assert_true(false);
        } catch (const CancellationException&) {
            // Expected CancellationException from context_ensure_active
        }
        assert_true(pending.frame.expired());
    }
}

void test_real_cancellation_during_suspended_predicate() {
    for (int op = 0; op < 3; ++op) {
        auto job = JobImpl::create(nullptr);
        Completion comp;
        comp.context = job;
        Trace trace;
        std::shared_ptr<CancellableContinuationImpl<bool>> pending;
        std::weak_ptr<BaseContinuationImpl> frame;
        auto capture = std::make_shared<int>(42);
        std::weak_ptr<int> capture_lifetime = capture;
        auto upstream = source(trace, 3);
        std::weak_ptr<Flow<int>> upstream_lifetime = upstream;
        void* r;
        {
            auto predicate = [&, capture](int, Continuation<void*>* continuation) -> void* {
                assert_equals(42, *capture);
                if (auto* base = dynamic_cast<BaseContinuationImpl*>(continuation)) {
                    frame = base->weak_from_this();
                }
                return suspend_cancellable_coroutine<bool>([&](CancellableContinuation<bool>& value) {
                    pending = dynamic_cast<CancellableContinuationImpl<bool>&>(value).shared_from_this();
                }, continuation);
            };
            r = op == 0 ? any<int>(upstream, predicate, &comp)
              : op == 1 ? all<int>(upstream, predicate, &comp)
                        : none<int>(upstream, predicate, &comp);
        }
        capture.reset();
        upstream.reset(); // retain upstream only through suspended frames
        assert_true(intrinsics::is_coroutine_suspended(r));
        assert_equals(0, comp.resumes);
        assert_false(frame.expired());
        assert_false(capture_lifetime.expired());
        assert_false(upstream_lifetime.expired());
        assert_true(pending != nullptr);

        job->cancel();

        // Require completion failure WITHOUT manually resuming predicate
        assert_equals(1, comp.resumes);
        assert_true(comp.result.is_failure());
        try {
            comp.result.get_or_throw();
            assert_true(false);
        } catch (const CancellationException&) {
            // CancellationException correctly caught and propagated
        }
        assert_equals(1, trace.finally_calls);

        // Release retained cancellation handle
        pending.reset();

        // Assert frames/captures/upstream released
        assert_true(frame.expired());
        assert_true(trace.frame.expired());
        assert_true(capture_lifetime.expired());
        assert_true(upstream_lifetime.expired());
    }
}

void test_capture_and_upstream_lifetime_release() {
    for (int op = 0; op < 3; ++op) {
        for (int with_job = 0; with_job < 2; ++with_job) {
            std::shared_ptr<JobImpl> job = with_job ? JobImpl::create(nullptr) : nullptr;
            Completion comp;
            if (job) comp.context = job;

            auto capture_marker = std::make_shared<int>(42);
            std::weak_ptr<int> weak_capture = capture_marker;

            Pending pending;
            std::weak_ptr<Flow<int>> weak_upstream;
            {
                auto raw_upstream = as_flow(std::vector<int>{1});
                weak_upstream = raw_upstream;

                void* r = op == 0 ? any<int>(raw_upstream, [&pending, capture_marker](int, Continuation<void*>* cont) {
                                        assert_equals(42, *capture_marker);
                                        return pending.suspend(cont);
                                    }, &comp)
                        : op == 1 ? all<int>(raw_upstream, [&pending, capture_marker](int, Continuation<void*>* cont) {
                                        assert_equals(42, *capture_marker);
                                        return pending.suspend(cont);
                                    }, &comp)
                                  : none<int>(raw_upstream, [&pending, capture_marker](int, Continuation<void*>* cont) {
                                        assert_equals(42, *capture_marker);
                                        return pending.suspend(cont);
                                    }, &comp);
                assert_true(intrinsics::is_coroutine_suspended(r));
            }
            // Reset external capture reference while suspended
            capture_marker.reset();

            // Assert weak handles survive until resume
            assert_false(weak_capture.expired());
            assert_false(weak_upstream.expired());
            assert_false(pending.frame.expired());

            // Resume predicate
            pending.resume(Result<void*>::success(new bool(true)));
            assert_equals(1, comp.resumes);
            void* ptr = comp.result.get_or_throw();
            delete static_cast<bool*>(ptr);

            // Assert weak handles expire after resume and completion
            assert_true(weak_capture.expired());
            assert_true(weak_upstream.expired());
            assert_true(pending.frame.expired());

            // Verify job remains active after normal completion
            if (job) {
                assert_true(job->is_active());
            }
        }
    }
}

void test_interleaved_concurrent_collections() {
    for (int op1 = 0; op1 < 3; ++op1) {
        for (int op2 = 0; op2 < 3; ++op2) {
            auto f = as_flow(std::vector<int>{1});
            Pending p1;
            Pending p2;
            Completion c1;
            Completion c2;

            void* r1 = op1 == 0 ? any<int>(f, [&](int, Continuation<void*>* cont) { return p1.suspend(cont); }, &c1)
                     : op1 == 1 ? all<int>(f, [&](int, Continuation<void*>* cont) { return p1.suspend(cont); }, &c1)
                                : none<int>(f, [&](int, Continuation<void*>* cont) { return p1.suspend(cont); }, &c1);
            void* r2 = op2 == 0 ? any<int>(f, [&](int, Continuation<void*>* cont) { return p2.suspend(cont); }, &c2)
                     : op2 == 1 ? all<int>(f, [&](int, Continuation<void*>* cont) { return p2.suspend(cont); }, &c2)
                                : none<int>(f, [&](int, Continuation<void*>* cont) { return p2.suspend(cont); }, &c2);
            assert_true(intrinsics::is_coroutine_suspended(r1));
            assert_true(intrinsics::is_coroutine_suspended(r2));
            assert_equals(0, c1.resumes);
            assert_equals(0, c2.resumes);
            assert_false(p1.frame.expired());
            assert_false(p2.frame.expired());

            // Resume first with true
            p1.resume(Result<void*>::success(new bool(true)));
            assert_equals(1, c1.resumes);
            assert_equals(0, c2.resumes);
            void* ptr1 = c1.result.get_or_throw();
            assert_true(ptr1 != nullptr);
            bool v1 = *static_cast<bool*>(ptr1);
            delete static_cast<bool*>(ptr1);
            assert_equals(op1 != 2, v1);
            assert_true(p1.frame.expired());
            assert_false(p2.frame.expired());

            // Resume second with false
            p2.resume(Result<void*>::success(new bool(false)));
            assert_equals(1, c1.resumes);
            assert_equals(1, c2.resumes);
            void* ptr2 = c2.result.get_or_throw();
            assert_true(ptr2 != nullptr);
            bool v2 = *static_cast<bool*>(ptr2);
            delete static_cast<bool*>(ptr2);
            assert_equals(op2 == 2, v2);
            assert_true(p2.frame.expired());
        }
    }
}

void test_delegated_none_result_consumption() {
    for (int outcome = 0; outcome < 2; ++outcome) {
        bool any_returns = (outcome == 1);
        Pending pending;
        Completion comp;
        auto f = as_flow(std::vector<int>{1});
        void* r = none<int>(f, [&](int, Continuation<void*>* cont) {
            return pending.suspend(cont);
        }, &comp);
        assert_true(intrinsics::is_coroutine_suspended(r));
        assert_false(pending.frame.expired());

        // Resume inner any with any_returns
        pending.resume(Result<void*>::success(new bool(any_returns)));
        assert_equals(1, comp.resumes);
        void* ptr = comp.result.get_or_throw();
        assert_true(ptr != nullptr);
        bool val = *static_cast<bool*>(ptr);
        delete static_cast<bool*>(ptr);
        assert_equals(!any_returns, val);
        assert_true(pending.frame.expired());
    }
}

} // namespace

int main() {
    try {
#define RUN_TEST(name) std::cout << #name << std::endl; name()
        // Ported 12 tests from BooleanTerminationTest.kt
        RUN_TEST(test_any_nominal);
        RUN_TEST(test_any_empty);
        RUN_TEST(test_any_infinite);
        RUN_TEST(test_any_short_circuit);
        RUN_TEST(test_all_nominal);
        RUN_TEST(test_all_empty);
        RUN_TEST(test_all_infinite);
        RUN_TEST(test_all_short_circuit);
        RUN_TEST(test_none_nominal);
        RUN_TEST(test_none_empty);
        RUN_TEST(test_none_infinite);
        RUN_TEST(test_none_short_circuit);

        // Deterministic continuation tests
        RUN_TEST(test_immediate_results_and_no_callback);
        RUN_TEST(test_delayed_predicate_suspensions);
        RUN_TEST(test_short_circuit_at_exact_value);
        RUN_TEST(test_upstream_suspension_and_finally_ordering);
        RUN_TEST(test_resumed_failures_propagation);
        RUN_TEST(test_foreign_abort_propagation);
        RUN_TEST(test_cancellation_before_owned_abort_ensure_active);
        RUN_TEST(test_real_cancellation_during_suspended_predicate);
        RUN_TEST(test_capture_and_upstream_lifetime_release);
        RUN_TEST(test_interleaved_concurrent_collections);
        RUN_TEST(test_delegated_none_result_consumption);
#undef RUN_TEST
        std::cout << "All 23 test_logic_suspension checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAILED: " << error.what() << '\n';
        return 1;
    }
}
