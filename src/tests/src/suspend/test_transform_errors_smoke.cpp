#include "kotlinx/coroutines/flow/Transform.hpp"
#include "kotlinx/coroutines/flow/Errors.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::flow;

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
    return flow::flow<T>([values = std::move(values)](FlowCollector<T>* col, Continuation<void*>* c) -> void* {
        for (const auto& item : values) {
            col->emit(item, c);
        }
        return nullptr;
    });
}

// ----------------------------------------------------------------------------
// Transform tests
// ----------------------------------------------------------------------------

void test_filter() {
    auto f = make_test_flow<int>({1, 2, 3, 4, 5});
    auto filtered = filter<int>(f, [](const int& v) { return v % 2 != 0; });
    AccumulatorCollector<int> col;
    filtered->collect(&col, nullptr);
    assert((col.items == std::vector<int>{1, 3, 5}));

    // Suspending filter
    auto f2 = make_test_flow<int>({10, 20, 30});
    auto filtered2 = filter<int>(f2, [](const int& v, Continuation<void*>*) -> void* {
        return new bool(v > 15);
    });
    AccumulatorCollector<int> col2;
    filtered2->collect(&col2, nullptr);
    assert((col2.items == std::vector<int>{20, 30}));
    std::cout << "test_filter passed" << std::endl;
}

void test_filter_not() {
    auto f = make_test_flow<int>({1, 2, 3, 4, 5});
    auto filtered = filter_not<int>(f, [](const int& v) { return v % 2 != 0; });
    AccumulatorCollector<int> col;
    filtered->collect(&col, nullptr);
    assert((col.items == std::vector<int>{2, 4}));
    std::cout << "test_filter_not passed" << std::endl;
}

struct Base {
    virtual ~Base() = default;
};
struct DerivedA : public Base {
    int a = 1;
};
struct DerivedB : public Base {
    int b = 2;
};

void test_filter_is_instance() {
    std::vector<std::shared_ptr<Base>> list = {
        std::make_shared<DerivedA>(),
        std::make_shared<DerivedB>(),
        std::make_shared<DerivedA>()
    };
    auto f = make_test_flow<std::shared_ptr<Base>>(list);
    auto filtered = filter_is_instance<DerivedA, Base>(f);
    AccumulatorCollector<std::shared_ptr<DerivedA>> col;
    filtered->collect(&col, nullptr);
    assert(col.items.size() == 2);
    assert(col.items[0]->a == 1);
    assert(col.items[1]->a == 1);
    std::cout << "test_filter_is_instance passed" << std::endl;
}

void test_filter_not_null() {
    int v1 = 10, v2 = 20;
    std::vector<int*> ptrs = {&v1, nullptr, &v2, nullptr};
    auto f = make_test_flow<int*>(ptrs);
    auto filtered = filter_not_null<int>(f);
    AccumulatorCollector<int*> col;
    filtered->collect(&col, nullptr);
    assert(col.items.size() == 2);
    assert(*col.items[0] == 10);
    assert(*col.items[1] == 20);

    // Optional overload
    std::vector<std::optional<std::string>> opts = {"hello", std::nullopt, "world"};
    auto f_opt = make_test_flow<std::optional<std::string>>(opts);
    auto filtered_opt = filter_not_null<std::string>(f_opt);
    AccumulatorCollector<std::string> col_opt;
    filtered_opt->collect(&col_opt, nullptr);
    assert((col_opt.items == std::vector<std::string>{"hello", "world"}));
    std::cout << "test_filter_not_null passed" << std::endl;
}

void test_map() {
    auto f = make_test_flow<int>({1, 2, 3});
    auto mapped = map<int, std::string>(f, [](int v) { return std::to_string(v * 10); });
    AccumulatorCollector<std::string> col;
    mapped->collect(&col, nullptr);
    assert((col.items == std::vector<std::string>{"10", "20", "30"}));

    // Suspending map
    auto mapped_suspend = map<int, int>(f, [](int v, Continuation<void*>*) -> void* {
        return new int(v + 100);
    });
    AccumulatorCollector<int> col_suspend;
    mapped_suspend->collect(&col_suspend, nullptr);
    assert((col_suspend.items == std::vector<int>{101, 102, 103}));
    std::cout << "test_map passed" << std::endl;
}

void test_map_not_null() {
    auto f = make_test_flow<int>({1, 2, 3, 4});
    auto mapped = map_not_null<int, std::string>(f, [](int v) -> std::optional<std::string> {
        if (v % 2 == 0) return std::to_string(v);
        return std::nullopt;
    });
    AccumulatorCollector<std::string> col;
    mapped->collect(&col, nullptr);
    assert((col.items == std::vector<std::string>{"2", "4"}));
    std::cout << "test_map_not_null passed" << std::endl;
}

void test_with_index() {
    auto f = make_test_flow<std::string>({"a", "b", "c"});
    auto indexed = with_index<std::string>(f);
    AccumulatorCollector<IndexedValue<std::string>> col;
    indexed->collect(&col, nullptr);
    assert(col.items.size() == 3);
    assert(col.items[0].index == 0 && col.items[0].value == "a");
    assert(col.items[1].index == 1 && col.items[1].value == "b");
    assert(col.items[2].index == 2 && col.items[2].value == "c");
    std::cout << "test_with_index passed" << std::endl;
}

