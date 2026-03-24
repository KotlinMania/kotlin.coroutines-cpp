// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/flow/terminal/LastTest.kt
// TODO(port): runTest wrapper not yet wired for suspend semantics
// TODO(port): Nullable flow elements (flowOf(1, null)) require void* boxing

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Reduce.hpp"

namespace kotlinx {
namespace coroutines {

using namespace flow;
using namespace testing;

class LastTest : public TestBase {
public:
    // @Test
    void test_last() {
        // TODO(port): runTest { ... }
        auto f = flow_of({1, 2, 3});
        void* result = last(f, nullptr);
        assert(result != nullptr);
        assert(*static_cast<int*>(result) == 3);
        delete static_cast<int*>(result);

        void* result2 = last_or_null(f, nullptr);
        assert(result2 != nullptr);
        assert(*static_cast<int*>(result2) == 3);
        delete static_cast<int*>(result2);
    }

    // @Test
    // TODO(port): Nullable types — flowOf(1, null) requires void* boxing
    // void test_nulls() { ... }

    // @Test
    // TODO(port): Nullable types — flowOf(null, 1) requires void* boxing
    // void test_nulls_last_or_null() { ... }

    // @Test
    void test_empty_flow() {
        // TODO(port): runTest { ... }
        // assertFailsWith<NoSuchElementException>
        bool caught = false;
        try {
            last(empty_flow<int>(), nullptr);
        } catch (const std::out_of_range&) {
            caught = true;
        }
        assert(caught);

        void* result = last_or_null(empty_flow<int>(), nullptr);
        assert(result == nullptr);
    }

    // @Test
    // TODO(port): BadClass test requires special equality semantics
    // void test_bad_class() { ... }
};

} // namespace coroutines
} // namespace kotlinx
