// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/flow/terminal/CountTest.kt
// TODO(port): runTest wrapper not yet wired for suspend semantics

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Count.hpp"

namespace kotlinx {
namespace coroutines {

using namespace flow;
using namespace testing;

class CountTest : public TestBase {
public:
    // @Test
    void test_count() {
        // TODO(port): runTest { ... }
        auto f = flow_of({239, 240});
        assert(count(f) == 2);
        assert(count(f, [](int) { return true; }) == 2);
        assert(count(f, [](int it) { return it % 2 == 0; }) == 1);
        assert(count(f, [](int) { return false; }) == 0);
    }

    // @Test
    void test_no_values() {
        // TODO(port): runTest { ... }
        assert(count(empty_flow<int>()) == 0);
        assert(count(empty_flow<int>(), [](int) { return false; }) == 0);
        assert(count(empty_flow<int>(), [](int) { return true; }) == 0);
    }

    // @Test
    void test_exception() {
        // TODO(port): runTest { ... }
        auto f = flow::flow<int>([](FlowCollector<int>* /*collector*/, Continuation<void*>*) -> void* {
            throw TestException();
            return nullptr;
        });

        // assertFailsWith<TestException>
        bool caught1 = false;
        try {
            count(f);
        } catch (const TestException&) {
            caught1 = true;
        }
        assert(caught1);

        bool caught2 = false;
        try {
            count(f, [](int) { return false; });
        } catch (const TestException&) {
            caught2 = true;
        }
        assert(caught2);
    }

    // @Test
    void test_exception_after_value() {
        // TODO(port): runTest { ... }
        auto f = flow::flow<int>([](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
            collector->emit(1, cont);
            throw TestException();
            return nullptr;
        });

        // assertFailsWith<TestException>
        bool caught1 = false;
        try {
            count(f);
        } catch (const TestException&) {
            caught1 = true;
        }
        assert(caught1);

        bool caught2 = false;
        try {
            count(f, [](int) { return false; });
        } catch (const TestException&) {
            caught2 = true;
        }
        assert(caught2);
    }
};

} // namespace coroutines
} // namespace kotlinx
