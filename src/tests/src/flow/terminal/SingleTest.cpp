// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/flow/terminal/SingleTest.kt
// TODO(port): runTest wrapper not yet wired for suspend semantics
// TODO(port): Nullable flow elements require void* boxing

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Reduce.hpp"

namespace kotlinx {
namespace coroutines {

using namespace flow;
using namespace testing;

class SingleTest : public TestBase {
public:
    // @Test
    void test_single() {
        // TODO(port): runTest { ... }
        auto f = flow::flow<long long>([](FlowCollector<long long>* collector, Continuation<void*>* cont) -> void* {
            collector->emit(239LL, cont);
            return nullptr;
        });

        void* result = single(f, nullptr);
        assert(result != nullptr);
        assert(*static_cast<long long*>(result) == 239LL);
        delete static_cast<long long*>(result);

        void* result2 = single_or_null(f, nullptr);
        assert(result2 != nullptr);
        assert(*static_cast<long long*>(result2) == 239LL);
        delete static_cast<long long*>(result2);
    }

    // @Test
    void test_multiple_values() {
        // TODO(port): runTest { ... }
        auto f = flow::flow<long long>([](FlowCollector<long long>* collector, Continuation<void*>* cont) -> void* {
            collector->emit(239LL, cont);
            collector->emit(240LL, cont);
            return nullptr;
        });

        // assertFailsWith<IllegalArgumentException>
        bool caught = false;
        try {
            single(f, nullptr);
        } catch (const std::invalid_argument&) {
            caught = true;
        }
        assert(caught);

        void* result = single_or_null(f, nullptr);
        assert(result == nullptr);
    }

    // @Test
    void test_no_values() {
        // TODO(port): runTest { ... }
        // assertFailsWith<NoSuchElementException>
        bool caught = false;
        try {
            single(empty_flow<int>(), nullptr);
        } catch (const std::out_of_range&) {
            caught = true;
        }
        assert(caught);

        void* result = single_or_null(empty_flow<int>(), nullptr);
        assert(result == nullptr);
    }

    // @Test
    void test_exception() {
        // TODO(port): runTest { ... }
        auto f = flow::flow<int>([](FlowCollector<int>* /*collector*/, Continuation<void*>*) -> void* {
            throw TestException();
            return nullptr;
        });

        bool caught1 = false;
        try {
            single(f, nullptr);
        } catch (const TestException&) {
            caught1 = true;
        }
        assert(caught1);

        bool caught2 = false;
        try {
            single_or_null(f, nullptr);
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

        bool caught1 = false;
        try {
            single(f, nullptr);
        } catch (const TestException&) {
            caught1 = true;
        }
        assert(caught1);

        bool caught2 = false;
        try {
            single_or_null(f, nullptr);
        } catch (const TestException&) {
            caught2 = true;
        }
        assert(caught2);
    }

    // @Test
    // TODO(port): Nullable types — flowOf<Int?>(1).single() requires void* boxing
    // void test_nullable_single() { ... }

    // @Test
    // TODO(port): BadClass test requires special equality semantics
    // void test_bad_class() { ... }

    // @Test
    // TODO(port): awaitCancellation not yet implemented
    // void test_single_no_wait() { ... }
};

} // namespace coroutines
} // namespace kotlinx
