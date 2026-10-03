/**
 * Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/ToCollectionTest.kt
 */

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Collection.hpp"

#include <iostream>
#include <vector>
#include <set>

namespace kotlinx {
namespace coroutines {

using namespace flow;
using namespace testing;

class ToCollectionTest : public TestBase {
private:
    std::shared_ptr<Flow<int>> make_test_flow() {
        return flow::flow<int>([](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
            for (int i = 0; i < 10; ++i) {
                collector->emit(42, cont);
            }
            return nullptr;
        });
    }

    std::shared_ptr<Flow<int>> make_empty_flow() {
        return flow_of<int>({});
    }

public:
    // @Test
    void test_to_list() {
        run_test([this](CoroutineScope*) {
            void* res1 = to_list(make_test_flow(), nullptr);
            assert_not_null(res1);
            auto* list1 = static_cast<std::vector<int>*>(res1);
            std::vector<int> expected1(10, 42);
            assert_equals(expected1, *list1);
            delete list1;

            void* res2 = to_list(make_empty_flow(), nullptr);
            assert_not_null(res2);
            auto* list2 = static_cast<std::vector<int>*>(res2);
            std::vector<int> expected2;
            assert_equals(expected2, *list2);
            delete list2;
        });
    }

    // @Test
    void test_to_set() {
        run_test([this](CoroutineScope*) {
            void* res1 = to_set(make_test_flow(), nullptr);
            assert_not_null(res1);
            auto* set1 = static_cast<std::set<int>*>(res1);
            std::set<int> expected1 = {42};
            assert_equals(expected1, *set1);
            delete set1;

            void* res2 = to_set(make_empty_flow(), nullptr);
            assert_not_null(res2);
            auto* set2 = static_cast<std::set<int>*>(res2);
            std::set<int> expected2;
            assert_equals(expected2, *set2);
            delete set2;
        });
    }
};

} // namespace coroutines
} // namespace kotlinx

int main() {
    using namespace kotlinx::coroutines;
    ToCollectionTest test;
    int failed = 0;

    auto run = [&](const char* name, void (ToCollectionTest::*method)()) {
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

    std::cout << "=== ToCollectionTest ===" << std::endl;
    run("test_to_list", &ToCollectionTest::test_to_list);
    run("test_to_set", &ToCollectionTest::test_to_set);

    std::cout << "=== Results: " << (2 - failed) << "/2 passed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}