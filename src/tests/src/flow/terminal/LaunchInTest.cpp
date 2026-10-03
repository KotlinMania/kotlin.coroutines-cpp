/**
 * Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/LaunchInTest.kt
 */

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/testing/NamedDispatchers.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Collect.hpp"
#include "kotlinx/coroutines/flow/Transform.hpp"
#include "kotlinx/coroutines/flow/Emitters.hpp"
#include "kotlinx/coroutines/flow/Errors.hpp"
#include <iostream>

namespace kotlinx {
namespace coroutines {

using namespace flow;
using namespace testing;

class LaunchInTest : public TestBase {
public:
    // Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/LaunchInTest.kt:9-28
    // @Test
    void test_launch_in() {
        run_test([this](CoroutineScope* scope) {
            auto f1 = flow::flow<int>([this](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
                expect(1);
                collector->emit(1, cont);
                throw TestException();
            });

            auto f2 = on_each<int>(f1, [this](int it) {
                assert_equals(1, it);
                expect(2);
            });

            auto f3 = on_completion<int>(f2, [this](std::exception_ptr it) {
                assert_not_null(it);
                assert_is<TestException>(it);
                expect(3);
            });

            auto f4 = catch_<int>(f3, [this](std::exception_ptr it) {
                assert_not_null(it);
                assert_is<TestException>(it);
                expect(4);
            });

            auto job = launch_in<int>(f4, scope);
            job->join_blocking();
            finish(5);
        });
    }

    // Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/LaunchInTest.kt:30-38
    // @Test
    void test_dispatcher() {
        run_test([this](CoroutineScope* scope) {
            auto f = flow::flow<int>([this](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
                assert_equals(std::string("flow"), NamedDispatchers::name());
                collector->emit(1, cont);
                expect(1);
                return nullptr;
            });

            auto job = launch_in<int>(f, scope + NamedDispatchers("flow"));
            job->join_blocking();
            finish(2);
        });
    }

    // Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/LaunchInTest.kt:40-52
    // @Test
    void test_unhandled_error() {
        run_test([](std::exception_ptr e) -> bool {
            try {
                if (e) std::rethrow_exception(e);
            } catch (const TestException&) {
                return true;
            } catch (...) {}
            return false;
        }, [this](CoroutineScope* scope) {
            auto f1 = flow::flow<int>([this](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
                collector->emit(1, cont);
                expect(1);
                return nullptr;
            });

            auto f2 = catch_<int>(f1, [this](std::exception_ptr) {
                expect_unreached();
            });

            auto f3 = on_completion<int>(f2, [this](std::exception_ptr) {
                finish(2);
                throw TestException();
            });

            launch_in<int>(f3, scope);
        });
    }

    // Coverage extension: launch_in with CoroutineScope&
    void test_scope_reference() {
        run_test([this](CoroutineScope* scope) {
            auto f = flow::flow<int>([this](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
                collector->emit(42, cont);
                expect(1);
                return nullptr;
            });
            auto job = launch_in<int>(f, *scope);
            job->join_blocking();
            finish(2);
        });
    }

    // Coverage extension: launch_in with std::shared_ptr<CoroutineScope>
    void test_scope_shared_ptr() {
        run_test([this](CoroutineScope* scope) {
            auto f = flow::flow<int>([this](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
                collector->emit(42, cont);
                expect(1);
                return nullptr;
            });
            auto shared_scope = std::shared_ptr<CoroutineScope>(scope, [](CoroutineScope*) {});
            auto job = launch_in<int>(f, shared_scope);
            job->join_blocking();
            finish(2);
        });
    }
};

} // namespace coroutines
} // namespace kotlinx

int main() {
    using namespace kotlinx::coroutines;
    LaunchInTest test;
    int failed = 0;

    auto run = [&](const char* name, void (LaunchInTest::*method)()) {
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

    std::cout << "=== LaunchInTest ===" << std::endl;
    run("test_launch_in", &LaunchInTest::test_launch_in);
    run("test_dispatcher", &LaunchInTest::test_dispatcher);
    run("test_unhandled_error", &LaunchInTest::test_unhandled_error);
    run("test_scope_reference", &LaunchInTest::test_scope_reference);
    run("test_scope_shared_ptr", &LaunchInTest::test_scope_shared_ptr);

    std::cout << "=== Results: " << (5 - failed) << "/5 passed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
