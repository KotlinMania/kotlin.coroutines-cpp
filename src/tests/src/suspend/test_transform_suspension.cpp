// port-lint: tests kotlinx-coroutines-core/common/test/flow/operators/FilterTest.kt
// port-lint: tests kotlinx-coroutines-core/common/test/flow/operators/MapTest.kt
// port-lint: tests kotlinx-coroutines-core/common/test/flow/operators/MapNotNullTest.kt
// port-lint: tests kotlinx-coroutines-core/common/test/flow/operators/OnEachTest.kt
// port-lint: tests kotlinx-coroutines-core/common/test/flow/operators/IndexedTest.kt
// port-lint: tests kotlinx-coroutines-core/common/test/flow/operators/ScanTest.kt
// port-lint: tests kotlinx-coroutines-core/common/test/flow/operators/ChunkedTest.kt
// port-lint: tests kotlinx-coroutines-core/common/test/flow/operators/FilterTrivialTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/FilterTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/MapTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/MapNotNullTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/OnEachTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/IndexedTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/ScanTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/ChunkedTest.kt
/**
 * @file test_transform_suspension.cpp
 * @brief Continuation ABI regressions for intermediate flow transformation operators.
 */

#include "kotlinx/coroutines/flow/Transform.hpp"
#include "kotlinx/coroutines/flow/Merge.hpp"
#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"

#include <climits>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
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

    void resume(Result<void*> outcome = Result<void*>::success(nullptr)) {
        auto* target = std::exchange(continuation, nullptr);
        assert_true(target != nullptr);
        target->resume_with(std::move(outcome));
    }
};

template <typename T>
struct Collector final : FlowCollector<T> {
    std::vector<T> values;
    Pending pending;
    bool pause = false;

    void* emit(T value, Continuation<void*>* continuation) override {
        values.push_back(std::move(value));
        return pause ? pending.suspend(continuation) : nullptr;
    }
};

