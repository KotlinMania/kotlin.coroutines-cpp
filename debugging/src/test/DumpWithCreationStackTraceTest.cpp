// Original file: kotlinx-coroutines-debug/test/DumpWithCreationStackTraceTest.kt
// TODO: Convert imports to C++ includes
// TODO: Implement DebugTestBase base class
// TODO: Convert @Before/@Test annotations
// TODO: Implement DebugProbes API
// TODO: Implement verifyDump function

namespace kotlinx {
namespace coroutines {
namespace debug {

class DumpWithCreationStackTraceTest : public DebugTestBase {
public:
    // @Before
    void set_up() override {
        DebugTestBase::set_up();
        // TODO: DebugProbes.enableCreationStackTraces = true
    }

    // @Test
    void test_coroutines_dump() {
        // TODO: runTest {
        //     auto deferred = createActiveDeferred()
        //     yield()
        //     verifyDump(
        //         "Coroutine \"coroutine#1\":BlockingCoroutine{Active}@70d1cb56, state: RUNNING\n" +
        //             "\tat java.lang.Thread.getStackTrace(Thread.java)\n" +
        //             "\tat kotlinx.coroutines.debug.internal.DebugProbesImpl.enhanceStackTraceWithThreadDumpImpl(DebugProbesImpl.kt)\n" +
        //             "\tat kotlinx.coroutines.debug.internal.DebugProbesImpl.dumpCoroutinesSynchronized(DebugProbesImpl.kt)\n" +
        //             "\tat kotlinx.coroutines.debug.internal.DebugProbesImpl.dumpCoroutines(DebugProbesImpl.kt)\n" +
        //             "\tat kotlinx.coroutines.debug.DebugProbes.dumpCoroutines(DebugProbes.kt:182)\n" +
        //             "\tat kotlinx.coroutines.debug.StacktraceUtilsKt.verifyDump(StacktraceUtils.kt)\n" +
        //             "\tat kotlinx.coroutines.debug.StacktraceUtilsKt.verifyDump$default(StacktraceUtils.kt)\n" +
        //             "\tat kotlinx.coroutines.debug.DumpWithCreationStackTraceTest$testCoroutinesDump$1.invokeSuspend(DumpWithCreationStackTraceTest.kt)\n" +
        //             "\tat _COROUTINE._CREATION._(CoroutineDebugging.kt)\n" +
        //             "\tat kotlin.coroutines.intrinsics.IntrinsicsKt__IntrinsicsJvmKt.createCoroutineUnintercepted(IntrinsicsJvm.kt)",
        //
        //         "Coroutine \"coroutine#2\":DeferredCoroutine{Active}@383fa309, state: SUSPENDED\n" +
        //             "\tat kotlinx.coroutines.debug.DumpWithCreationStackTraceTest$createActiveDeferred$1.invokeSuspend(DumpWithCreationStackTraceTest.kt)"
        //     )
        //     deferred.cancelAndJoin()
        // }
    }

private:
    // TODO: Deferred<void>* create_active_deferred(CoroutineScope* scope)
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
};

} // namespace debug
} // namespace coroutines
} // namespace kotlinx
