// Original file: kotlinx-coroutines-debug/test/SanitizedProbesTest.kt
// TODO: Convert @file:Suppress annotation
// TODO: Convert imports to C++ includes
// TODO: Implement DebugTestBase base class
// TODO: Convert @Test annotation
// TODO: Implement DebugProbes API
// TODO: Implement verifyDump, verifyStackTrace
// TODO: Implement select builder

// @file:Suppress("PackageDirectoryMismatch")
namespace definitely {
namespace not {
namespace kotlinx {
namespace coroutines {

class SanitizedProbesTest : public DebugTestBase {
public:
    // @Before
    void set_up() override {
        DebugTestBase::set_up();
        // TODO: DebugProbes.sanitizeStackTraces = true
        // TODO: DebugProbes.enableCreationStackTraces = true
    }

    // @Test
    void test_recovered_stack_trace() {
        // TODO: runTest {
        //     auto deferred = createDeferred()
        //     auto traces = std::vector<std::string>{
        //         "java.util.concurrent.ExecutionException\n" +
        //                 "\tat definitely.not.kotlinx.coroutines.SanitizedProbesTest$createDeferredNested$1.invokeSuspend(SanitizedProbesTest.kt:97)\n" +
        //                 "\tat _COROUTINE._BOUNDARY._(CoroutineDebugging.kt)\n" +
        //                 "\tat definitely.not.kotlinx.coroutines.SanitizedProbesTest.oneMoreNestedMethod(SanitizedProbesTest.kt:67)\n" +
        //                 "\tat definitely.not.kotlinx.coroutines.SanitizedProbesTest.nestedMethod(SanitizedProbesTest.kt:61)\n" +
        //                 "\tat definitely.not.kotlinx.coroutines.SanitizedProbesTest$testRecoveredStackTrace$1.invokeSuspend(SanitizedProbesTest.kt:50)\n" +
        //                 "\tat _COROUTINE._CREATION._(CoroutineDebugging.kt)\n" +
        //                 "\tat kotlin.coroutines.intrinsics.IntrinsicsKt__IntrinsicsJvmKt.createCoroutineUnintercepted(IntrinsicsJvm.kt:116)\n" +
        //                 "\tat kotlinx.coroutines.intrinsics.CancellableKt.startCoroutineCancellable(Cancellable.kt:23)\n" +
        //                 "\tat kotlinx.coroutines.testing.TestBase.runTest$default(TestBase.kt:141)\n" +
        //                 "\tat definitely.not.kotlinx.coroutines.SanitizedProbesTest.testRecoveredStackTrace(SanitizedProbesTest.kt:33)",
        //         "Caused by: java.util.concurrent.ExecutionException\n" +
        //                 "\tat definitely.not.kotlinx.coroutines.SanitizedProbesTest$createDeferredNested$1.invokeSuspend(SanitizedProbesTest.kt:57)\n" +
        //                 "\tat kotlin.coroutines.jvm.internal.BaseContinuationImpl.resumeWith(ContinuationImpl.kt:32)\n"
        //     }
        //     nestedMethod(deferred, traces)
        //     deferred.join()
        // }
    }

    // @Test
    void test_coroutines_dump() {
        // TODO: Large verifyDump call with multiple coroutines
    }

    // @Test
    void test_select_builder() {
        // TODO: runTest {
        //     auto selector = launchSelector()
        //     expect(1)
        //     yield()
        //     expect(3)
        //     verifyDump(...)
        //     finish(4)
        //     selector.cancelAndJoin()
        // }
    }

private:
    void* launch_selector(/* CoroutineScope* scope */) {
        // TODO: auto job = CompletableDeferred(Unit)
        // return launch {
        //     select<Int> {
        //         job.onJoin {
        //             expect(2)
        //             delay(Long.MAX_VALUE)
        //             1
        //         }
        //     }
        // }
        return nullptr;
    }

    void* create_active_deferred(/* CoroutineScope* scope */) {
        // TODO: return async {
        //     suspendingMethod()
        //     assertTrue(true)
        // }
        return nullptr;
    }

    // suspend
    void suspending_method() {
        // TODO: delay(Long.MAX_VALUE)
    }

    void* create_deferred(/* CoroutineScope* scope */) {
        // TODO: return createDeferredNested()
        return nullptr;
    }

    void* create_deferred_nested(/* CoroutineScope* scope */) {
        // TODO: return async(NonCancellable) {
        //     throw ExecutionException(null)
        // }
        return nullptr;
    }

    // suspend
    void nested_method(void* deferred, const std::vector<std::string>& traces) {
        // TODO: oneMoreNestedMethod(deferred, traces)
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
};

} // namespace coroutines
} // namespace kotlinx
} // namespace not
} // namespace definitely
