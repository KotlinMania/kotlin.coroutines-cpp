/**
 * Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/SingleTest.kt
 */

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Reduce.hpp"

#include <iostream>
#include <optional>
#include <stdexcept>

namespace kotlinx {
namespace coroutines {

using namespace flow;
using namespace testing;

class SingleTest : public TestBase {
public:
    // @Test
    void test_single() {
        run_test([](CoroutineScope*) {
            auto f = flow::flow<long long>([](FlowCollector<long long>* collector, Continuation<void*>* cont) -> void* {
                collector->emit(239LL, cont);
                return nullptr;
            });

            void* res1 = single(f, nullptr);
            assert_not_null(res1);
            assert_equals(239LL, *static_cast<long long*>(res1));
            delete static_cast<long long*>(res1);

            void* res2 = single_or_null(f, nullptr);
            assert_not_null(res2);
            assert_equals(239LL, *static_cast<long long*>(res2));
            delete static_cast<long long*>(res2);
        });
    }

    // @Test
    void test_multiple_values() {
        run_test([](CoroutineScope*) {
            auto f = flow::flow<long long>([](FlowCollector<long long>* collector, Continuation<void*>* cont) -> void* {
                collector->emit(239LL, cont);
                collector->emit(240LL, cont);
                return nullptr;
            });

            assert_fails_with<std::invalid_argument>([&]() {
                single(f, nullptr);
            });

            void* res = single_or_null(f, nullptr);
            assert_null(res);
        });
    }

    // @Test
    void test_no_values() {
        run_test([](CoroutineScope*) {
            auto f = flow::flow<int>([](FlowCollector<int>*, Continuation<void*>*) -> void* {
                return nullptr;
            });

            assert_fails_with<NoSuchElementException>([&]() {
                single(f, nullptr);
            });

            void* res = single_or_null(f, nullptr);
            assert_null(res);
        });
    }

    // @Test
    void test_exception() {
        run_test([](CoroutineScope*) {
            auto f = flow::flow<int>([](FlowCollector<int>*, Continuation<void*>*) -> void* {
                throw TestException();
            });

            assert_fails_with<TestException>([&]() {
                single(f, nullptr);
            });
            assert_fails_with<TestException>([&]() {
                single_or_null(f, nullptr);
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
                single(f, nullptr);
            });
            assert_fails_with<TestException>([&]() {
                single_or_null(f, nullptr);
            });
        });
    }

    // @Test
    void test_nullable_single() {
        run_test([](CoroutineScope*) {
            auto f1 = flow_of<std::optional<int>>({1});
            void* res1 = single(f1, nullptr);
            assert_not_null(res1);
            auto val1 = *static_cast<std::optional<int>*>(res1);
            delete static_cast<std::optional<int>*>(res1);
            assert_true(val1.has_value());
            assert_equals(1, val1.value());

            auto f_null = flow_of<std::optional<int>>({std::nullopt});
            void* res_null = single(f_null, nullptr);
            assert_not_null(res_null);
            auto val_null = *static_cast<std::optional<int>*>(res_null);
            delete static_cast<std::optional<int>*>(res_null);
            assert_true(!val_null.has_value());

            auto f_empty = flow_of<std::optional<int>>({});
            assert_fails_with<NoSuchElementException>([&]() {
                single(f_empty, nullptr);
            });

            void* res2 = single_or_null(f1, nullptr);
            assert_not_null(res2);
            auto val2 = *static_cast<std::optional<int>*>(res2);
            delete static_cast<std::optional<int>*>(res2);
            assert_true(val2.has_value());
            assert_equals(1, val2.value());

            void* res2_null = single_or_null(f_null, nullptr);
            assert_not_null(res2_null);
            auto val2_null = *static_cast<std::optional<int>*>(res2_null);
            delete static_cast<std::optional<int>*>(res2_null);
            assert_true(!val2_null.has_value());

            void* res2_empty = single_or_null(f_empty, nullptr);
            assert_null(res2_empty);
        });
    }

    // @Test
    void test_bad_class() {
        run_test([](CoroutineScope*) {
            BadClass instance;
            auto f = flow_of<BadClass>({instance});
            void* res1 = single(f, nullptr);
            assert_not_null(res1);
            delete static_cast<BadClass*>(res1);

            void* res2 = single_or_null(f, nullptr);
            assert_not_null(res2);
            delete static_cast<BadClass*>(res2);

            auto f2 = flow::flow<BadClass>([](FlowCollector<BadClass>* collector, Continuation<void*>* cont) -> void* {
                collector->emit(BadClass(), cont);
                collector->emit(BadClass(), cont);
                return nullptr;
            });
            assert_fails_with<std::invalid_argument>([&]() {
                single(f2, nullptr);
            });
        });
    }

    // @Test
    void test_single_no_wait() {
        run_test([](CoroutineScope*) {
            auto f = flow::flow<int>([](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
                collector->emit(1, cont);
                collector->emit(2, cont);
                return nullptr;
            });

            void* res = single_or_null(f, nullptr);
            assert_null(res);
        });
    }
};

} // namespace coroutines
} // namespace kotlinx

int main() {
    using namespace kotlinx::coroutines;
    SingleTest test;
    int failed = 0;

    auto run = [&](const char* name, void (SingleTest::*method)()) {
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

    std::cout << "=== SingleTest ===" << std::endl;
    run("test_single", &SingleTest::test_single);
    run("test_multiple_values", &SingleTest::test_multiple_values);
    run("test_no_values", &SingleTest::test_no_values);
    run("test_exception", &SingleTest::test_exception);
    run("test_exception_after_value", &SingleTest::test_exception_after_value);
    run("test_nullable_single", &SingleTest::test_nullable_single);
    run("test_bad_class", &SingleTest::test_bad_class);
    run("test_single_no_wait", &SingleTest::test_single_no_wait);

    std::cout << "=== Results: " << (8 - failed) << "/8 passed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}