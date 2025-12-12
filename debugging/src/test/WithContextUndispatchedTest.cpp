// Original file: kotlinx-coroutines-debug/test/WithContextUndispatchedTest.kt
// TODO: Convert imports to C++ includes
// TODO: Implement DebugTestBase base class
// TODO: Convert @Test annotation
// TODO: Implement flow APIs (flowOf, flow, zip, flowOn, collect)
// TODO: Implement FlowCollector
// TODO: Implement verifyPartialDump

namespace kotlinx {
namespace coroutines {
namespace debug {

// Test for our internal optimization "withContextUndispatched"
class WithContextUndispatchedTest : public DebugTestBase {
public:
    // @Test
    void test_zip() {
        // TODO: runTest {
        //     auto f1 = flowOf("a")
        //     auto f2 = flow {
        //         nestedEmit()
        //         yield()
        //     }
        //     f1.zip(f2) { i, j -> i + j }.collect {
        //         bar(false)
        //     }
        // }
    }

private:
    // suspend
    void nested_emit(/* FlowCollector<Int>* collector */) {
        // TODO: emit(1)
        // emit(2)
    }

public:
    // @Test
    void test_undispatched_flow_on() {
        // TODO: runTest {
        //     auto flow = flowOf(1, 2, 3).flowOn(CoroutineName("..."))
        //     flow.collect {
        //         bar(true)
        //     }
        // }
    }

    // @Test
    void test_undispatched_flow_on_with_nested_caller() {
        // TODO: runTest {
        //     auto flow = flow {
        //         nestedEmit()
        //     }.flowOn(CoroutineName("..."))
        //     flow.collect {
        //         bar(true)
        //     }
        // }
    }

private:
    // suspend
    void bar(bool for_flow_on) {
        // TODO: yield()
        // if (forFlowOn) {
        //     verifyFlowOn()
        // } else {
        //     verifyZip()
        // }
        // yield()
    }

    // suspend
    void verify_flow_on() {
        // TODO: yield() // suspend
        // verifyPartialDump(1, "verifyFlowOn", "bar")
    }

    // suspend
    void verify_zip() {
        // TODO: yield() // suspend
        // verifyPartialDump(2, "verifyZip", "bar", "nestedEmit")
    }
};

} // namespace debug
} // namespace coroutines
} // namespace kotlinx
