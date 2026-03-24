// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/flow/terminal/LaunchInTest.kt
// TODO(port): runTest wrapper not yet wired for suspend semantics
// TODO(port): launchIn, onEach, onCompletion, catch operators need full implementation

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"

namespace kotlinx {
namespace coroutines {

using namespace flow;
using namespace testing;

class LaunchInTest : public TestBase {
public:
    // @Test
    void test_launch_in() {
        // TODO(port): runTest { ... }
        // TODO(port): Requires flow operator chain: onEach, onCompletion, catch, launchIn
        // Original Kotlin:
        //   val flow = flow {
        //       expect(1)
        //       emit(1)
        //       throw TestException()
        //   }.onEach {
        //       assertEquals(1, it)
        //       expect(2)
        //   }.onCompletion {
        //       assertIs<TestException>(it)
        //       expect(3)
        //   }.catch {
        //       assertTrue { it is TestException }
        //       expect(4)
        //   }
        //   flow.launchIn(this).join()
        //   finish(5)
    }

    // @Test
    void test_dispatcher() {
        // TODO(port): runTest { ... }
        // TODO(port): NamedDispatchers + launchIn with context
        // Original Kotlin:
        //   flow {
        //       assertEquals("flow", NamedDispatchers.name())
        //       emit(1)
        //       expect(1)
        //   }.launchIn(this + NamedDispatchers("flow")).join()
        //   finish(2)
    }

    // @Test
    void test_unhandled_error() {
        // TODO(port): runTest(expected = { it is TestException }) { ... }
        // TODO(port): Requires onCompletion operator
        // Original Kotlin:
        //   flow {
        //       emit(1)
        //       expect(1)
        //   }.catch {
        //       expectUnreached()
        //   }.onCompletion {
        //       finish(2)
        //       throw TestException()
        //   }.launchIn(this)
    }
};

} // namespace coroutines
} // namespace kotlinx
