/**
 * Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/LastTest.kt
 */

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Reduce.hpp"

#include <iostream>
#include <optional>

namespace kotlinx {
namespace coroutines {

using namespace flow;
using namespace testing;

class LastTest : public TestBase {
public:
    // @Test
    void test_last() {
        run_test([](CoroutineScope*) {
            auto f = flow_of({1, 2, 3});
            void* res1 = last(f, nullptr);
            assert_not_null(res1);
            assert_equals(3, *static_cast<int*>(res1));
            delete static_cast<int*>(res1);

            void* res2 = last_or_null(f, nullptr);
            assert_not_null(res2);
            assert_equals(3, *static_cast<int*>(res2));
            delete static_cast<int*>(res2);
        });
    }

    // @Test
    void test_nulls() {
        run_test([](CoroutineScope*) {
            auto f = flow_of<std::optional<int>>({1, std::nullopt});
            void* res1 = last(f, nullptr);
            assert_not_null(res1);
            auto val1 = *static_cast<std::optional<int>*>(res1);
            delete static_cast<std::optional<int>*>(res1);
            assert_true(!val1.has_value());

            void* res2 = last_or_null(f, nullptr);
            assert_not_null(res2);
            auto val2 = *static_cast<std::optional<int>*>(res2);
            delete static_cast<std::optional<int>*>(res2);
            assert_true(!val2.has_value());
        });
    }

    // @Test
    void test_nulls_last_or_null() {
        run_test([](CoroutineScope*) {
            auto f = flow_of<std::optional<int>>({std::nullopt, 1});
            void* res = last_or_null(f, nullptr);
            assert_not_null(res);
            auto val = *static_cast<std::optional<int>*>(res);
            delete static_cast<std::optional<int>*>(res);
            assert_true(val.has_value());
            assert_equals(1, val.value());
        });
    }

    // @Test
    void test_empty_flow() {
        run_test([](CoroutineScope*) {
            assert_fails_with<NoSuchElementException>([]() {
                last(empty_flow<int>(), nullptr);
            });

            void* result = last_or_null(empty_flow<int>(), nullptr);
            assert_null(result);
        });
    }

    // @Test
    void test_bad_class() {
        run_test([](CoroutineScope*) {
            BadClass instance;
            auto f = flow_of<BadClass>({instance});
            void* res1 = last(f, nullptr);
            assert_not_null(res1);
            delete static_cast<BadClass*>(res1);

            void* res2 = last_or_null(f, nullptr);
            assert_not_null(res2);
            delete static_cast<BadClass*>(res2);
        });
    }
};

} // namespace coroutines
} // namespace kotlinx

int main() {
    using namespace kotlinx::coroutines;
    LastTest test;
    int failed = 0;

    auto run = [&](const char* name, void (LastTest::*method)()) {
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

    std::cout << "=== LastTest ===" << std::endl;
    run("test_last", &LastTest::test_last);
    run("test_nulls", &LastTest::test_nulls);
    run("test_nulls_last_or_null", &LastTest::test_nulls_last_or_null);
    run("test_empty_flow", &LastTest::test_empty_flow);
    run("test_bad_class", &LastTest::test_bad_class);

    std::cout << "=== Results: " << (5 - failed) << "/5 passed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}