void test_on_each() {
    auto f = make_test_flow<int>({1, 2, 3});
    std::vector<int> side_effects;
    auto tracked = on_each<int>(f, [&side_effects](const int& v) {
        side_effects.push_back(v);
    });
    AccumulatorCollector<int> col;
    tracked->collect(&col, nullptr);
    assert((col.items == std::vector<int>{1, 2, 3}));
    assert((side_effects == std::vector<int>{1, 2, 3}));
    std::cout << "test_on_each passed" << std::endl;
}

void test_running_fold_and_scan() {
    auto f = make_test_flow<int>({1, 2, 3});
    auto folded = running_fold<int, int>(f, 0, [](int acc, int val) { return acc + val; });
    AccumulatorCollector<int> col;
    folded->collect(&col, nullptr);
    assert((col.items == std::vector<int>{0, 1, 3, 6}));

    // scan alias
    auto scanned = scan<int, int>(f, 10, [](int acc, int val) { return acc + val; });
    AccumulatorCollector<int> col2;
    scanned->collect(&col2, nullptr);
    assert((col2.items == std::vector<int>{10, 11, 13, 16}));
    std::cout << "test_running_fold_and_scan passed" << std::endl;
}

void test_running_reduce() {
    auto f = make_test_flow<int>({1, 2, 3, 4});
    auto reduced = running_reduce<int>(f, [](int acc, int val) { return acc * val; });
    AccumulatorCollector<int> col;
    reduced->collect(&col, nullptr);
    assert((col.items == std::vector<int>{1, 2, 6, 24}));
    std::cout << "test_running_reduce passed" << std::endl;
}

void test_chunked() {
    auto f = make_test_flow<int>({1, 2, 3, 4, 5});
    auto chunked_flow = chunked<int>(f, 2);
    AccumulatorCollector<std::vector<int>> col;
    chunked_flow->collect(&col, nullptr);
    assert(col.items.size() == 3);
    assert((col.items[0] == std::vector<int>{1, 2}));
    assert((col.items[1] == std::vector<int>{3, 4}));
    assert((col.items[2] == std::vector<int>{5}));

    bool threw = false;
    try {
        chunked<int>(f, 0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
    std::cout << "test_chunked passed" << std::endl;
}

// ----------------------------------------------------------------------------
// Errors tests
// ----------------------------------------------------------------------------

void test_catch() {
    auto failing = flow::flow<int>([](FlowCollector<int>* col, Continuation<void*>* c) -> void* {
        col->emit(1, c);
        col->emit(2, c);
        throw std::runtime_error("upstream failure");
    });

    auto caught = catch_<int>(failing, [](FlowCollector<int>* col, std::exception_ptr) {
        col->emit(999, nullptr);
    });

    AccumulatorCollector<int> col;
    caught->collect(&col, nullptr);
    assert((col.items == std::vector<int>{1, 2, 999}));

    // Downstream exception is NOT caught
    class ThrowingCollector : public FlowCollector<int> {
    public:
        void* emit(int, Continuation<void*>*) override {
            throw std::logic_error("downstream failure");
        }
    };
    ThrowingCollector throwing_col;
    bool downstream_threw = false;
    try {
        caught->collect(&throwing_col, nullptr);
    } catch (const std::logic_error& e) {
        downstream_threw = true;
    }
    assert(downstream_threw);
    std::cout << "test_catch passed" << std::endl;
}

void test_retry() {
    int attempts = 0;
    auto f = flow::flow<int>([&attempts](FlowCollector<int>* col, Continuation<void*>* c) -> void* {
        ++attempts;
        col->emit(1, c);
        if (attempts < 3) {
            throw std::runtime_error("temporary error");
        }
        col->emit(2, c);
        return nullptr;
    });

    auto retried = retry<int>(f, 3, [](std::exception_ptr) {
        return true;
    });

    AccumulatorCollector<int> col;
    retried->collect(&col, nullptr);
    assert((col.items == std::vector<int>{1, 1, 1, 2}));
    assert(attempts == 3);
    std::cout << "test_retry passed" << std::endl;
}

void test_retry_when() {
    int attempts = 0;
    auto f = flow::flow<int>([&attempts](FlowCollector<int>*, Continuation<void*>*) -> void* {
        ++attempts;
        throw std::runtime_error("attempt " + std::to_string(attempts));
    });

    std::vector<std::int64_t> recorded_attempts;
    auto retried = retry_when<int>(f, [&recorded_attempts](
        FlowCollector<int>*, std::exception_ptr, std::int64_t attempt) {
        recorded_attempts.push_back(attempt);
        return attempt < 2; // Retry twice (attempts 0, 1), then stop on 2
    });

    AccumulatorCollector<int> col;
    bool failed = false;
    try {
        retried->collect(&col, nullptr);
    } catch (const std::runtime_error&) {
        failed = true;
    }
    assert(failed);
    assert((recorded_attempts == std::vector<std::int64_t>{0, 1, 2}));
    std::cout << "test_retry_when passed" << std::endl;
}

int main() {
    test_filter();
    test_filter_not();
    test_filter_is_instance();
    test_filter_not_null();
    test_map();
    test_map_not_null();
    test_with_index();
    test_on_each();
    test_running_fold_and_scan();
    test_running_reduce();
    test_chunked();
    test_catch();
    test_retry();
    test_retry_when();

    std::cout << "\nAll Transform & Errors tests passed successfully!" << std::endl;
    return 0;
}
