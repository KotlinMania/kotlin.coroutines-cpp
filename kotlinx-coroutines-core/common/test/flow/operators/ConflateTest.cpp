// Original file: kotlinx-coroutines-core/common/test/flow/operators/ConflateTest.kt
//
// TODO: Mechanical C++ transliteration - Requires comprehensive updates:
// - Import test framework headers
// - Map withVirtualTime to C++ equivalent
// - Map delay() to C++ equivalent
// - Map conflate() operator

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlin.test.*

class ConflateTest : public TestBase {
public:
    // TODO: @Test
    // from example
    void testExample() {
        // TODO: withVirtualTime {
        expect(1);
        auto flow_var = flow([](auto& emit) {
            for (int i = 1; i <= 30; ++i) {
                delay(100);
                emit(i);
            }
        });
        auto result = flow_var.conflate().on_each([](auto it) {
            delay(1000);
        }).to_list();
        assertEquals(std::vector<int>{1, 10, 20, 30}, result);
        finish(2);
        // TODO: }
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
