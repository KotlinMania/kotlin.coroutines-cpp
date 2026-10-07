#include "kotlinx/coroutines/flow/Collect.hpp"
#include "kotlinx/coroutines/flow/Reduce.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::flow;
using namespace kotlinx::coroutines::testing;

class Completion final : public Continuation<void*> {
public:
    int resumes = 0;
    Result<void*> result = Result<void*>::success(nullptr);
    std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance();
    std::shared_ptr<CoroutineContext> get_context() const override {
        return context;
    }
    void resume_with(Result<void*> outcome) override {
        ++resumes;
        result = std::move(outcome);
    }
};

template <typename T>
class AccumulatorCollector : public FlowCollector<T> {
public:
    std::vector<T> items;
    void* emit(T value, Continuation<void*>*) override {
        items.push_back(std::move(value));
        return nullptr;
    }
};

template <typename T>
std::shared_ptr<Flow<T>> make_test_flow(std::vector<T> values) {
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:85-90
    // The iterable frame must resume its loop after a suspended emit.
    return as_flow<T>(values);
}

template <typename T>
std::shared_ptr<Flow<T>> make_tracking_flow(std::vector<T> values, std::shared_ptr<std::vector<T>> emitted) {
    return flow::flow<T>([values = std::move(values), emitted](FlowCollector<T>* col, Continuation<void*>* c) -> void* {
        for (const auto& item : values) {
            emitted->push_back(item);
            col->emit(item, c);
        }
        return nullptr;
    });
}

// ----------------------------------------------------------------------------
// Collect tests
// ----------------------------------------------------------------------------

void test_collect_nop() {
    Completion completion;
    auto tracked = std::make_shared<std::vector<int>>();
    auto f = make_tracking_flow<int>({1, 2, 3}, tracked);
    collect<int>(f, &completion);
    assert_equals(std::vector<int>{1, 2, 3}, *tracked);
    std::cout << "test_collect_nop completed" << std::endl;
}

void test_collect_action() {
    Completion completion;
    auto f = make_test_flow<int>({10, 20, 30});
    std::vector<int> collected;
    collect<int>(f, [&collected](int val) {
        collected.push_back(val);
    }, &completion);
    assert_equals(std::vector<int>{10, 20, 30}, collected);

    // Suspending action overload
    std::vector<int> collected_susp;
    collect<int>(f, [&collected_susp](int val, Continuation<void*>*) -> void* {
        collected_susp.push_back(val * 2);
        return nullptr;
    }, &completion);
    assert_equals(std::vector<int>{20, 40, 60}, collected_susp);
    std::cout << "test_collect_action completed" << std::endl;
}

void test_launch_in() {
    auto scope = create_coroutine_scope(std::dynamic_pointer_cast<CoroutineContext>(make_job())->operator+(
        std::shared_ptr<CoroutineContext>(&Dispatchers::get_unconfined(), [](CoroutineContext*) {})));
    auto tracked = std::make_shared<std::vector<int>>();
    auto f = make_tracking_flow<int>({100, 200}, tracked);
    auto job = launch_in<int>(f, scope.get());
    assert_not_null(job.get());
    assert_equals(std::vector<int>{100, 200}, *tracked);
    std::cout << "test_launch_in completed" << std::endl;
}

void test_collect_indexed() {
    Completion completion;
    auto f = make_test_flow<std::string>({"apple", "banana", "cherry"});
    std::vector<std::pair<int, std::string>> collected;
    collect_indexed<std::string>(f, [&collected](int idx, std::string val) {
        collected.emplace_back(idx, std::move(val));
    }, &completion);

    assert_equals(static_cast<size_t>(3), collected.size());
    assert_equals(0, collected[0].first);
    assert_equals(std::string("apple"), collected[0].second);
    assert_equals(1, collected[1].first);
    assert_equals(std::string("banana"), collected[1].second);
    assert_equals(2, collected[2].first);
    assert_equals(std::string("cherry"), collected[2].second);

    // Suspending action overload
    std::vector<int> indices;
    collect_indexed<std::string>(f, [&indices](int idx, std::string, Continuation<void*>*) -> void* {
        indices.push_back(idx);
        return nullptr;
    }, &completion);
    assert_equals(std::vector<int>{0, 1, 2}, indices);
    std::cout << "test_collect_indexed completed" << std::endl;
}

