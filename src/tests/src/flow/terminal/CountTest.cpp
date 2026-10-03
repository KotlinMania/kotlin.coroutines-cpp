/**
 * Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/CountTest.kt
 */

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Count.hpp"

#include <iostream>

namespace kotlinx {
namespace coroutines {

using namespace flow;
using namespace testing;

namespace {

int unbox_count(void* ptr) {
    if (ptr == nullptr || intrinsics::is_coroutine_suspended(ptr)) {
        throw std::logic_error("count() returned null or suspended unexpectedly");
    }
    int val = *static_cast<int*>(ptr);
    delete static_cast<int*>(ptr);
    return val;
}

} // namespace

class CountTest : public TestBase {
public:
    // @Test
    void test_count() {
        run_test([](CoroutineScope*) {
            auto f = flow_of({239, 240});
            assert_equals(2, unbox_count(count(f)));
            assert_equals(2, unbox_count(count(f, [](const int&) { return true; })));
            assert_equals(1, unbox_count(count(f, [](const int& it) { return it % 2 == 0; })));
            assert_equals(0, unbox_count(count(f, [](const int&) { return false; })));
        });
    }

    // @Test
    void test_no_values() {
        run_test([](CoroutineScope*) {
            assert_equals(0, unbox_count(count(flow_of<int>({}))));
            assert_equals(0, unbox_count(count(flow_of<int>({}), [](const int&) { return false; })));
            assert_equals(0, unbox_count(count(flow_of<int>({}), [](const int&) { return true; })));
        });
    }

    // @Test
    void test_exception() {
        run_test([](CoroutineScope*) {
            auto f = flow::flow<int>([](FlowCollector<int>*, Continuation<void*>*) -> void* {
                throw TestException();
            });

            assert_fails_with<TestException>([&]() {
                unbox_count(count(f));
            });
            assert_fails_with<TestException>([&]() {
                unbox_count(count(f, [](const int&) { return false; }));
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
                unbox_count(count(f));
            });
            assert_fails_with<TestException>([&]() {
                unbox_count(count(f, [](const int&) { return false; }));
            });
        });
    }
};

} // namespace coroutines
} // namespace kotlinx

int main() {
    using namespace kotlinx::coroutines;
    CountTest test;
    int failed = 0;

    auto run = [&](const char* name, void (CountTest::*method)()) {
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

    std::cout << "=== CountTest ===" << std::endl;
    run("test_count", &CountTest::test_count);
    run("test_no_values", &CountTest::test_no_values);
    run("test_exception", &CountTest::test_exception);
    run("test_exception_after_value", &CountTest::test_exception_after_value);

    std::cout << "=== Results: " << (4 - failed) << "/4 passed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}