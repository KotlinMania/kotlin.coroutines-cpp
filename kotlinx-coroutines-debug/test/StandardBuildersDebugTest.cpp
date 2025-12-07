// Original file: kotlinx-coroutines-debug/test/StandardBuildersDebugTest.kt
// TODO: Convert imports to C++ includes
// TODO: Implement DebugTestBase base class
// TODO: Convert @Test annotation
// TODO: Implement DebugProbes API
// TODO: Implement sequence and iterator builders

namespace kotlinx {
namespace coroutines {
namespace debug {

class StandardBuildersDebugTest : public DebugTestBase {
public:
    // @Test
    void test_builders_are_missing_from_dump_by_default() {
        // TODO: runTest {
        //     auto [b1, b2] = createBuilders()
        //
        //     auto coroutines = DebugProbes.dumpCoroutinesInfo()
        //     assertEquals(1, coroutines.size)
        //     assertTrue { b1.hasNext() && b2.hasNext() } // Don't let GC collect our coroutines until the test is complete
        // }
    }

    // @Test
    void test_builders_can_be_enabled() {
        // TODO: runTest {
        //     try {
        //         DebugProbes.ignoreCoroutinesWithEmptyContext = false
        //         auto [b1, b2] = createBuilders()
        //         auto coroutines = DebugProbes.dumpCoroutinesInfo()
        //         assertEquals(3, coroutines.size)
        //         assertTrue { b1.hasNext() && b2.hasNext() } // Don't let GC collect our coroutines until the test is complete
        //     } finally {
        //         DebugProbes.ignoreCoroutinesWithEmptyContext = true
        //     }
        // }
    }

private:
    std::pair<void*, void*> create_builders() {
        // TODO: auto fromSequence = sequence {
        //     while (true) {
        //         yield(1)
        //     }
        // }.iterator()
        //
        // auto fromIterator = iterator {
        //     while (true) {
        //         yield(1)
        //     }
        // }
        // // Start coroutines
        // fromIterator.hasNext()
        // fromSequence.hasNext()
        // return {fromSequence, fromIterator}
        return {nullptr, nullptr};
    }
};

} // namespace debug
} // namespace coroutines
} // namespace kotlinx