void test_collect_latest() {
    Completion completion;
    completion.context = std::shared_ptr<CoroutineContext>(
        &Dispatchers::get_unconfined(), [](CoroutineContext*) {});
    auto f = make_test_flow<int>({1, 2, 3});
    std::vector<int> out;
    collect_latest<int>(f, [&out](int val) {
        out.push_back(val);
    }, &completion);
    // In sequential test, all items complete
    assert_equals(static_cast<size_t>(3), out.size());
    assert_equals(1, out[0]);
    assert_equals(2, out[1]);
    assert_equals(3, out[2]);
    std::cout << "test_collect_latest completed" << std::endl;
}

void test_emit_all() {
    Completion completion;
    auto f = make_test_flow<int>({5, 10, 15});
    AccumulatorCollector<int> col;
    emit_all<int>(&col, f, &completion);
    assert_equals(std::vector<int>{5, 10, 15}, col.items);
    std::cout << "test_emit_all completed" << std::endl;
}

// ----------------------------------------------------------------------------
// Reduce tests
// ----------------------------------------------------------------------------

void test_reduce() {
    Completion completion;
    auto f = make_test_flow<int>({1, 2, 3, 4});
    void* res_ptr = reduce<int>(f, [](int acc, int val) { return acc + val; }, &completion);
    auto* sum = static_cast<int*>(res_ptr);
    assert_equals(10, *sum);
    delete sum;

    // Multiplication
    auto f2 = make_test_flow<int>({2, 3, 4});
    void* mul_ptr = reduce<int>(f2, [](int acc, int val) { return acc * val; }, &completion);
    auto* mul = static_cast<int*>(mul_ptr);
    assert_equals(24, *mul);
    delete mul;

    // Suspending overload
    void* susp_ptr = reduce<int>(f, [](int acc, int val, Continuation<void*>*) -> void* {
        return new int(acc + val);
    }, &completion);
    auto* susp_sum = static_cast<int*>(susp_ptr);
    assert_equals(10, *susp_sum);
    delete susp_sum;

    // Empty flow throws NoSuchElementException
    auto empty = make_test_flow<int>({});
    bool caught = false;
    try {
        reduce<int>(empty, [](int acc, int val) { return acc + val; }, &completion);
    } catch (const NoSuchElementException&) {
        caught = true;
    }
    assert_true(caught);

    std::cout << "test_reduce completed" << std::endl;
}

void test_fold() {
    Completion completion;
    auto f = make_test_flow<int>({1, 2, 3});
    void* res_ptr = fold<int, int>(f, 10, [](int acc, int val) { return acc + val; }, &completion);
    auto* sum = static_cast<int*>(res_ptr);
    assert_equals(16, *sum);
    delete sum;

    // Empty flow returns initial value
    auto empty = make_test_flow<int>({});
    void* empty_ptr = fold<int, int>(empty, 42, [](int acc, int val) { return acc + val; }, &completion);
    auto* empty_res = static_cast<int*>(empty_ptr);
    assert_equals(42, *empty_res);
    delete empty_res;

    // Suspending fold overload
    void* susp_ptr = fold<int, int>(f, 100, [](int acc, int val, Continuation<void*>*) -> void* {
        return new int(acc + val);
    }, &completion);
    auto* susp_sum = static_cast<int*>(susp_ptr);
    assert_equals(106, *susp_sum);
    delete susp_sum;

    std::cout << "test_fold completed" << std::endl;
}

void test_single() {
    Completion completion;
    auto f = make_test_flow<int>({42});
    void* res_ptr = single<int>(f, &completion);
    auto* val = static_cast<int*>(res_ptr);
    assert_equals(42, *val);
    delete val;

    // Empty flow throws NoSuchElementException
    auto empty = make_test_flow<int>({});
    bool caught_empty = false;
    try {
        single<int>(empty, &completion);
    } catch (const NoSuchElementException&) {
        caught_empty = true;
    }
    assert_true(caught_empty);

    // Flow with more than one element throws std::invalid_argument
    auto multi = make_test_flow<int>({1, 2});
    bool caught_multi = false;
    try {
        single<int>(multi, &completion);
    } catch (const std::invalid_argument&) {
        caught_multi = true;
    }
    assert_true(caught_multi);

    std::cout << "test_single completed" << std::endl;
}

