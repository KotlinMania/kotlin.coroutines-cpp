// Original file: kotlinx-coroutines-debug/test/StartModeProbesTest.kt
// TODO: Convert imports to C++ includes
// TODO: Implement DebugTestBase base class
// TODO: Convert @Test annotation
// TODO: Implement CoroutineStart modes
// TODO: Implement verifyPartialDump
// TODO: Implement @Suppress("DEPRECATION_ERROR")

namespace kotlinx {
namespace coroutines {
namespace debug {

class StartModeProbesTest : public DebugTestBase {
public:
    // @Test
    void test_undispatched() {
        // TODO: runTest {
        //     expect(1)
        //     auto job = launch(start = CoroutineStart.UNDISPATCHED) {
        //         expect(2)
        //         undispatchedSleeping()
        //         assertTrue(true)
        //     }
        //
        //     yield()
        //     expect(3)
        //     verifyPartialDump(2, "StartModeProbesTest.undispatchedSleeping")
        //     job.cancelAndJoin()
        //     verifyPartialDump(1, "StartModeProbesTest$testUndispatched")
        //     finish(4)
        // }
    }

private:
    // suspend
    void undispatched_sleeping() {
        // TODO: delay(Long.MAX_VALUE)
        // assertTrue(true)
    }

public:
    // @Test
    void test_with_timeout_with_undispatched() {
        // TODO: runTest {
        //     expect(1)
        //     auto job = launchUndispatched()
        //
        //     yield()
        //     expect(3)
        //     verifyPartialDump(
        //         2,
        //         "StartModeProbesTest$launchUndispatched$1.invokeSuspend",
        //         "StartModeProbesTest.withTimeoutHelper",
        //         "StartModeProbesTest$withTimeoutHelper$2.invokeSuspend"
        //     )
        //     job.cancelAndJoin()
        //     verifyPartialDump(1, "StartModeProbesTest$testWithTimeoutWithUndispatched")
        //     finish(4)
        // }
    }

private:
    void* launch_undispatched(/* CoroutineScope* scope */) {
        // TODO: return launch(start = CoroutineStart.UNDISPATCHED) {
        //     withTimeoutHelper()
        //     assertTrue(true)
        // }
        return nullptr;
    }

    // suspend
    void with_timeout_helper() {
        // TODO: withTimeout(Long.MAX_VALUE) {
        //     expect(2)
        //     delay(Long.MAX_VALUE)
        // }
        //
        // assertTrue(true)
    }

public:
    // @Test
    void test_with_timeout() {
        // TODO: runTest {
        //     withTimeout(Long.MAX_VALUE) {
        //         testActiveDump(
        //             false,
        //             "StartModeProbesTest$testWithTimeout$1.invokeSuspend",
        //             "state: RUNNING"
        //         )
        //     }
        // }
    }

    // @Test
    void test_with_timeout_after_yield() {
        // TODO: runTest {
        //     withTimeout(Long.MAX_VALUE) {
        //         testActiveDump(
        //             true,
        //             "StartModeProbesTest$testWithTimeoutAfterYield$1.invokeSuspend",
        //             "StartModeProbesTest$testWithTimeoutAfterYield$1$1.invokeSuspend",
        //             "StartModeProbesTest.testActiveDump",
        //             "state: RUNNING"
        //         )
        //     }
        // }
    }

private:
    // suspend
    void test_active_dump(bool should_yield, const std::vector<std::string>& expected_frames) {
        // TODO: if (shouldYield) yield()
        // verifyPartialDump(1, *expectedFrames)
        // assertTrue(true)
    }

public:
    // @Test
    void test_with_tail_call() {
        // TODO: runTest {
        //     expect(1)
        //     auto job = tailCallMethod()
        //     yield()
        //     expect(3)
        //     verifyPartialDump(2, "StartModeProbesTest$launchFromTailCall$2")
        //     job.cancelAndJoin()
        //     verifyPartialDump(1, "StartModeProbesTest$testWithTailCall")
        //     finish(4)
        // }
    }

private:
    // suspend - returns Job*
    void* tail_call_method(/* CoroutineScope* scope */) {
        // TODO: return launchFromTailCall()
        return nullptr;
    }

    // suspend - returns Job*
    void* launch_from_tail_call(/* CoroutineScope* scope */) {
        // TODO: return launch {
        //     expect(2)
        //     delay(Long.MAX_VALUE)
        // }
        return nullptr;
    }

public:
    // @Test
    void test_coroutine_scope() {
        // TODO: runTest {
        //     expect(1)
        //     auto job = launch(start = CoroutineStart.UNDISPATCHED) {
        //         runScope()
        //     }
        //
        //     yield()
        //     expect(3)
        //     verifyPartialDump(
        //         2,
        //         "StartModeProbesTest$runScope$2.invokeSuspend",
        //         "StartModeProbesTest$testCoroutineScope$1$job$1.invokeSuspend")
        //     job.cancelAndJoin()
        //     finish(4)
        // }
    }

private:
    // suspend
    void run_scope() {
        // TODO: coroutineScope {
        //     expect(2)
        //     delay(Long.MAX_VALUE)
        // }
    }

public:
    // @Test
    void test_lazy() {
        // TODO: runTest({ it is CancellationException }) {
        //     launch(start = CoroutineStart.LAZY) {  }
        //     actor<Int>(start = CoroutineStart.LAZY) {  }
        //     @Suppress("DEPRECATION_ERROR")
        //     broadcast<Int>(start = CoroutineStart.LAZY) {  }
        //     async(start = CoroutineStart.LAZY) { 1 }
        //     verifyPartialDump(5, "BlockingCoroutine",
        //         "LazyStandaloneCoroutine", "LazyActorCoroutine",
        //         "LazyBroadcastCoroutine", "LazyDeferredCoroutine")
        //     cancel()
        // }
    }
};

} // namespace debug
} // namespace coroutines
} // namespace kotlinx
