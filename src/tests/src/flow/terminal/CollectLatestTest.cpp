// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/flow/terminal/CollectLatestTest.kt
// NOTE(port): runTest wrapper not yet wired for suspend semantics
// NOTE(port): collectLatest operator requires coroutine infrastructure (launch + cancellation)

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"

namespace kotlinx {
namespace coroutines {

using namespace flow;
using namespace testing;

class CollectLatestTest : public TestBase {
public:
    // @Test
    void test_no_suspension() {
        // NOTE(port): runTest { ... }
        // NOTE(port): collectLatest requires coroutine launch + cancellation semantics
        // Original Kotlin:
        //   flowOf(1, 2, 3).collectLatest { expect(it) }
        //   finish(4)
    }

    // @Test
    void test_suspension() {
        // NOTE(port): runTest { ... }
        // NOTE(port): collectLatest + yield() requires full suspend semantics
        // Original Kotlin:
        //   flowOf(1, 2, 3).collectLatest { yield(); expect(1) }
        //   finish(2)
    }

    // @Test
    void test_upstream_error_suspension() {
        // NOTE(port): runTest(expected = { it is TestException }) { ... }
        // NOTE(port): collectLatest + exception propagation
        // Original Kotlin:
        //   try {
        //       flow { emit(1); throw TestException() }.collectLatest { expect(1) }
        //       expectUnreached()
        //   } finally {
        //       finish(2)
        //   }
    }

    // @Test
    void test_downstream_error() {
        // NOTE(port): runTest(expected = { it is TestException }) { ... }
        // NOTE(port): collectLatest + hang + downstream exception
        // Original Kotlin:
        //   try {
        //       flow { emit(1); hang { expect(1) } }.collectLatest { throw TestException() }
        //       expectUnreached()
        //   } finally {
        //       finish(2)
        //   }
    }
};

} // namespace coroutines
} // namespace kotlinx
