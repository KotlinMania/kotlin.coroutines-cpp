// Original file: kotlinx-coroutines-debug/test/EnhanceStackTraceWithTreadDumpAsJsonTest.kt
// TODO: Convert @file:Suppress annotation
// TODO: Convert imports to C++ includes
// TODO: Implement DebugTestBase base class
// TODO: Implement Gson JSON parsing
// TODO: Implement DebugProbesImpl API
// TODO: Convert data class to C++ struct

// @file:Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
namespace kotlinx {
namespace coroutines {
namespace debug {

class EnhanceStackTraceWithTreadDumpAsJsonTest : public DebugTestBase {
private:
    struct StackTraceElementInfoFromJson {
        std::string declaring_class;
        std::string method_name;
        std::string* file_name;  // String?
        int line_number;
    };

public:
    // @Test
    void test_enhanced_stack_trace_format_with_deferred() {
        // TODO: runTest {
        //     auto deferred = async {
        //         suspendingMethod()
        //         assertTrue(true)
        //     }
        //     yield()
        //
        //     auto coroutineInfo = DebugProbesImpl.dumpCoroutinesInfo()
        //     assertEquals(coroutineInfo.size, 2)
        //     auto info = coroutineInfo[1]
        //     auto enhancedStackTraceAsJsonString = DebugProbesImpl.enhanceStackTraceWithThreadDumpAsJson(info)
        //     auto enhancedStackTraceFromJson = Gson().fromJson(enhancedStackTraceAsJsonString, Array<StackTraceElementInfoFromJson>::class.java)
        //     auto enhancedStackTrace = DebugProbesImpl.enhanceStackTraceWithThreadDump(info, info.lastObservedStackTrace)
        //     assertEquals(enhancedStackTrace.size, enhancedStackTraceFromJson.size)
        //     for ((frame, frameFromJson) in enhancedStackTrace.zip(enhancedStackTraceFromJson)) {
        //         assertEquals(frame.className, frameFromJson.declaringClass)
        //         assertEquals(frame.methodName, frameFromJson.methodName)
        //         assertEquals(frame.fileName, frameFromJson.fileName)
        //         assertEquals(frame.lineNumber, frameFromJson.lineNumber)
        //     }
        //
        //     deferred.cancelAndJoin()
        // }
    }

private:
    // suspend
    void suspending_method() {
        // TODO: delay(Long.MAX_VALUE)
    }
};

} // namespace debug
} // namespace coroutines
} // namespace kotlinx
