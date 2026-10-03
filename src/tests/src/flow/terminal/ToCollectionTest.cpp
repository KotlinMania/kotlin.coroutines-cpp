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
#include <stdexcept>

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

    class DeferredTestFlow : public Flow<int> {
    public:
        FlowCollector<int>* retained_collector = nullptr;
        Continuation<void*>* retained_completion = nullptr;

        void* collect(FlowCollector<int>* c, Continuation<void*>* cont) override {
            retained_collector = c;
            retained_completion = cont;
            return intrinsics::get_COROUTINE_SUSPENDED();
        }
    };

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

    // @Test
    void test_to_collection_custom_destination() {
        run_test([this](CoroutineScope*) {
            std::vector<int> custom_dest = {1, 2};
            void* res = to_collection(make_test_flow(), &custom_dest, nullptr);
            assert_equals(static_cast<void*>(&custom_dest), res);
            assert_equals(12, static_cast<int>(custom_dest.size()));
            assert_equals(1, custom_dest[0]);
            assert_equals(2, custom_dest[1]);
            assert_equals(42, custom_dest[2]);
            assert_equals(42, custom_dest[11]);
        });
    }

    // @Test
    void test_suspended_to_list() {
        run_test([](CoroutineScope*) {
            auto deferred_impl = std::make_shared<DeferredTestFlow>();
            std::shared_ptr<Flow<int>> deferred = deferred_impl;
            std::vector<int>* resumed_list = nullptr;
            bool resumed = false;

            auto cont = make_continuation<void*>(nullptr, [&resumed_list, &resumed](Result<void*> res) {
                resumed = true;
                if (res.is_success()) {
                    resumed_list = static_cast<std::vector<int>*>(res.get_or_throw());
                }
            });

            void* r = to_list(deferred, cont.get());
            assert_true(intrinsics::is_coroutine_suspended(r));
            assert_true(!resumed);

            assert_not_null(deferred_impl->retained_collector);
            assert_not_null(deferred_impl->retained_completion);

            deferred_impl->retained_collector->emit(10, nullptr);
            deferred_impl->retained_collector->emit(20, nullptr);
            deferred_impl->retained_collector->emit(30, nullptr);

            deferred_impl->retained_completion->resume_with(Result<void*>::success(nullptr));

            assert_true(resumed);
            assert_not_null(resumed_list);
            std::vector<int> expected = {10, 20, 30};
            assert_equals(expected, *resumed_list);
            delete resumed_list;
        });
    }

    // @Test
    void test_suspended_to_set() {
        run_test([](CoroutineScope*) {
            auto deferred_impl = std::make_shared<DeferredTestFlow>();
            std::shared_ptr<Flow<int>> deferred = deferred_impl;
            std::set<int>* resumed_set = nullptr;
            bool resumed = false;

            auto cont = make_continuation<void*>(nullptr, [&resumed_set, &resumed](Result<void*> res) {
                resumed = true;
                if (res.is_success()) {
                    resumed_set = static_cast<std::set<int>*>(res.get_or_throw());
                }
            });

            void* r = to_set(deferred, cont.get());
            assert_true(intrinsics::is_coroutine_suspended(r));
            assert_true(!resumed);

            assert_not_null(deferred_impl->retained_collector);
            assert_not_null(deferred_impl->retained_completion);

            deferred_impl->retained_collector->emit(10, nullptr);
            deferred_impl->retained_collector->emit(20, nullptr);
            deferred_impl->retained_collector->emit(10, nullptr);

            deferred_impl->retained_completion->resume_with(Result<void*>::success(nullptr));

            assert_true(resumed);
            assert_not_null(resumed_set);
            std::set<int> expected = {10, 20};
            assert_equals(expected, *resumed_set);
            delete resumed_set;
        });
    }

    // @Test
    void test_suspended_failure_cleanup() {
        run_test([](CoroutineScope*) {
            auto deferred_impl = std::make_shared<DeferredTestFlow>();
            std::shared_ptr<Flow<int>> deferred = deferred_impl;
            bool resumed = false;
            bool failed = false;

            auto cont = make_continuation<void*>(nullptr, [&resumed, &failed](Result<void*> res) {
                resumed = true;
                if (res.is_failure()) {
                    failed = true;
                }
            });

            void* r = to_list(deferred, cont.get());
            assert_true(intrinsics::is_coroutine_suspended(r));
            assert_true(!resumed);

            deferred_impl->retained_completion->resume_with(
                Result<void*>::failure(std::make_exception_ptr(std::runtime_error("flow error"))));

            assert_true(resumed);
            assert_true(failed);
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
    run("test_to_collection_custom_destination", &ToCollectionTest::test_to_collection_custom_destination);
    run("test_suspended_to_list", &ToCollectionTest::test_suspended_to_list);
    run("test_suspended_to_set", &ToCollectionTest::test_suspended_to_set);
    run("test_suspended_failure_cleanup", &ToCollectionTest::test_suspended_failure_cleanup);

    std::cout << "=== Results: " << (6 - failed) << "/6 passed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}