struct Trace {
    int collection_starts = 0;
    int completed_emits = 0;
    int finally_calls = 0;
    std::exception_ptr failure;
    bool pause_upstream = false;
    Pending upstream;
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
                    ++trace_.collection_starts;
                    if (trace_.pause_upstream) coroutine_yield(this, trace_.upstream.suspend(this));
                    for (value_ = 1; value_ <= count_; ++value_) {
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

template <typename T>
std::vector<T> collect_now(const std::shared_ptr<Flow<T>>& f) {
    Collector<T> collector;
    collector.pause = false;
    Completion completion;
    void* res = f->collect(&collector, &completion);
    assert_true(res == nullptr);
    assert_equals(0, completion.resumes);
    return collector.values;
}

// Struct without default constructor to verify RunningReduceFrame does not require default construction.
struct NoDefault {

    int value;

    explicit NoDefault(int v) : value(v) {}
    NoDefault() = delete;
    NoDefault(const NoDefault& o) = default;
    NoDefault(NoDefault&& o) noexcept = default;
    NoDefault& operator=(const NoDefault& o) { value = o.value; return *this; }
    NoDefault& operator=(NoDefault&& o) noexcept { value = o.value; return *this; }

    bool operator==(const NoDefault& o) const { return value == o.value; }
};

struct BaseType {
    virtual ~BaseType() = default;
    int id = 0;
    explicit BaseType(int i) : id(i) {}
};

struct DerivedType : BaseType {
    std::string tag;
    DerivedType(int i, std::string t) : BaseType(i), tag(std::move(t)) {}
};

// ============================================================================
// Upstream Behavioral Tests
// ============================================================================

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/FilterTest.kt
void test_filter_and_filter_not_nominal() {
    auto f = as_flow(std::vector<int>{1, 2, 3, 4, 5});
    auto evens = filter(f, [](int x) { return x % 2 == 0; });
    assert_true(collect_now(evens) == std::vector<int>({2, 4}));

    auto odds = filter_not(f, [](int x) { return x % 2 == 0; });
    assert_true(collect_now(odds) == std::vector<int>({1, 3, 5}));

    auto empty = empty_flow<int>();
    assert_true(collect_now(filter(empty, [](int) { return true; })).empty());
    assert_true(collect_now(filter_not(empty, [](int) { return false; })).empty());

    assert_true(collect_now(filter(f, [](int) { return true; })) == std::vector<int>({1, 2, 3, 4, 5}));
    assert_true(collect_now(filter(f, [](int) { return false; })).empty());
}

// C++ pointer representation coverage.
void test_filter_is_instance_and_not_null() {
    // 1. Shared pointer downcasting
    std::vector<std::shared_ptr<BaseType>> mixed{
        std::make_shared<BaseType>(1),
        std::make_shared<DerivedType>(2, "derived1"),
        std::make_shared<BaseType>(3),
        std::make_shared<DerivedType>(4, "derived2")
    };
    auto derived_flow = filter_is_instance<DerivedType>(as_flow(mixed));
    auto derived_res = collect_now(derived_flow);
    assert_equals(size_t(2), derived_res.size());
    assert_equals(2, derived_res[0]->id);
    assert_true(derived_res[0]->tag == "derived1");
    assert_equals(4, derived_res[1]->id);
    assert_true(derived_res[1]->tag == "derived2");

    // 2. Raw pointer downcasting
    BaseType b1(10);
    DerivedType d1(20, "raw_d");
    std::vector<BaseType*> raw_ptrs{&b1, &d1};
    auto raw_derived = filter_is_instance<DerivedType>(as_flow(raw_ptrs));
    auto raw_res = collect_now(raw_derived);
    assert_equals(size_t(1), raw_res.size());
    assert_equals(20, raw_res[0]->id);

    // 3. Raw pointer filter_not_null
    int v1 = 100, v2 = 200;
    std::vector<int*> ptrs{&v1, nullptr, &v2, nullptr};
    auto nn_ptrs = filter_not_null(as_flow(ptrs));
    auto nn_res = collect_now(nn_ptrs);
    assert_equals(size_t(2), nn_res.size());
    assert_equals(100, *nn_res[0]);
    assert_equals(200, *nn_res[1]);

    // 4. std::optional filter_not_null
    std::vector<std::optional<int>> opts{std::optional<int>(1), std::nullopt, std::optional<int>(3), std::nullopt};
    auto nn_opts = filter_not_null(as_flow(opts));
    assert_true(collect_now(nn_opts) == std::vector<int>({1, 3}));
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/MapTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/MapNotNullTest.kt
void test_map_and_map_not_null_nominal() {
    auto f = as_flow(std::vector<int>{1, 2, 3});
    auto mapped = map<int, int>(f, [](int x) { return x * 10; });
    assert_true(collect_now(mapped) == std::vector<int>({10, 20, 30}));

    auto mapped_str = map<int, std::string>(f, [](int x) { return "item_" + std::to_string(x); });
    assert_true(collect_now(mapped_str) == std::vector<std::string>({"item_1", "item_2", "item_3"}));

    // map_not_null (std::optional)
    auto mnn = map_not_null<int, int>(f, [](int x) -> std::optional<int> {
        if (x % 2 != 0) return x * 100;
        return std::nullopt;
    });
    assert_true(collect_now(mnn) == std::vector<int>({100, 300}));

    assert_true(collect_now(map<int, int>(empty_flow<int>(), [](int x) { return x; })).empty());
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/IndexedTest.kt
void test_with_index_nominal_and_isolation() {
    auto f = as_flow(std::vector<std::string>{"alpha", "beta", "gamma"});
    auto indexed = with_index(f);

    auto res1 = collect_now(indexed);
    assert_equals(size_t(3), res1.size());
    assert_equals(0, res1[0].index);
    assert_true(res1[0].value == "alpha");
    assert_equals(1, res1[1].index);
    assert_true(res1[1].value == "beta");
    assert_equals(2, res1[2].index);
    assert_true(res1[2].value == "gamma");

    // Per-collection isolation: second collection starts at 0 again
    auto res2 = collect_now(indexed);
    assert_equals(size_t(3), res2.size());
    assert_equals(0, res2[0].index);
    assert_equals(1, res2[1].index);
    assert_equals(2, res2[2].index);

    assert_true(collect_now(with_index(empty_flow<int>())).empty());
}

// Boundary checks for the production guard; collection still starts at index zero.
void test_index_overflow_guard() {
    assert_equals(INT_MAX, kotlinx::coroutines::flow::internal::check_index_overflow(INT_MAX));
    bool failed = false;
    try {
        kotlinx::coroutines::flow::internal::check_index_overflow(INT_MIN);
    } catch (const std::overflow_error& e) {
        failed = true;
        assert_true(std::string(e.what()) == "Index overflow has happened");
    }
    assert_true(failed);
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/OnEachTest.kt
void test_on_each_nominal() {
    std::vector<int> side_effects;
    auto f = as_flow(std::vector<int>{1, 2, 3});
    auto tracked = on_each(f, [&side_effects](int x) {
        side_effects.push_back(x * 10);
    });
    auto res = collect_now(tracked);
    assert_true(res == std::vector<int>({1, 2, 3}));
    assert_true(side_effects == std::vector<int>({10, 20, 30}));

    assert_true(collect_now(on_each(empty_flow<int>(), [](int) {})).empty());
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/ScanTest.kt
void test_scan_and_running_fold_nominal() {
    auto f = as_flow(std::vector<int>{1, 2, 3});
    auto folded = running_fold<int, int>(f, 0, [](int acc, int x) { return acc + x; });
    assert_true(collect_now(folded) == std::vector<int>({0, 1, 3, 6}));

    // scan alias
    auto scanned = scan<int, int>(f, 0, [](int acc, int x) { return acc + x; });
    assert_true(collect_now(scanned) == std::vector<int>({0, 1, 3, 6}));

    // Empty flow emits exactly the initial value
    auto empty = empty_flow<int>();
    assert_true(collect_now(running_fold<int, int>(empty, 42, [](int acc, int x) { return acc + x; })) == std::vector<int>{42});
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/ScanTest.kt
void test_running_reduce_nominal() {
    auto f = as_flow(std::vector<int>{1, 2, 3, 4});
    auto reduced = running_reduce<int>(f, [](int acc, int x) { return acc + x; });
    assert_true(collect_now(reduced) == std::vector<int>({1, 3, 6, 10}));

    // Single element emits exactly that element without calling operation
    int op_calls = 0;
    auto single = as_flow(std::vector<int>{99});
    auto single_red = running_reduce<int>(single, [&op_calls](int acc, int x) {
        ++op_calls;
        return acc + x;
    });
    assert_true(collect_now(single_red) == std::vector<int>{99});
    assert_equals(0, op_calls);

    // Empty flow emits nothing
    auto empty = empty_flow<int>();
    assert_true(collect_now(running_reduce<int>(empty, [](int a, int b) { return a + b; })).empty());
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/ChunkedTest.kt
void test_chunked_nominal() {
    // Negative or zero size throws std::invalid_argument immediately
    for (int invalid_size : {0, -1, -10}) {
        bool failed = false;
        try {
            chunked(as_flow(std::vector<int>{1, 2}), invalid_size);
        } catch (const std::invalid_argument& e) {
            failed = true;
            assert_true(std::string(e.what()) == "Expected positive chunk size, but got " + std::to_string(invalid_size));
        }
        assert_true(failed);
    }

    auto f = as_flow(std::vector<int>{1, 2, 3, 4, 5});
    auto chunks = chunked(f, 2);
    auto res = collect_now(chunks);
    assert_equals(size_t(3), res.size());
    assert_true(res[0] == std::vector<int>({1, 2}));
    assert_true(res[1] == std::vector<int>({3, 4}));
    assert_true(res[2] == std::vector<int>({5}));

    assert_true(collect_now(chunked(empty_flow<int>(), 3)).empty());
}

// ============================================================================
// Deterministic Suspension & Cancellation Regressions
// ============================================================================

// 1. Predicate suspension FOLLOWED BY downstream emission suspension
void test_filter_suspension_followed_by_downstream_suspension() {
    Trace trace;
    Completion completion;
    Collector<int> collector;
    collector.pause = true; // downstream will pause on emit

    Pending pred_pending;
    std::weak_ptr<BaseContinuationImpl> pred_frame;

    auto f = filter<int>(source(trace, 1), [&](int x, Continuation<void*>* cont) -> void* {
        if (auto* b = dynamic_cast<BaseContinuationImpl*>(cont)) pred_frame = b->weak_from_this();
        return pred_pending.suspend(cont);
    });

    void* r = f->collect(&collector, &completion);
    assert_true(intrinsics::is_coroutine_suspended(r));
    assert_equals(0, completion.resumes);
    assert_false(pred_frame.expired());
    assert_equals(size_t(0), collector.values.size());

    // Step 1: Resume predicate with true
    pred_pending.resume(Result<void*>::success(new bool(true)));
    // Predicate unboxed, and then emitted downstream! Downstream collector suspended!
    assert_equals(0, completion.resumes);
    assert_equals(size_t(1), collector.values.size());
    assert_equals(1, collector.values[0]);
    assert_false(collector.pending.frame.expired());

    // Step 2: Resume downstream emission
    collector.pending.resume(Result<void*>::success(nullptr));
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_success());
    assert_equals(1, trace.finally_calls);
    assert_true(trace.frame.expired());
    assert_true(pred_frame.expired());
    assert_true(collector.pending.frame.expired());
}

// 2. Real Job cancellation during suspended predicate
void test_filter_real_cancellation_during_predicate() {
    auto job = JobImpl::create(nullptr);
    Completion comp;
    comp.context = job;
    Trace trace;
    std::shared_ptr<CancellableContinuationImpl<bool>> pending;
    std::weak_ptr<BaseContinuationImpl> frame;
    auto capture = std::make_shared<int>(42);
    std::weak_ptr<int> capture_lifetime = capture;
    auto upstream = source(trace, 2);
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
        r = filter<int>(upstream, predicate)->collect(nullptr, &comp);
    }
    capture.reset();
    upstream.reset();

    assert_true(intrinsics::is_coroutine_suspended(r));
    assert_equals(0, comp.resumes);
    assert_false(frame.expired());
    assert_false(capture_lifetime.expired());
    assert_false(upstream_lifetime.expired());
    assert_true(pending != nullptr);

    // Cancel job while suspended in predicate
    job->cancel();

    // Must fail without manual resume
    assert_equals(1, comp.resumes);
    assert_true(comp.result.is_failure());
    try {
        comp.result.get_or_throw();
        assert_true(false);
    } catch (const CancellationException&) {
    }
    assert_equals(1, trace.finally_calls);

    pending.reset();
    assert_true(frame.expired());
    assert_true(trace.frame.expired());
    assert_true(capture_lifetime.expired());
    assert_true(upstream_lifetime.expired());
}

// 3. Map transform suspension FOLLOWED BY downstream emission suspension
void test_map_suspension_followed_by_downstream_suspension() {
    Trace trace;
    Completion completion;
    Collector<std::string> collector;
    collector.pause = true;

    Pending transform_pending;
    std::weak_ptr<BaseContinuationImpl> map_frame;

    auto f = map<int, std::string>(source(trace, 1), [&](int x, Continuation<void*>* cont) -> void* {
        if (auto* b = dynamic_cast<BaseContinuationImpl*>(cont)) map_frame = b->weak_from_this();
        return transform_pending.suspend(cont);
    });

    void* r = f->collect(&collector, &completion);
    assert_true(intrinsics::is_coroutine_suspended(r));
    assert_equals(0, completion.resumes);
    assert_false(map_frame.expired());

    // Step 1: Resume transform with heap box
    transform_pending.resume(Result<void*>::success(new std::string("transformed_1")));
    // MapFrame unboxed the string into transformed_ and emitted downstream -> collector suspended!
    assert_equals(0, completion.resumes);
    assert_equals(size_t(1), collector.values.size());
    assert_true(collector.values[0] == "transformed_1");

    // Step 2: Resume downstream collector
    collector.pending.resume(Result<void*>::success(nullptr));
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_success());
    assert_equals(1, trace.finally_calls);
    assert_true(trace.frame.expired());
    assert_true(map_frame.expired());
    assert_true(collector.pending.frame.expired());
}

// 4. MapNotNull optional suspension
void test_map_not_null_suspension() {
    Trace trace;
    Completion completion;
    Collector<int> collector;
    collector.pause = true;

    Pending transform_pending;
    auto f = map_not_null<int, int>(source(trace, 2), [&](int x, Continuation<void*>* cont) -> void* {
        return transform_pending.suspend(cont);
    });

    assert_true(intrinsics::is_coroutine_suspended(f->collect(&collector, &completion)));

    // First item returns std::nullopt (disengaged) -> should NOT emit downstream
    transform_pending.resume(Result<void*>::success(new std::optional<int>(std::nullopt)));
    assert_equals(size_t(0), collector.values.size());
    assert_equals(0, completion.resumes);

    // Second item returns engaged value 999 -> emits downstream and suspends collector
    transform_pending.resume(Result<void*>::success(new std::optional<int>(999)));
    assert_equals(size_t(1), collector.values.size());
    assert_equals(999, collector.values[0]);

    // Resume collector
    collector.pending.resume();
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_success());
    assert_equals(1, trace.finally_calls);
}

// 5. OnEach suspension FOLLOWED BY downstream suspension
void test_on_each_suspension_followed_by_downstream_suspension() {
    Trace trace;
    Completion completion;
    Collector<int> collector;
    collector.pause = true;

    Pending action_pending;
    std::weak_ptr<BaseContinuationImpl> on_each_frame;

    auto f = on_each<int>(source(trace, 1), [&](const int& x, Continuation<void*>* cont) -> void* {
        if (auto* b = dynamic_cast<BaseContinuationImpl*>(cont)) on_each_frame = b->weak_from_this();
        return action_pending.suspend(cont);
    });

    assert_true(intrinsics::is_coroutine_suspended(f->collect(&collector, &completion)));
    assert_equals(size_t(0), collector.values.size());

    // Step 1: Resume action (Unit = nullptr)
    action_pending.resume(Result<void*>::success(nullptr));
    // Action finished -> downstream emit suspended
    assert_equals(size_t(1), collector.values.size());
    assert_equals(1, collector.values[0]);

    // Step 2: Resume downstream
    collector.pending.resume();
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_success());
    assert_equals(1, trace.finally_calls);
    assert_true(on_each_frame.expired());
}

// 6. RunningFold initial emission delay prevents upstream collection start
void test_running_fold_delayed_initial_emit() {
    Trace trace;
    Completion completion;
    Collector<int> collector;
    collector.pause = true;

    auto f = running_fold<int, int>(source(trace, 2), 100, [](int acc, int x) { return acc + x; });
    assert_true(intrinsics::is_coroutine_suspended(f->collect(&collector, &completion)));

    // Initial emission has occurred (100), but upstream has NOT started yet!
    assert_equals(size_t(1), collector.values.size());
    assert_equals(100, collector.values[0]);
    assert_equals(0, trace.collection_starts);
    assert_equals(0, trace.completed_emits);

    // Resume initial emission -> upstream begins and emits 1
    collector.pending.resume();
    assert_equals(size_t(2), collector.values.size());
    assert_equals(101, collector.values[1]);
    assert_equals(1, trace.collection_starts);
    assert_equals(0, trace.completed_emits);

    // Resume next emission -> upstream emits 2
    collector.pending.resume();
    assert_equals(size_t(3), collector.values.size());
    assert_equals(103, collector.values[2]);
    assert_equals(1, trace.completed_emits);

    // Final resume settles collection
    collector.pending.resume();
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_success());
    assert_equals(1, trace.finally_calls);
}

// 7. RunningFold operation failure does not update accumulator or emit stale value
void test_running_fold_operation_failure() {
    Trace trace;
    Completion completion;
    Collector<int> collector;
    collector.pause = false;

    Pending op_pending;
    auto f = running_fold<int, int>(source(trace, 2), 50, [&](int acc, int x, Continuation<void*>* cont) -> void* {
        if (x == 2) {
            return op_pending.suspend(cont);
        }
        return new int(acc + x);
    });

    assert_true(intrinsics::is_coroutine_suspended(f->collect(&collector, &completion)));
    assert_equals(size_t(2), collector.values.size());
    assert_equals(50, collector.values[0]); // initial
    assert_equals(51, collector.values[1]); // 50 + 1

    // Fail operation for element 2
    auto ex = std::make_exception_ptr(TestException("fold op failed"));
    op_pending.resume(Result<void*>::failure(ex));

    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_failure());
    assert_true(completion.result.exception_or_null() == ex);
    // Values must NOT contain a third stale emission
    assert_equals(size_t(2), collector.values.size());
    assert_equals(1, trace.finally_calls);
}

// RunningReduce accepts elements without a default constructor.
void test_running_reduce_non_default_constructible() {

    std::vector<NoDefault> items;
    items.emplace_back(10);
    items.emplace_back(20);
    items.emplace_back(30);

    auto f = as_flow(std::move(items));
    auto reduced = running_reduce<NoDefault>(f, [](const NoDefault& acc, NoDefault val) {
        return NoDefault(acc.value + val.value);
    });

    auto res = collect_now(reduced);
    assert_equals(size_t(3), res.size());
    assert_equals(10, res[0].value);
    assert_equals(30, res[1].value);
    assert_equals(60, res[2].value);

    // The deleted default constructor is checked by instantiating the real operator.
}

// 9. RunningReduce with nullable-first element (std::optional<int>)
void test_running_reduce_nullable_first() {
    int op_calls = 0;
    std::vector<std::optional<int>> items{
        std::nullopt,
        std::optional<int>(10),
        std::optional<int>(20)
    };

    auto f = as_flow(items);
    auto reduced = running_reduce<std::optional<int>>(f, [&](const std::optional<int>& acc, std::optional<int> val) -> std::optional<int> {
        ++op_calls;
        int acc_val = acc.value_or(0);
        int item_val = val.value_or(0);
        return std::optional<int>(acc_val + item_val);
    });

    auto res = collect_now(reduced);
    assert_equals(size_t(3), res.size());
    // First element must be nullopt, and operation was NOT called on it!
    assert_false(res[0].has_value());
    assert_true(res[1].has_value() && *res[1] == 10);
    assert_true(res[2].has_value() && *res[2] == 30);
    // Operation was called exactly twice (for 2nd and 3rd elements)
    assert_equals(2, op_calls);
}

// 10. Chunked buffer retention during pending downstream emission
void test_chunked_suspension_buffer_retention() {
    Trace trace;
    Completion completion;
    Collector<std::vector<int>> collector;
    collector.pause = true;

    auto f = chunked(source(trace, 3), 2); // will emit [1, 2] then [3]
    assert_true(intrinsics::is_coroutine_suspended(f->collect(&collector, &completion)));

    // Full chunk [1, 2] is emitted, collector is paused
    assert_equals(size_t(1), collector.values.size());
    assert_true(collector.values[0] == std::vector<int>({1, 2}));
    assert_false(collector.pending.frame.expired());

    // Resume first chunk
    collector.pending.resume();
    // Upstream finishes and flushes partial chunk [3]
    assert_equals(size_t(2), collector.values.size());
    assert_true(collector.values[1] == std::vector<int>({3}));

    // Resume partial chunk
    collector.pending.resume();
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_success());
    assert_equals(1, trace.finally_calls);
    assert_true(collector.pending.frame.expired());
}

