// Original file: kotlinx-coroutines-debug/test/DebugProbesTest.kt
// TODO: Convert imports to C++ includes
// TODO: Implement DebugTestBase base class
// TODO: Convert @Test annotations to C++ test framework
// TODO: Implement suspend functions and coroutine builders
// TODO: Implement DebugProbes.withDebugProbes
// TODO: Implement verifyStackTrace function
// TODO: Handle ExecutionException and Deferred types
// TODO: Implement AtomicBoolean and threading primitives

namespace kotlinx {
namespace coroutines {
namespace debug {

class DebugProbesTest : public DebugTestBase {
private:
    // TODO: template<typename T> using Deferred = ...;

    // suspend - returns Deferred<void>*
    void* create_deferred(/* CoroutineScope* scope */) {
        // TODO: return async(NonCancellable) {
        //     throw ExecutionException(null)
        // }
        return nullptr;
    }

public:
    // @Test
    void test_async() {
        // TODO: runTest {
        //     auto deferred = createDeferred()
        //     auto traces = std::vector<std::string>{
        //         "java.util.concurrent.ExecutionException\n" +
        //             "\tat kotlinx.coroutines.debug.DebugProbesTest$createDeferred$1.invokeSuspend(DebugProbesTest.kt:14)\n" +
        //             "\tat _COROUTINE._BOUNDARY._(CoroutineDebugging.kt)\n" +
        //             "\tat kotlinx.coroutines.debug.DebugProbesTest.oneMoreNestedMethod(DebugProbesTest.kt:49)\n" +
        //             "\tat kotlinx.coroutines.debug.DebugProbesTest.nestedMethod(DebugProbesTest.kt:44)\n" +
        //             "\tat kotlinx.coroutines.debug.DebugProbesTest$testAsync$1.invokeSuspend(DebugProbesTest.kt:17)\n",
        //         "Caused by: java.util.concurrent.ExecutionException\n" +
        //             "\tat kotlinx.coroutines.debug.DebugProbesTest$createDeferred$1.invokeSuspend(DebugProbesTest.kt:14)\n" +
        //             "\tat kotlin.coroutines.jvm.internal.BaseContinuationImpl.resumeWith(ContinuationImpl.kt:32)"
        //     }
        //     nestedMethod(deferred, traces)
        //     deferred.join()
        // }
    }

    // @Test
    void test_async_with_probes() {
        // TODO: DebugProbes.withDebugProbes {
        //     DebugProbes.sanitizeStackTraces = false
        //     runTest {
        //         auto deferred = createDeferred()
        //         auto traces = ...
        //         nestedMethod(deferred, traces)
        //         deferred.join()
        //     }
        // }
    }

    // @Test
    void test_async_with_sanitized_probes() {
        // TODO: Similar to test_async_with_probes but with sanitized traces
    }

private:
    // suspend
    void nested_method(void* deferred, const std::vector<std::string>& traces) {
        // TODO: one_more_nested_method(deferred, traces)
        // assertTrue(true) // Prevent tail-call optimization
    }

    // suspend
    void one_more_nested_method(void* deferred, const std::vector<std::string>& traces) {
        // TODO: try {
        //     deferred.await()
        //     expectUnreached()
        // } catch (ExecutionException e) {
        //     verifyStackTrace(e, traces)
        // }
    }

public:
    // @Test
    void test_multiple_consecutive_probe_resumed() {
        // TODO: runTest {
        //     auto job = launch {
        //         expect(1)
        //         foo()
        //         expect(4)
        //         delay(Long.MAX_VALUE)
        //         expectUnreached()
        //     }
        //     yield()
        //     yield()
        //     expect(5)
        //     auto infos = DebugProbes.dumpCoroutinesInfo()
        //     assertEquals(2, infos.size)
        //     assertEquals(setOf(State.RUNNING, State.SUSPENDED), infos.map { it.state }.toSet())
        //     job.cancel()
        //     finish(6)
        // }
    }

    // @Test
    void test_multiple_consecutive_probe_resumed_and_later_running() {
        // TODO: runTest {
        //     auto reachedActiveStage = AtomicBoolean(false)
        //     auto job = launch(Dispatchers.Default) {
        //         expect(1)
        //         foo()
        //         expect(4)
        //         yield()
        //         reachedActiveStage.set(true)
        //         while (isActive) {
        //             // Spin until test is done
        //         }
        //     }
        //     while (!reachedActiveStage.get()) {
        //         delay(10)
        //     }
        //     expect(5)
        //     auto infos = DebugProbes.dumpCoroutinesInfo()
        //     assertEquals(2, infos.size)
        //     assertEquals(setOf(State.RUNNING, State.RUNNING), infos.map { it.state }.toSet())
        //     job.cancel()
        //     finish(6)
        // }
    }

private:
    // suspend
    void foo() {
        // TODO: bar()
        // Kill TCO
        // expect(3)
    }

    // suspend
    void bar() {
        // TODO: yield()
        // expect(2)
    }
};

} // namespace debug
} // namespace coroutines
} // namespace kotlinx
