#include "kotlinx/coroutines/flow/Zip.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/testing/TestBase.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::flow;
using namespace kotlinx::coroutines::testing;

template <typename T>
class AccumulatorCollector : public FlowCollector<T> {
public:
    std::vector<T> items;
    void* emit(T value, Continuation<void*>*) override {
        items.push_back(std::move(value));
        return nullptr;
    }
};

void test_combine_success() {
    auto f1 = flow::flow<int>([](FlowCollector<int>* col, Continuation<void*>* c) -> void* {
        col->emit(1, c);
        col->emit(2, c);
        return nullptr;
    });

    auto f2 = flow::flow<std::string>([](FlowCollector<std::string>* col, Continuation<void*>* c) -> void* {
        col->emit("a", c);
        col->emit("b", c);
        return nullptr;
    });

    auto combined = combine<int, std::string, std::string>(
        f1, f2,
        [](int n, const std::string& s) {
            return std::to_string(n) + s;
        }
    );

    AccumulatorCollector<std::string> collector;
    combined->collect(&collector, nullptr);
    assert_false(collector.items.empty());
    // Latest values should produce combinations
    assert_equals(std::string("2b"), collector.items.back());
    std::cout << "test_combine_success passed" << std::endl;
}

void test_combine_error_propagation() {
    auto f1 = flow::flow<int>([](FlowCollector<int>* col, Continuation<void*>* c) -> void* {
        col->emit(1, c);
        throw std::runtime_error("combine_f1_boom");
    });

    auto f2 = flow::flow<std::string>([](FlowCollector<std::string>* col, Continuation<void*>* c) -> void* {
        col->emit("a", c);
        return nullptr;
    });

    auto combined = combine<int, std::string, std::string>(
        f1, f2,
        [](int n, const std::string& s) {
            return std::to_string(n) + s;
        }
    );

    AccumulatorCollector<std::string> collector;
    bool caught = false;
    try {
        combined->collect(&collector, nullptr);
    } catch (const std::runtime_error& e) {
        if (std::string(e.what()) == "combine_f1_boom") {
            caught = true;
        }
    }
    assert_true(caught);
    std::cout << "test_combine_error_propagation passed" << std::endl;
}

void test_zip_success() {
    auto f1 = flow::flow<int>([](FlowCollector<int>* col, Continuation<void*>* c) -> void* {
        col->emit(1, c);
        col->emit(2, c);
        col->emit(3, c);
        return nullptr;
    });

    auto f2 = flow::flow<std::string>([](FlowCollector<std::string>* col, Continuation<void*>* c) -> void* {
        col->emit("a", c);
        col->emit("b", c);
        col->emit("c", c);
        col->emit("d", c);
        return nullptr;
    });

    auto zipped = zip<int, std::string, std::string>(
        f1, f2,
        [](int n, const std::string& s) {
            return std::to_string(n) + s;
        }
    );

    AccumulatorCollector<std::string> collector;
    zipped->collect(&collector, nullptr);
    assert_equals(static_cast<size_t>(3), collector.items.size());
    assert_equals(std::string("1a"), collector.items[0]);
    assert_equals(std::string("2b"), collector.items[1]);
    assert_equals(std::string("3c"), collector.items[2]);
    std::cout << "test_zip_success passed" << std::endl;
}

void test_zip_early_termination() {
    auto f1 = flow::flow<int>([](FlowCollector<int>* col, Continuation<void*>* c) -> void* {
        col->emit(10, c);
        col->emit(20, c);
        return nullptr;
    });

    auto f2 = flow::flow<int>([](FlowCollector<int>* col, Continuation<void*>* c) -> void* {
        for (int i = 1; i <= 100; ++i) {
            col->emit(i, c);
        }
        return nullptr;
    });

    auto zipped = zip<int, int, int>(
        f1, f2,
        [](int a, int b) { return a + b; }
    );

    AccumulatorCollector<int> collector;
    zipped->collect(&collector, nullptr);
    assert_equals(static_cast<size_t>(2), collector.items.size());
    assert_equals(11, collector.items[0]);
    assert_equals(22, collector.items[1]);
    std::cout << "test_zip_early_termination passed" << std::endl;
}

void test_zip_flow2_error_propagation() {
    auto f1 = flow::flow<int>([](FlowCollector<int>* col, Continuation<void*>* c) -> void* {
        col->emit(1, c);
        col->emit(2, c);
        return nullptr;
    });

    auto f2 = flow::flow<int>([](FlowCollector<int>*, Continuation<void*>*) -> void* {
        throw std::runtime_error("zip_flow2_boom");
    });

    auto zipped = zip<int, int, int>(
        f1, f2,
        [](int a, int b) { return a + b; }
    );

    AccumulatorCollector<int> collector;
    bool caught = false;
    try {
        zipped->collect(&collector, nullptr);
    } catch (const std::runtime_error& e) {
        if (std::string(e.what()) == "zip_flow2_boom") {
            caught = true;
        }
    }
    assert_true(caught);
    std::cout << "test_zip_flow2_error_propagation passed" << std::endl;
}

void test_zip_flow1_error_propagation() {
    auto f1 = flow::flow<int>([](FlowCollector<int>*, Continuation<void*>*) -> void* {
        throw std::runtime_error("zip_flow1_boom");
    });

    auto f2 = flow::flow<int>([](FlowCollector<int>* col, Continuation<void*>* c) -> void* {
        col->emit(1, c);
        return nullptr;
    });

    auto zipped = zip<int, int, int>(
        f1, f2,
        [](int a, int b) { return a + b; }
    );

    AccumulatorCollector<int> collector;
    bool caught = false;
    try {
        zipped->collect(&collector, nullptr);
    } catch (const std::runtime_error& e) {
        if (std::string(e.what()) == "zip_flow1_boom") {
            caught = true;
        }
    }
    assert_true(caught);
    std::cout << "test_zip_flow1_error_propagation passed" << std::endl;
}

int main() {
    test_combine_success();
    test_combine_error_propagation();
    test_zip_success();
    test_zip_early_termination();
    test_zip_flow2_error_propagation();
    test_zip_flow1_error_propagation();
    std::cout << "All combine and zip smoke tests passed successfully!" << std::endl;
    return 0;
}