// 11. Chunked failure does NOT flush partial chunk
void test_chunked_failure_does_not_flush_partial_chunk() {
    Trace trace;
    Completion completion;
    Collector<std::vector<int>> collector;
    collector.pause = false;

    // Upstream emits 1 element, then fails (chunk size 2 means 1 element is partial)
    auto err_flow = kotlinx::coroutines::flow::internal::unsafe_flow<int>([](FlowCollector<int>* collector, Continuation<void*>* completion) -> void* {
        class FailFrame final : public ContinuationImpl {
        public:
            FailFrame(FlowCollector<int>* collector, Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  collector_(collector) {}
            void retain() { self_ref_ = shared_from_this(); }
            void* invoke_suspend(Result<void*> result) override {
                coroutine_begin(this)
                coroutine_yield(this, collector_->emit(1, this));
                throw TestException("upstream failed before full chunk");
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
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        auto frame = std::make_shared<FailFrame>(collector, completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });

    auto f = chunked(err_flow, 2);
    bool caught = false;
    try {
        void* r = f->collect(&collector, &completion);
        if (intrinsics::is_coroutine_suspended(r)) {
            completion.result.get_or_throw();
        }
    } catch (const TestException&) {
        caught = true;
    }
    assert_true(caught);
    // Partial chunk [1] must NOT be flushed!
    assert_equals(size_t(0), collector.values.size());
}

// 12. Interleaved concurrent collections isolation
void test_interleaved_concurrent_collections() {
    Trace trace;
    auto base_flow = source(trace, 2);
    auto map_flow = map<int, int>(base_flow, [](int x) { return x * 10; });

    Collector<int> col1;
    col1.pause = true;
    Completion comp1;

    Collector<int> col2;
    col2.pause = true;
    Completion comp2;

    assert_true(intrinsics::is_coroutine_suspended(map_flow->collect(&col1, &comp1)));
    assert_true(intrinsics::is_coroutine_suspended(map_flow->collect(&col2, &comp2)));

    assert_equals(size_t(1), col1.values.size());
    assert_equals(10, col1.values[0]);
    assert_equals(size_t(1), col2.values.size());
    assert_equals(10, col2.values[0]);

    // Interleaved resumes
    col1.pending.resume();
    assert_equals(size_t(2), col1.values.size());
    assert_equals(20, col1.values[1]);

    col2.pending.resume();
    assert_equals(size_t(2), col2.values.size());
    assert_equals(20, col2.values[1]);

    col1.pending.resume();
    assert_equals(1, comp1.resumes);
    assert_true(comp1.result.is_success());

    col2.pending.resume();
    assert_equals(1, comp2.resumes);
    assert_true(comp2.result.is_success());
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/FilterTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/MapTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/MapNotNullTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/OnEachTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/IndexedTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/ScanTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/ChunkedTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/flow/operators/FilterTrivialTest.kt
void test_original_nominal_cases() {
    auto two = as_flow(std::vector<int>{1, 2});
    assert_true(collect_now(filter(two, [](int v) { return v % 2 == 0; })) == std::vector<int>{2});
    assert_true(collect_now(filter(two, [](int) { return true; })) == std::vector<int>({1, 2}));
    assert_true(collect_now(filter(two, [](int) { return false; })).empty());
    assert_true(collect_now(filter_not(two, [](int) { return true; })).empty());
    assert_true(collect_now(filter_not(two, [](int) { return false; })) == std::vector<int>({1, 2}));
    assert_true(collect_now(map<int, int>(two, [](int v) { return v + 1; })) == std::vector<int>({2, 3}));
    std::vector<int> actions;
    assert_true(collect_now(on_each(two, [&](int v) { actions.push_back(v); })) == std::vector<int>({1, 2}));
    assert_true(actions == std::vector<int>({1, 2}));
    auto nullable = as_flow(std::vector<std::optional<int>>{1, std::nullopt, 2});
    assert_true(collect_now(map_not_null<std::optional<int>, int>(nullable,
        [](std::optional<int> v) { return v; })) == std::vector<int>({1, 2}));
    assert_true(collect_now(filter_not_null(as_flow(std::vector<std::optional<int>>{1, 2, std::nullopt}))) == std::vector<int>({1, 2}));
    assert_true(collect_now(filter_not_null(empty_flow<std::optional<int>>())).empty());
    assert_true(collect_now(with_index(as_flow(std::vector<int>{3, 2, 1}))) ==
        std::vector<IndexedValue<int>>({{0, 3}, {1, 2}, {2, 1}}));
    auto append = [](const std::vector<int>& acc, int v) { auto result = acc; result.push_back(v); return result; };
    auto three = as_flow(std::vector<int>{1, 2, 3});
    const std::vector<std::vector<int>> expected{{}, {1}, {1, 2}, {1, 2, 3}};
    assert_true(collect_now(scan<int, std::vector<int>>(three, {}, append)) == expected);
    assert_true(collect_now(running_fold<int, std::vector<int>>(three, {}, append)) == expected);
    assert_true(collect_now(running_reduce(as_flow(std::vector<int>{1, 2, 3, 4, 5}),
        [](int a, int v) { return a + v; })) == std::vector<int>({1, 3, 6, 10, 15}));
    auto nulls = as_flow(std::vector<std::optional<int>>{std::nullopt, 2, std::nullopt, std::nullopt, std::nullopt, 5});
    assert_true(collect_now(running_reduce(nulls, [](std::optional<int> a, std::optional<int> v) {
        return v ? (a ? std::optional<int>(*a + *v) : v) : a;
    })) == std::vector<std::optional<int>>({std::nullopt, 2, 2, 2, 2, 7}));
    auto five = as_flow(std::vector<int>{1, 2, 3, 4, 5});
    assert_true(collect_now(chunked(five, 2)) == std::vector<std::vector<int>>({{1, 2}, {3, 4}, {5}}));
    assert_true(collect_now(chunked(five, 3)) == std::vector<std::vector<int>>({{1, 2, 3}, {4, 5}}));
    assert_true(collect_now(chunked(as_flow(std::vector<int>{1, 2, 3, 4}), 2)) == std::vector<std::vector<int>>({{1, 2}, {3, 4}}));
    assert_true(collect_now(chunked(as_flow(std::vector<int>{1}), 3)) == std::vector<std::vector<int>>({{1}}));
    assert_true(collect_now(chunked(empty_flow<int>(), 1)).empty());
    assert_true(collect_now(chunked(empty_flow<int>(), 2)).empty());
    for (int invalid : {-1, 0, INT_MIN, INT_MIN + 1}) {
        bool failed = false;
        try { chunked(empty_flow<int>(), invalid); }
        catch (const std::invalid_argument&) { failed = true; }
        assert_true(failed);
    }
    auto strings = chunked(as_flow(std::vector<std::string>{"a", "b", "c", "d", "e"}), 2);
    assert_true(collect_now(map<std::vector<std::string>, std::string>(strings,
        [](std::vector<std::string> chunk) { std::string result; for (const auto& v : chunk) result += v; return result; })) ==
        std::vector<std::string>({"ab", "cd", "e"}));
    int empty_calls = 0;
    assert_true(collect_now(map<int, int>(empty_flow<int>(), [&](int v) { ++empty_calls; return v; })).empty());
    assert_true(collect_now(map_not_null<int, int>(empty_flow<int>(), [&](int v) -> std::optional<int> { ++empty_calls; return v; })).empty());
    assert_true(collect_now(on_each(empty_flow<int>(), [&](int) { ++empty_calls; })).empty());
    assert_equals(0, empty_calls);
}

void test_pointer_overload_compatibility() {
    int a = 1, b = 2;
    auto src = as_flow(std::vector<int>{1, 2, 3});
    std::function<int*(int)> raw = [&](int v) { return v == 1 ? &a : (v == 3 ? &b : nullptr); };
    assert_true(collect_now(map_not_null<int, int>(src, raw)) == std::vector<int>({1, 2}));
    std::string borrowed = "owned by caller";
    assert_true(collect_now(map_not_null<int, std::string>(src, [&](int v) -> std::string* {
        return v == 2 ? nullptr : &borrowed;
    })) == std::vector<std::string>({"owned by caller", "owned by caller"}));
    assert_true(borrowed == "owned by caller");
    auto shared = std::make_shared<int>(3);
    std::function<std::shared_ptr<int>(int)> ptr = [shared](int v) { return v == 2 ? nullptr : shared; };
    assert_true(collect_now(map_not_null<int, int>(src, ptr)) == std::vector<std::shared_ptr<int>>({shared, shared}));
    assert_true(collect_now(map_not_null<int, int>(src, [shared](int v) -> std::shared_ptr<int> {
        return v == 2 ? nullptr : shared;
    })) == std::vector<std::shared_ptr<int>>({shared, shared}));
    assert_true(collect_now(filter_not_null(as_flow(std::vector<std::shared_ptr<int>>{shared, nullptr, shared}))) ==
        std::vector<std::shared_ptr<int>>({shared, shared}));
}

void test_stateful_interleaved_collections() {
    Trace trace;
    auto src = source(trace, 2);
    auto indexed = with_index(src);
    Collector<IndexedValue<int>> a, b;
    a.pause = b.pause = true;
    Completion ca, cb;
    assert_true(intrinsics::is_coroutine_suspended(indexed->collect(&a, &ca)));
    assert_true(intrinsics::is_coroutine_suspended(indexed->collect(&b, &cb)));
    a.pending.resume(); b.pending.resume();
    assert_true(a.values == std::vector<IndexedValue<int>>({{0, 1}, {1, 2}}));
    assert_true(a.values == b.values);
    b.pending.resume(); a.pending.resume();
    assert_equals(1, ca.resumes); assert_equals(1, cb.resumes);
    assert_true(ca.result.is_success() && cb.result.is_success());

    auto folded = running_fold<int, int>(src, 10, [](int acc, int v) { return acc + v; });
    Collector<int> x, y;
    x.pause = y.pause = true;
    Completion cx, cy;
    assert_true(intrinsics::is_coroutine_suspended(folded->collect(&x, &cx)));
    assert_true(intrinsics::is_coroutine_suspended(folded->collect(&y, &cy)));
    for (int i = 0; i < 3; ++i) { y.pending.resume(); x.pending.resume(); }
    assert_true(x.values == std::vector<int>({10, 11, 13}));
    assert_true(x.values == y.values);
    assert_equals(1, cx.resumes); assert_equals(1, cy.resumes);
    assert_true(cx.result.is_success() && cy.result.is_success());
}

void test_real_action_and_operation_cancellation() {
    for (bool reduction : {false, true}) {
        auto job = JobImpl::create(nullptr);
        Completion completion;
        completion.context = job;
        Trace trace;
        Collector<int> sink;
        auto capture = std::make_shared<int>(42);
        std::weak_ptr<int> weak_capture = capture;
        auto upstream = source(trace, 3);
        std::weak_ptr<Flow<int>> weak_upstream = upstream;
        std::weak_ptr<BaseContinuationImpl> weak_frame;
        std::shared_ptr<CancellableContinuationImpl<void>> action_pending;
        std::shared_ptr<CancellableContinuationImpl<int>> operation_pending;
        {
            std::shared_ptr<Flow<int>> transformed;
            if (reduction) {
                transformed = running_reduce<int>(upstream, [&, capture](int acc, int value, Continuation<void*>* c) -> void* {
                    assert_equals(42, *capture); assert_equals(1, acc); assert_equals(2, value);
                    weak_frame = dynamic_cast<BaseContinuationImpl*>(c)->weak_from_this();
                    return suspend_cancellable_coroutine<int>([&](CancellableContinuation<int>& p) {
                        operation_pending = dynamic_cast<CancellableContinuationImpl<int>&>(p).shared_from_this();
                    }, c);
                });
            } else {
                transformed = on_each<int>(upstream, [&, capture](int value, Continuation<void*>* c) -> void* {
                    assert_equals(42, *capture); assert_equals(1, value);
                    weak_frame = dynamic_cast<BaseContinuationImpl*>(c)->weak_from_this();
                    return suspend_cancellable_coroutine<void>([&](CancellableContinuation<void>& p) {
                        action_pending = dynamic_cast<CancellableContinuationImpl<void>&>(p).shared_from_this();
                    }, c);
                });
            }
            assert_true(intrinsics::is_coroutine_suspended(transformed->collect(&sink, &completion)));
        }
        capture.reset(); upstream.reset();
        assert_false(weak_frame.expired()); assert_false(weak_capture.expired()); assert_false(weak_upstream.expired());
        assert_equals(0, completion.resumes);
        job->cancel();
        assert_equals(1, completion.resumes);
        assert_true(completion.result.is_failure());
        bool cancelled = false;
        try { completion.result.get_or_throw(); } catch (const CancellationException&) { cancelled = true; }
        assert_true(cancelled);
        assert_equals(1, trace.finally_calls);
        assert_true(sink.values == (reduction ? std::vector<int>{1} : std::vector<int>{}));
        action_pending.reset(); operation_pending.reset();
        assert_true(weak_frame.expired()); assert_true(trace.frame.expired());
        assert_true(weak_capture.expired()); assert_true(weak_upstream.expired());
    }
}

void test_flatten_concat_retains_suspended_collector() {
    Trace first, second;
    first.pause_upstream = second.pause_upstream = true;
    auto outer = flow_of<std::shared_ptr<Flow<int>>>({source(first, 2), source(second, 2)});
    auto merged = flatten_merge<int>(outer, 1);
    Collector<int> sink;
    Completion completion;
    assert_true(intrinsics::is_coroutine_suspended(merged->collect(&sink, &completion)));
    first.upstream.resume();
    assert_equals(1, first.finally_calls);
    assert_equals(1, second.collection_starts);
    assert_equals(0, completion.resumes);
    second.upstream.resume();
    assert_true(sink.values == (std::vector<int>{1, 2, 1, 2}));
    assert_equals(1, completion.resumes);
    assert_true(completion.result.is_success());
}

void test_flatten_concat_owns_transient_inner(int mode) {
    Trace trace;
    trace.pause_upstream = true;
    std::weak_ptr<Flow<int>> inner_lifetime;
    std::weak_ptr<BaseContinuationImpl> collect_lifetime;
    auto outer = kotlinx::coroutines::flow::internal::unsafe_flow<std::shared_ptr<Flow<int>>>(
        [&](auto* collector, auto* completion) -> void* {
            collect_lifetime = dynamic_cast<BaseContinuationImpl*>(completion)->weak_from_this();
            auto inner = source(trace, 2);
            inner_lifetime = inner;
            return collector->emit(std::move(inner), completion);
        });
    std::weak_ptr<Flow<std::shared_ptr<Flow<int>>>> outer_lifetime = outer;
    auto merged = flatten_concat<int>(outer);
    Collector<int> sink;
    sink.pause = true;
    Completion completion;
    auto job = mode == 3 ? JobImpl::create(nullptr) : nullptr;
    if (job) completion.context = job;
    assert_true(intrinsics::is_coroutine_suspended(merged->collect(&sink, &completion)));
    outer.reset(); merged.reset();
    assert_false(outer_lifetime.expired());
    assert_false(inner_lifetime.expired());
    assert_false(collect_lifetime.expired());
    auto failure = std::make_exception_ptr(std::runtime_error("flatten failure"));
    if (mode == 3) { job->cancel(); failure = job->get_cancellation_exception(); }
    trace.upstream.resume(mode == 1 ? Result<void*>::failure(failure) : Result<void*>::success(nullptr));
    if (mode == 0 || mode == 2) {
        assert_true(sink.values == (std::vector<int>{1}));
        assert_equals(0, completion.resumes);
        assert_false(inner_lifetime.expired());
        sink.pending.resume(mode == 2 ? Result<void*>::failure(failure) : Result<void*>::success(nullptr));
        if (mode == 0) {
            assert_true(sink.values == (std::vector<int>{1, 2}));
            assert_equals(0, completion.resumes);
            sink.pending.resume();
        }
    }
    assert_equals(1, completion.resumes);
    assert_equals(1, trace.finally_calls);
    if (mode == 0) assert_true(completion.result.is_success());
    else {
        assert_true(completion.result.exception_or_null() == failure);
        assert_true(trace.failure == failure);
    }
    assert_true(collect_lifetime.expired());
    assert_true(trace.frame.expired());
    assert_true(inner_lifetime.expired());
    assert_true(outer_lifetime.expired());
}

} // namespace

int main() {
    std::cout << "Running test_transform_suspension..." << std::endl;

    test_flatten_concat_retains_suspended_collector();
    for (int mode : {0, 1, 2, 3}) test_flatten_concat_owns_transient_inner(mode);
    test_filter_and_filter_not_nominal();
    std::cout << "  test_filter_and_filter_not_nominal passed" << std::endl;

    test_filter_is_instance_and_not_null();
    std::cout << "  test_filter_is_instance_and_not_null passed" << std::endl;

    test_map_and_map_not_null_nominal();
    std::cout << "  test_map_and_map_not_null_nominal passed" << std::endl;

    test_with_index_nominal_and_isolation();
    std::cout << "  test_with_index_nominal_and_isolation passed" << std::endl;

    test_index_overflow_guard();
    std::cout << "  test_index_overflow_guard passed" << std::endl;

    test_on_each_nominal();
    std::cout << "  test_on_each_nominal passed" << std::endl;

    test_scan_and_running_fold_nominal();
    std::cout << "  test_scan_and_running_fold_nominal passed" << std::endl;

    test_running_reduce_nominal();
    std::cout << "  test_running_reduce_nominal passed" << std::endl;

    test_chunked_nominal();
    std::cout << "  test_chunked_nominal passed" << std::endl;

    test_filter_suspension_followed_by_downstream_suspension();
    std::cout << "  test_filter_suspension_followed_by_downstream_suspension passed" << std::endl;

    test_filter_real_cancellation_during_predicate();
    std::cout << "  test_filter_real_cancellation_during_predicate passed" << std::endl;

    test_map_suspension_followed_by_downstream_suspension();
    std::cout << "  test_map_suspension_followed_by_downstream_suspension passed" << std::endl;

    test_map_not_null_suspension();
    std::cout << "  test_map_not_null_suspension passed" << std::endl;

    test_on_each_suspension_followed_by_downstream_suspension();
    std::cout << "  test_on_each_suspension_followed_by_downstream_suspension passed" << std::endl;

    test_running_fold_delayed_initial_emit();
    std::cout << "  test_running_fold_delayed_initial_emit passed" << std::endl;

    test_running_fold_operation_failure();
    std::cout << "  test_running_fold_operation_failure passed" << std::endl;

    test_running_reduce_non_default_constructible();
    std::cout << "  test_running_reduce_non_default_constructible passed" << std::endl;

    test_running_reduce_nullable_first();
    std::cout << "  test_running_reduce_nullable_first passed" << std::endl;

    test_chunked_suspension_buffer_retention();
    std::cout << "  test_chunked_suspension_buffer_retention passed" << std::endl;

    test_chunked_failure_does_not_flush_partial_chunk();
    std::cout << "  test_chunked_failure_does_not_flush_partial_chunk passed" << std::endl;

    test_interleaved_concurrent_collections();
    std::cout << "  test_interleaved_concurrent_collections passed" << std::endl;

    test_original_nominal_cases();
    std::cout << "  test_original_nominal_cases passed" << std::endl;
    test_pointer_overload_compatibility();
    std::cout << "  test_pointer_overload_compatibility passed" << std::endl;
    test_stateful_interleaved_collections();
    std::cout << "  test_stateful_interleaved_collections passed" << std::endl;
    test_real_action_and_operation_cancellation();
    std::cout << "  test_real_action_and_operation_cancellation passed" << std::endl;
    std::cout << "All test_transform_suspension tests passed successfully!" << std::endl;
    return 0;
}