void test_single_or_null() {
    Completion completion;
    auto f = make_test_flow<int>({77});
    void* res_ptr = single_or_null<int>(f, &completion);
    assert_not_null(res_ptr);
    auto* val = static_cast<int*>(res_ptr);
    assert_equals(77, *val);
    delete val;

    // Empty flow returns nullptr
    auto empty = make_test_flow<int>({});
    void* empty_ptr = single_or_null<int>(empty, &completion);
    assert_null(empty_ptr);

    // Multi-element flow returns nullptr and aborts early
    auto tracked = std::make_shared<std::vector<int>>();
    auto multi = make_tracking_flow<int>({1, 2, 3, 4}, tracked);
    void* multi_ptr = single_or_null<int>(multi, &completion);
    assert_null(multi_ptr);
    // Aborted after second element!
    assert_equals(static_cast<size_t>(2), tracked->size());

    std::cout << "test_single_or_null completed" << std::endl;
}

void test_first() {
    Completion completion;
    auto tracked = std::make_shared<std::vector<int>>();
    auto f = make_tracking_flow<int>({10, 20, 30}, tracked);
    void* res_ptr = first<int>(f, &completion);
    assert_not_null(res_ptr);
    auto* val = static_cast<int*>(res_ptr);
    assert_equals(10, *val);
    delete val;
    // Short-circuited after first emission!
    assert_equals(static_cast<size_t>(1), tracked->size());

    // Empty flow throws NoSuchElementException
    auto empty = make_test_flow<int>({});
    bool caught = false;
    try {
        first<int>(empty, &completion);
    } catch (const NoSuchElementException&) {
        caught = true;
    }
    assert_true(caught);

    std::cout << "test_first completed" << std::endl;
}

void test_first_predicate() {
    Completion completion;
    auto tracked = std::make_shared<std::vector<int>>();
    auto f = make_tracking_flow<int>({1, 3, 4, 7, 8}, tracked);
    void* res_ptr = first<int>(f, [](const int& v) { return v % 2 == 0; }, &completion);
    assert_not_null(res_ptr);
    auto* val = static_cast<int*>(res_ptr);
    assert_equals(4, *val);
    delete val;
    // Short-circuited after 4 (emitted 1, 3, 4)
    assert_equals(static_cast<size_t>(3), tracked->size());

    // No matching elements throws NoSuchElementException
    auto f2 = make_test_flow<int>({1, 3, 5});
    bool caught = false;
    try {
        first<int>(f2, [](const int& v) { return v % 2 == 0; }, &completion);
    } catch (const NoSuchElementException&) {
        caught = true;
    }
    assert_true(caught);

    std::cout << "test_first_predicate completed" << std::endl;
}

void test_first_or_null() {
    Completion completion;
    auto tracked = std::make_shared<std::vector<int>>();
    auto f = make_tracking_flow<int>({99, 100}, tracked);
    void* res_ptr = first_or_null<int>(f, &completion);
    assert_not_null(res_ptr);
    auto* val = static_cast<int*>(res_ptr);
    assert_equals(99, *val);
    delete val;
    assert_equals(static_cast<size_t>(1), tracked->size());

    // Empty flow returns nullptr
    auto empty = make_test_flow<int>({});
    void* empty_ptr = first_or_null<int>(empty, &completion);
    assert_null(empty_ptr);

    std::cout << "test_first_or_null completed" << std::endl;
}

void test_first_or_null_predicate() {
    Completion completion;
    auto tracked = std::make_shared<std::vector<int>>();
    auto f = make_tracking_flow<int>({1, 3, 6, 7}, tracked);
    void* res_ptr = first_or_null<int>(f, [](const int& v) { return v % 2 == 0; }, &completion);
    assert_not_null(res_ptr);
    auto* val = static_cast<int*>(res_ptr);
    assert_equals(6, *val);
    delete val;
    assert_equals(static_cast<size_t>(3), tracked->size());

    // No matching elements returns nullptr
    auto f2 = make_test_flow<int>({1, 3, 5});
    void* res_none = first_or_null<int>(f2, [](const int& v) { return v % 2 == 0; }, &completion);
    assert_null(res_none);

    std::cout << "test_first_or_null_predicate completed" << std::endl;
}

