// Original file: kotlinx-coroutines-debug/test/DumpCoroutineInfoAsJsonAndReferencesTest.kt
// TODO: Convert @file:Suppress annotation
// TODO: Convert imports to C++ includes
// TODO: Implement @ExperimentalStdlibApi
// TODO: Implement DebugTestBase base class
// TODO: Implement DebugProbesImpl API
// TODO: Implement Gson JSON parsing
// TODO: Convert data class to C++ struct
// TODO: Implement coroutine context APIs

// @file:Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
namespace kotlinx {
namespace coroutines {
namespace debug {

// @ExperimentalStdlibApi
class DumpCoroutineInfoAsJsonAndReferencesTest : public DebugTestBase {
private:
    struct CoroutineInfoFromJson {
        std::string* name;  // String?
        long* id;           // Long?
        std::string* dispatcher;  // String?
        long* sequence_number;    // Long?
        std::string* state;       // String?
    };

public:
    // @Test
    void test_dump_of_unnamed_coroutine() {
        // TODO: run_test_with_named_deferred(name = nullptr)
    }

    // @Test
    void test_dump_of_named_coroutine() {
        // TODO: run_test_with_named_deferred("Name")
    }

    // @Test
    void test_dump_of_named_coroutine_with_special_characters() {
        // TODO: run_test_with_named_deferred("Name with\n \"special\" characters\\/\t\b")
    }

    // @Test
    void test_dump_with_no_coroutines() {
        // TODO: auto dumpResult = DebugProbesImpl.dumpCoroutinesInfoAsJsonAndReferences()
        // TODO: assertEquals(dumpResult.size, 4)
        // TODO: assertIsEmptyArray(dumpResult[1])
        // TODO: assertIsEmptyArray(dumpResult[2])
        // TODO: assertIsEmptyArray(dumpResult[3])
    }

private:
    void assert_is_empty_array(void* obj) {
        // TODO: assertTrue(obj is Array<*> && obj.isEmpty())
    }

    void run_test_with_named_deferred(const char* name) {
        // TODO: runTest {
        //     auto context = (name == nullptr) ? EmptyCoroutineContext : CoroutineName(name)
        //     auto deferred = async(context) {
        //         suspendingMethod()
        //         assertTrue(true)
        //     }
        //     yield()
        //     verifyDump()
        //     deferred.cancelAndJoin()
        // }
    }

    // suspend
    void suspending_method() {
        // TODO: delay(Long.MAX_VALUE)
    }

    void verify_dump() {
        // TODO: auto dumpResult = DebugProbesImpl.dumpCoroutinesInfoAsJsonAndReferences()
        // TODO: Parse JSON and verify all fields match
    }
};

} // namespace debug
} // namespace coroutines
} // namespace kotlinx
