// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/TakeTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/DropTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/DropWhileTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/TakeWhileTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/TransformWhileTest.kt
/** Continuation ABI regressions for limiting-flow emission, predicates and ownership. */
#include "kotlinx/coroutines/flow/Limit.hpp"
#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include <climits>
#include <iostream>
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
        if (auto* base = dynamic_cast<BaseContinuationImpl*>(value)) frame = base->weak_from_this();
        return intrinsics::get_COROUTINE_SUSPENDED();
    }
    void resume(Result<void*> result = Result<void*>::success(nullptr)) {
        auto* target = std::exchange(continuation, nullptr);
        assert_true(target != nullptr);
        target->resume_with(std::move(result));
    }
};

struct Collector final : FlowCollector<int> {
    std::vector<int> values;
    Pending pending;
    bool pause = true;
    void* emit(int value, Continuation<void*>* continuation) override {
        values.push_back(value);
        return pause ? pending.suspend(continuation) : nullptr;
    }
};

struct Trace {
    int completed_emits = 0;
    int finally_calls = 0;
    std::exception_ptr failure;
    bool pause_upstream = false;
    Pending upstream;
    std::weak_ptr<BaseContinuationImpl> frame;
};

// Lower the upstream test flow's sequential emit calls and a surrounding finally.
std::shared_ptr<Flow<int>> source(Trace& trace) {
    return kotlinx::coroutines::flow::flow<int>([&trace](FlowCollector<int>* collector, Continuation<void*>* completion) -> void* {
        class Frame final : public ContinuationImpl {
        public:
            Frame(Trace& trace, FlowCollector<int>* collector, Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  trace_(trace), collector_(collector) {}
            void retain() { self_ref_ = shared_from_this(); }
            void* invoke_suspend(Result<void*> result) override {
                try {
                    coroutine_begin(this)
                    if (trace_.pause_upstream) coroutine_yield(this, trace_.upstream.suspend(this));
                    for (value_ = 1; value_ <= 3; ++value_) {
                        coroutine_yield(this, collector_->emit(value_, this));
                        ++trace_.completed_emits;
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
            FlowCollector<int>* collector_;
            int value_ = 0;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        auto frame = std::make_shared<Frame>(trace, collector, completion);
        trace.frame = frame;
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

std::vector<int> collect_now(const std::shared_ptr<Flow<int>>& flow) {
    Collector collector;
    collector.pause = false;
    Completion completion;
    assert_true(flow->collect(&collector, &completion) == nullptr);
    assert_equals(0, completion.resumes);
    return collector.values;
}

void require_failure(const Completion& completion, std::exception_ptr failure) {
    assert_equals(1, completion.resumes);
    assert_true(completion.result.exception_or_null() == failure);
}

void require_cancelled(const Completion& completion) {
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_failure());
    try {
        completion.result.get_or_throw();
        assert_true(false);
    } catch (const CancellationException&) {
    }
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/TakeTest.kt:8-53
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/DropTest.kt:8-33
void test_immediate_counts_and_empty_flows() {
    auto values = as_flow(std::vector<int>{1, 2, 3});
    assert_true(collect_now(take(values, 1)) == std::vector<int>{1});
    assert_true(collect_now(take(values, 2)) == std::vector<int>({1, 2}));
    assert_true(collect_now(take(values, INT_MAX)) == std::vector<int>({1, 2, 3}));
    assert_true(collect_now(drop(values, 1)) == std::vector<int>({2, 3}));
    assert_true(collect_now(drop(values, 0)) == std::vector<int>({1, 2, 3}));
    assert_true(collect_now(drop(values, INT_MAX)).empty());
    assert_true(collect_now(drop(take(drop(values, 1), 2), 1)) == std::vector<int>{3});
    assert_true(collect_now(take(empty_flow<int>(), 10)).empty());
    assert_true(collect_now(drop(empty_flow<int>(), 1)).empty());
    for (int count : {0, -1}) {
        bool failed = false;
        try { take(values, count); } catch (const std::invalid_argument& error) {
            assert_true(std::string(error.what()) == "Requested element count " + std::to_string(count) + " should be positive");
            failed = true;
        }
        assert_true(failed);
    }
    bool failed = false;
    try { drop(values, -1); } catch (const std::invalid_argument& error) {
        assert_true(std::string(error.what()) == "Drop count should be non-negative, but had -1");
        failed = true;
    }
    assert_true(failed);
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/DropWhileTest.kt:8-30
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/TakeWhileTest.kt:9-26
void test_immediate_predicates() {
    auto values = as_flow(std::vector<int>{1, 2, 3});
    assert_true(collect_now(drop_while(values, [](int) { return false; })) == std::vector<int>({1, 2, 3}));
    assert_true(collect_now(drop_while(values, [](int) { return true; })).empty());
    assert_true(collect_now(drop_while(values, [](int value) { return value < 2; })) == std::vector<int>({2, 3}));
    assert_true(collect_now(take_while(values, [](int value) { return value < 2; })) == std::vector<int>{1});
    assert_true(collect_now(take_while(values, [](int) { return true; })) == std::vector<int>({1, 2, 3}));
    assert_true(collect_now(take_while(values, [](int) { return false; })).empty());
    for (bool answer : {false, true}) {
        auto predicate = [answer](int) { return answer; };
        assert_true(collect_now(take_while(empty_flow<int>(), predicate)).empty());
        assert_true(collect_now(drop_while(empty_flow<int>(), predicate)).empty());
    }
}

void test_take_waits_for_final_emit() {
    Completion completion;
    Collector collector;
    auto limited = take<int>(as_flow(std::vector<int>{1, 2}), 1);
    auto* result = limited->collect(&collector, &completion);
    assert_true(intrinsics::is_coroutine_suspended(result));
    assert_equals(0, completion.resumes);
    assert_true(collector.values == std::vector<int>{1});
    limited.reset();
    assert_false(collector.pending.frame.expired());
    collector.pending.resume();
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_success());
    assert_true(collector.values == std::vector<int>{1});
    assert_true(collector.pending.frame.expired());
}

void test_take_finally_and_resumed_failure() {
    for (bool fail : {false, true}) {
        for (int count : {1, 2}) {
            Trace trace;
            Completion completion;
            Collector collector;
            auto limited = take(source(trace), count);
            auto upstream_failure = std::make_exception_ptr(TestException());
            assert_true(intrinsics::is_coroutine_suspended(limited->collect(&collector, &completion)));
            limited.reset();
            assert_equals(0, trace.finally_calls);
            if (count == 2) {
                collector.pending.resume();
                assert_equals(1, trace.completed_emits);
                assert_equals(0, completion.resumes);
            }
            collector.pending.resume(fail ? Result<void*>::failure(upstream_failure) : Result<void*>::success(nullptr));
            assert_equals(1, completion.resumes);
            assert_equals(1, trace.finally_calls);
            assert_equals(count - 1, trace.completed_emits);
            if (fail) {
                require_failure(completion, upstream_failure);
                assert_true(trace.failure == upstream_failure);
            } else {
                assert_true(completion.result.is_success());
                assert_true(trace.failure != nullptr);
            }
            assert_true(trace.frame.expired());
            assert_true(collector.pending.frame.expired());
        }
    }
}

void test_nested_take_ownership() {
    for (int inner_count : {1, 2, 3}) {
        for (int outer_count : {1, 2, 3}) {
            Trace trace;
            Completion completion;
            Collector collector;
            auto pipeline = take(take(source(trace), inner_count), outer_count);
            assert_true(intrinsics::is_coroutine_suspended(pipeline->collect(&collector, &completion)));
            pipeline.reset();
            const int count = std::min(inner_count, outer_count);
            for (int i = 0; i < count; ++i) collector.pending.resume();
            assert_equals(size_t(count), collector.values.size());
            assert_equals(1, completion.resumes);
            assert_true(completion.result.is_success());
            assert_equals(1, trace.finally_calls);
            assert_true(trace.frame.expired());
        }
    }
}

void test_upstream_and_drop_lifetime_without_job() {
    Trace trace;
    trace.pause_upstream = true;
    Completion completion;
    Collector collector;
    auto upstream = source(trace);
    std::weak_ptr<Flow<int>> retained = upstream;
    auto pipeline = drop(upstream, 1);
    upstream.reset();
    assert_true(intrinsics::is_coroutine_suspended(pipeline->collect(&collector, &completion)));
    pipeline.reset();
    assert_false(retained.expired());
    trace.upstream.resume();
    assert_true(collector.values == std::vector<int>{2});
    collector.pending.resume();
    assert_true(collector.values == std::vector<int>({2, 3}));
    collector.pending.resume();
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_success());
    assert_equals(1, trace.finally_calls);
    assert_true(trace.frame.expired());
    assert_true(retained.expired());
}

void test_drop_while_predicate_then_emit() {
    Pending predicate;
    std::vector<int> tested;
    Completion completion;
    Collector collector;
    auto flow = drop_while(as_flow(std::vector<int>{1, 2, 3}), [&](int value, Continuation<void*>* continuation) -> void* {
        tested.push_back(value);
        return predicate.suspend(continuation);
    });
    assert_true(intrinsics::is_coroutine_suspended(flow->collect(&collector, &completion)));
    flow.reset();
    assert_true(collector.values.empty());
    predicate.resume(Result<void*>::success(new bool(true)));
    assert_true(tested == std::vector<int>({1, 2}));
    assert_true(collector.values.empty());
    predicate.resume(Result<void*>::success(new bool(false)));
    assert_true(collector.values == std::vector<int>{2});
    collector.pending.resume();
    assert_true(collector.values == std::vector<int>({2, 3}));
    assert_true(tested == std::vector<int>({1, 2}));
    collector.pending.resume();
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_success());
    assert_true(predicate.frame.expired());
    assert_true(collector.pending.frame.expired());
}

void test_take_while_predicate_then_emit() {
    Trace trace;
    Pending predicate;
    int calls = 0;
    Completion completion;
    Collector collector;
    auto flow = take_while(source(trace), [&](int value, Continuation<void*>* continuation) -> void* {
        assert_equals(++calls, value);
        return predicate.suspend(continuation);
    });
    assert_true(intrinsics::is_coroutine_suspended(flow->collect(&collector, &completion)));
    flow.reset();
    predicate.resume(Result<void*>::success(new bool(true)));
    assert_true(collector.values == std::vector<int>{1});
    assert_equals(1, calls);
    collector.pending.resume();
    assert_equals(2, calls);
    assert_equals(1, trace.completed_emits);
    predicate.resume(Result<void*>::success(new bool(false)));
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_success());
    assert_equals(1, trace.finally_calls);
    assert_true(collector.values == std::vector<int>{1});
    assert_true(trace.frame.expired());
    assert_true(predicate.frame.expired());
}

void test_predicate_failure_and_foreign_abort() {
    int foreign_owner = 0;
    for (int kind : {0, 1, 2}) {
        for (bool foreign_abort : {false, true}) {
            for (bool delayed : {false, true}) {
                Trace trace;
                Pending predicate;
                Completion completion;
                Collector collector;
                auto failure = foreign_abort
                    ? std::make_exception_ptr(kotlinx::coroutines::flow::internal::AbortFlowException(&foreign_owner))
                    : std::make_exception_ptr(TestException());
                auto function = [&](int, Continuation<void*>* continuation) -> void* {
                    if (!delayed) std::rethrow_exception(failure);
                    return predicate.suspend(continuation);
                };
                auto pipeline = kind == 0 ? drop_while(source(trace), function)
                    : kind == 1 ? take_while(source(trace), function)
                    : transform_while<int, int>(source(trace), [&](FlowCollector<int>*, int value, Continuation<void*>* continuation) {
                        return function(value, continuation);
                    });
                if (delayed) {
                    assert_true(intrinsics::is_coroutine_suspended(pipeline->collect(&collector, &completion)));
                    pipeline.reset();
                    predicate.resume(Result<void*>::failure(failure));
                    assert_equals(1, completion.resumes);
                } else {
                    try {
                        pipeline->collect(&collector, &completion);
                        assert_true(false);
                    } catch (...) {
                        completion.result = Result<void*>::failure(std::current_exception());
                    }
                    assert_equals(0, completion.resumes);
                }
                assert_true(completion.result.is_failure());
                if (foreign_abort) {
                    try {
                        completion.result.get_or_throw();
                    } catch (const kotlinx::coroutines::flow::internal::AbortFlowException& error) {
                        assert_true(error.owner == &foreign_owner);
                    }
                } else {
                    assert_true(completion.result.exception_or_null() == failure);
                }
                assert_true(collector.values.empty());
                assert_equals(1, trace.finally_calls);
                assert_true(trace.frame.expired());
            }
        }
    }
}

void test_take_propagates_foreign_abort() {
    int owner = 0;
    Completion completion;
    Collector collector;
    auto pipeline = take(as_flow(std::vector<int>{1, 2}), 1);
    assert_true(intrinsics::is_coroutine_suspended(pipeline->collect(&collector, &completion)));
    pipeline.reset();
    collector.pending.resume(Result<void*>::failure(std::make_exception_ptr(
        kotlinx::coroutines::flow::internal::AbortFlowException(&owner))));
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_failure());
    try {
        completion.result.get_or_throw();
    } catch (const kotlinx::coroutines::flow::internal::AbortFlowException& error) {
        assert_true(error.owner == &owner);
    }
    assert_true(collector.pending.frame.expired());
}

void test_collect_while_checks_cancellation_after_owned_abort() {
    auto job = JobImpl::create(nullptr);
    Completion completion;
    completion.context = job;
    Pending predicate;
    auto flow = as_flow(std::vector<int>{1, 2});
    assert_true(intrinsics::is_coroutine_suspended(collect_while<int>(flow,
        [&](int, Continuation<void*>* continuation) { return predicate.suspend(continuation); }, &completion)));
    flow.reset();
    job->cancel();
    assert_equals(0, completion.resumes);
    predicate.resume(Result<void*>::success(new bool(false)));
    require_cancelled(completion);
    assert_true(predicate.frame.expired());
}

void test_take_keeps_its_distinct_abort_contract() {
    auto job = JobImpl::create(nullptr);
    Completion completion;
    completion.context = job;
    Collector collector;
    auto flow = take(as_flow(std::vector<int>{1, 2}), 1);
    assert_true(intrinsics::is_coroutine_suspended(flow->collect(&collector, &completion)));
    flow.reset();
    job->cancel();
    collector.pending.resume();
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_success());
}

void test_cancellable_predicate_releases_collection() {
    for (int kind : {0, 1, 2}) {
        auto job = JobImpl::create(nullptr);
        Completion completion;
        completion.context = job;
        Collector collector;
        Trace trace;
        std::shared_ptr<CancellableContinuationImpl<bool>> pending;
        std::weak_ptr<BaseContinuationImpl> frame;
        auto function = [&](int, Continuation<void*>* continuation) -> void* {
            frame = dynamic_cast<BaseContinuationImpl*>(continuation)->weak_from_this();
            return suspend_cancellable_coroutine<bool>([&](CancellableContinuation<bool>& value) {
                pending = dynamic_cast<CancellableContinuationImpl<bool>&>(value).shared_from_this();
            }, continuation);
        };
        auto pipeline = kind == 0 ? drop_while(source(trace), function)
            : kind == 1 ? take_while(source(trace), function)
            : transform_while<int, int>(source(trace), [function](FlowCollector<int>*, int value, Continuation<void*>* continuation) mutable {
                return function(value, continuation);
            });
        assert_true(intrinsics::is_coroutine_suspended(pipeline->collect(&collector, &completion)));
        pipeline.reset();
        assert_false(frame.expired());
        job->cancel();
        require_cancelled(completion);
        assert_equals(1, trace.finally_calls);
        pending.reset();
        assert_true(frame.expired());
        assert_true(trace.frame.expired());
        assert_true(collector.values.empty());
    }
}

void test_capture_released_before_job_ends() {
    auto job = JobImpl::create(nullptr);
    Completion completion;
    completion.context = job;
    Collector collector;
    auto marker = std::make_shared<int>(1);
    std::weak_ptr<int> lifetime = marker;
    Pending predicate;
    auto pipeline = take_while(as_flow(std::vector<int>{1, 2}),
        [&, marker](int, Continuation<void*>* continuation) {
            assert_equals(1, *marker);
            return predicate.suspend(continuation);
        });
    marker.reset();
    assert_true(intrinsics::is_coroutine_suspended(pipeline->collect(&collector, &completion)));
    pipeline.reset();
    assert_false(lifetime.expired());
    predicate.resume(Result<void*>::success(new bool(false)));
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_success());
    assert_true(lifetime.expired());
    assert_true(job->is_active());
}

void test_cancellation_during_final_emit() {
    class CancellableCollector final : public FlowCollector<int> {
    public:
        std::shared_ptr<CancellableContinuationImpl<void>> pending;
        std::weak_ptr<BaseContinuationImpl> frame;
        void* emit(int, Continuation<void*>* continuation) override {
            frame = dynamic_cast<BaseContinuationImpl*>(continuation)->weak_from_this();
            return suspend_cancellable_coroutine<void>([this](CancellableContinuation<void>& value) {
                pending = dynamic_cast<CancellableContinuationImpl<void>&>(value).shared_from_this();
            }, continuation);
        }
    } collector;
    Trace trace;
    auto job = JobImpl::create(nullptr);
    Completion completion;
    completion.context = job;
    auto pipeline = take(source(trace), 1);
    assert_true(intrinsics::is_coroutine_suspended(pipeline->collect(&collector, &completion)));
    pipeline.reset();
    job->cancel();
    require_cancelled(completion);
    assert_equals(1, trace.finally_calls);
    assert_equals(0, trace.completed_emits);
    collector.pending.reset();
    assert_true(collector.frame.expired());
    assert_true(trace.frame.expired());
}

void test_resume_before_predicate_returns() {
    auto pipeline = take_while(as_flow(std::vector<int>{1, 2, 3}),
        [](int value, Continuation<void*>* continuation) {
            return suspend_cancellable_coroutine<bool>([value](CancellableContinuation<bool>& resumed) {
                resumed.resume(value < 3, nullptr);
            }, continuation);
        });
    assert_true(collect_now(pipeline) == std::vector<int>({1, 2}));
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/TransformWhileTest.kt:28-44,63-67
void test_transform_while_multiple_suspended_emits() {
    Trace trace;
    Completion completion;
    Collector collector;
    auto pipeline = transform_while<int, int>(source(trace),
        [](FlowCollector<int>* collector, int value, Continuation<void*>* completion) -> void* {
            class Frame final : public ContinuationImpl {
            public:
                Frame(FlowCollector<int>* collector, int value, Continuation<void*>* completion)
                    : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                      collector_(collector), value_(value) {}
                void retain() { self_ref_ = shared_from_this(); }
                void* invoke_suspend(Result<void*> result) override {
                    coroutine_begin(this)
                    coroutine_yield(this, collector_->emit(value_, this));
                    coroutine_yield(this, collector_->emit(value_ + 10, this));
                    return new bool(value_ < 2);
                }
            protected:
                void release_intercepted() override {
                    ContinuationImpl::release_intercepted();
                    self_ref_.reset();
                }
            private:
                void* _label = nullptr;
                FlowCollector<int>* collector_;
                int value_;
                std::shared_ptr<BaseContinuationImpl> self_ref_;
            };
            auto frame = std::make_shared<Frame>(collector, value, completion);
            frame->retain();
            return frame->start(Result<void*>::success(nullptr));
        });
    assert_true(intrinsics::is_coroutine_suspended(pipeline->collect(&collector, &completion)));
    pipeline.reset();
    for (int i = 0; i < 4; ++i) {
        assert_equals(0, completion.resumes);
        collector.pending.resume();
    }
    assert_true(collector.values == std::vector<int>({1, 11, 2, 12}));
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_success());
    assert_equals(1, trace.finally_calls);
    assert_true(trace.frame.expired());
    assert_true(collector.pending.frame.expired());
}

void test_independent_collections() {
    auto flow = take(as_flow(std::vector<int>{1, 2, 3}), 2);
    Completion first;
    Completion second;
    Collector a;
    Collector b;
    assert_true(intrinsics::is_coroutine_suspended(flow->collect(&a, &first)));
    assert_true(intrinsics::is_coroutine_suspended(flow->collect(&b, &second)));
    a.pending.resume();
    a.pending.resume();
    assert_equals(1, first.resumes);
    assert_equals(0, second.resumes);
    b.pending.resume();
    b.pending.resume();
    assert_equals(1, second.resumes);
    assert_true(a.values == std::vector<int>({1, 2}));
    assert_true(a.values == b.values);
    assert_true(collect_now(flow) == a.values);
}

} // namespace

int main() {
    try {
#define RUN_LIMIT_TEST(name) std::cout << #name << std::endl; name()
        RUN_LIMIT_TEST(test_immediate_counts_and_empty_flows);
        RUN_LIMIT_TEST(test_immediate_predicates);
        RUN_LIMIT_TEST(test_take_waits_for_final_emit);
        RUN_LIMIT_TEST(test_take_finally_and_resumed_failure);
        RUN_LIMIT_TEST(test_nested_take_ownership);
        RUN_LIMIT_TEST(test_upstream_and_drop_lifetime_without_job);
        RUN_LIMIT_TEST(test_drop_while_predicate_then_emit);
        RUN_LIMIT_TEST(test_take_while_predicate_then_emit);
        RUN_LIMIT_TEST(test_predicate_failure_and_foreign_abort);
        RUN_LIMIT_TEST(test_take_propagates_foreign_abort);
        RUN_LIMIT_TEST(test_collect_while_checks_cancellation_after_owned_abort);
        RUN_LIMIT_TEST(test_take_keeps_its_distinct_abort_contract);
        RUN_LIMIT_TEST(test_cancellable_predicate_releases_collection);
        RUN_LIMIT_TEST(test_capture_released_before_job_ends);
        RUN_LIMIT_TEST(test_cancellation_during_final_emit);
        RUN_LIMIT_TEST(test_resume_before_predicate_returns);
        RUN_LIMIT_TEST(test_transform_while_multiple_suspended_emits);
        RUN_LIMIT_TEST(test_independent_collections);
#undef RUN_LIMIT_TEST
        std::cout << "18 Limit suspension checks passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
