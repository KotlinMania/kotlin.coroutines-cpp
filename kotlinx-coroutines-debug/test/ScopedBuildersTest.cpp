// Original file: kotlinx-coroutines-debug/test/ScopedBuildersTest.kt
// TODO: Convert imports to C++ includes
// TODO: Implement DebugTestBase base class
// TODO: Convert @Test annotation
// TODO: Implement verifyDump function
// TODO: Implement coroutineScope, withContext
// TODO: Implement wrapperDispatcher

namespace kotlinx {
namespace coroutines {
namespace debug {

class ScopedBuildersTest : public DebugTestBase {
public:
    // @Test
    void test_nested_scopes() {
        // TODO: runBlocking {
        //     auto job = launch { doInScope() }
        //     yield()
        //     yield()
        //     verifyDump(
        //         "Coroutine \"coroutine#1\":BlockingCoroutine{Active}@16612a51, state: RUNNING",
        //
        //         "Coroutine \"coroutine#2\":StandaloneCoroutine{Active}@6b53e23f, state: SUSPENDED\n" +
        //             "\tat kotlinx.coroutines.debug.ScopedBuildersTest$doWithContext$2.invokeSuspend(ScopedBuildersTest.kt:49)\n" +
        //             "\tat kotlinx.coroutines.debug.ScopedBuildersTest.doWithContext(ScopedBuildersTest.kt:47)\n" +
        //             "\tat kotlinx.coroutines.debug.ScopedBuildersTest$doInScope$2.invokeSuspend(ScopedBuildersTest.kt:41)\n" +
        //             "\tat kotlinx.coroutines.debug.ScopedBuildersTest$testNestedScopes$1$job$1.invokeSuspend(ScopedBuildersTest.kt:30)"
        //     )
        //     job.cancelAndJoin()
        //     finish(4)
        // }
    }

private:
    // suspend
    void do_in_scope() {
        // TODO: coroutineScope {
        //     expect(1)
        //     doWithContext()
        //     expectUnreached()
        // }
    }

    // suspend
    void do_with_context() {
        // TODO: expect(2)
        // withContext(wrapperDispatcher(coroutineContext)) {
        //     expect(3)
        //     delay(Long.MAX_VALUE)
        // }
        // expectUnreached()
    }
};

} // namespace debug
} // namespace coroutines
} // namespace kotlinx
