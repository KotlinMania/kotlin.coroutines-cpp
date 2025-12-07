// Original file: kotlinx-coroutines-debug/test/LazyCoroutineTest.kt
// TODO: Convert imports to C++ includes
// TODO: Implement DebugTestBase base class
// TODO: Convert @Test annotation
// TODO: Implement CoroutineStart.LAZY
// TODO: Implement DebugProbes API
// TODO: Implement verifyPartialDump

namespace kotlinx {
namespace coroutines {
namespace debug {

class LazyCoroutineTest : public DebugTestBase {
public:
    // @Test
    void test_lazy_completed_coroutine() {
        // TODO: runTest {
        //     auto job = launch(start = CoroutineStart.LAZY) {}
        //     job.invokeOnCompletion { expect(2) }
        //     expect(1)
        //     job.cancelAndJoin()
        //     expect(3)
        //     assertEquals(1, DebugProbes.dumpCoroutinesInfo().size) // Outer runBlocking
        //     verifyPartialDump(1, "BlockingCoroutine{Active}")
        //     finish(4)
        // }
    }
};

} // namespace debug
} // namespace coroutines
} // namespace kotlinx
