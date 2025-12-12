// Original file: kotlinx-coroutines-debug/test/DebugLeaksTest.kt
// TODO: Convert imports to C++ includes
// TODO: Implement @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
// TODO: Implement DebugTestBase base class
// TODO: Convert @Test annotations to C++ test framework
// TODO: Implement GlobalScope, CoroutineStart, iterator, yield
// TODO: Implement FieldWalker for reachability testing
// TODO: Implement suspendCancellableCoroutine

/**
 * This is fast but fragile version of DebugLeaksStressTest that check reachability of a captured object
 * in DebugProbesImpl via FieldWalker.
 */
// @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
class DebugLeaksTest : public DebugTestBase {
private:
    class Captured {
    };

public:
    // @Test
    void test_iterator_leak() {
        // TODO: auto captured = Captured();
        // TODO: iterator { yield(captured) }
        // assert_no_captured_reference();
    }

    // @Test
    void test_lazy_global_coroutine_leak() {
        // TODO: auto captured = Captured();
        // TODO: GlobalScope.launch(start = CoroutineStart.LAZY) { println(captured) }
        // assert_no_captured_reference();
    }

    // @Test
    void test_lazy_cancelled_child_coroutine_leak() {
        // TODO: runTest {
        //     auto captured = Captured();
        //     coroutineScope {
        //         auto child = launch(start = CoroutineStart.LAZY) { println(captured) }
        //         child.cancel()
        //     }
        //     assert_no_captured_reference()
        // }
    }

    // @Test
    void test_abandoned_global_coroutine_leak() {
        // TODO: auto captured = Captured();
        // TODO: GlobalScope.launch {
        //     suspend_forever()
        //     println(captured)
        // }
        // assert_no_captured_reference();
    }

private:
    // suspend
    void suspend_forever() {
        // TODO: suspendCancellableCoroutine<Unit> {  }
    }

    void assert_no_captured_reference() {
        // TODO: FieldWalker.assertReachableCount(0, DebugProbesImpl, rootStatics = true) { it is Captured }
    }
};