void test_last() {
    Completion completion;
    auto f = make_test_flow<int>({1, 2, 3});
    void* res_ptr = last<int>(f, &completion);
    assert_not_null(res_ptr);
    auto* val = static_cast<int*>(res_ptr);
    assert_equals(3, *val);
    delete val;

    // Empty flow throws NoSuchElementException
    auto empty = make_test_flow<int>({});
    bool caught = false;
    try {
        last<int>(empty, &completion);
    } catch (const NoSuchElementException&) {
        caught = true;
    }
    assert_true(caught);

    std::cout << "test_last completed" << std::endl;
}

void test_last_or_null() {
    Completion completion;
    auto f = make_test_flow<int>({10, 20, 30});
    void* res_ptr = last_or_null<int>(f, &completion);
    assert_not_null(res_ptr);
    auto* val = static_cast<int*>(res_ptr);
    assert_equals(30, *val);
    delete val;

    // Empty flow returns nullptr
    auto empty = make_test_flow<int>({});
    void* empty_ptr = last_or_null<int>(empty, &completion);
    assert_null(empty_ptr);

    std::cout << "test_last_or_null completed" << std::endl;
}

void test_suspended_fold_and_reduce() {
    class DeferredFlow : public Flow<int> {
    public:
        FlowCollector<int>* retained = nullptr;
        Continuation<void*>* completion = nullptr;
        void* collect(FlowCollector<int>* c, Continuation<void*>* cont) override {
            retained = c;
            completion = cont;
            return intrinsics::get_COROUTINE_SUSPENDED();
        }
    };

    // 1. Fold suspension test (immediate suspension return + deferred emissions)
    auto deferred = std::make_shared<DeferredFlow>();
    int resumed_val = 0;
    bool resumed = false;
    auto cont = make_continuation<void*>(EmptyCoroutineContext::instance(), [&resumed_val, &resumed](Result<void*> res) {
        resumed = true;
        if (res.is_success()) {
            auto* p = static_cast<int*>(res.get_or_throw());
            if (p) {
                resumed_val = *p;
                delete p;
            }
        }
    });

    void* r = fold<int, int>(deferred, 10, [](int acc, int val) { return acc + val; }, cont.get());
    assert_true(intrinsics::is_coroutine_suspended(r));
    assert_false(resumed);

    // Now emit values and complete via deferred continuation
    deferred->retained->emit(5, deferred->completion);
    deferred->retained->emit(15, deferred->completion);
    deferred->completion->resume_with(Result<void*>::success(nullptr));

    assert_true(resumed);
    assert_equals(30, resumed_val); // 10 + 5 + 15 = 30

    // 2. Reduce suspension test (empty flow fails only upon completion)
    auto empty_deferred = std::make_shared<DeferredFlow>();
    bool empty_failed = false;
    auto empty_cont = make_continuation<void*>(EmptyCoroutineContext::instance(), [&empty_failed](Result<void*> res) {
        if (res.is_failure()) {
            try {
                std::rethrow_exception(res.exception_or_null());
            } catch (const NoSuchElementException&) {
                empty_failed = true;
            }
        }
    });

    void* r_red = reduce<int>(empty_deferred, [](int acc, int val) { return acc + val; }, empty_cont.get());
    assert_true(intrinsics::is_coroutine_suspended(r_red));
    assert_false(empty_failed);

    // Complete empty flow
    empty_deferred->completion->resume_with(Result<void*>::success(nullptr));
    assert_true(empty_failed);

    std::cout << "test_suspended_fold_and_reduce completed" << std::endl;
}

int main() {
    test_collect_nop();
    test_collect_action();
    test_launch_in();
    test_collect_indexed();
    test_collect_latest();
    test_emit_all();

    test_reduce();
    test_fold();
    test_single();
    test_single_or_null();
    test_first();
    test_first_predicate();
    test_first_or_null();
    test_first_or_null_predicate();
    test_last();
    test_last_or_null();
    test_suspended_fold_and_reduce();

    std::cout << "All Collect and Reduce cases completed" << std::endl;
    return 0;
}
