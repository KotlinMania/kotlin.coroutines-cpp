// Original file: kotlinx-coroutines-core/common/test/flow/operators/FlatMapLatestTest.kt
//
// TODO: Mechanical C++ transliteration - Requires comprehensive updates:
// - Import test framework headers
// - Map flatMapLatest operator
// - Handle template type parameters

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlin.test.*

class FlatMapLatestTest : public TestBase {
public:

    // TODO: @Test
    void testFlatMapLatest() {
        // TODO: runTest {
        auto flow_var = flow_of(1, 2, 3).flat_map_latest([](int value) {
            return flow_of(value, value + 1);
        });
        assertEquals(std::vector<int>{1, 2, 2, 3, 3, 4}, flow_var.to_list());
        // TODO: }
    }

    // TODO: @Test
    void testEmission() {
        // TODO: runTest {
        auto list = flow([](auto& emit) {
            for (int i = 0; i < 5; ++i) {
                emit(i);
            }
        }).flat_map_latest([](auto it) { return flow_of(it); }).to_list();
        assertEquals(std::vector<int>{0, 1, 2, 3, 4}, list);
        // TODO: }
    }

    // TODO: @Test
    void testSwitchIntuitiveBehaviour() {
        // TODO: runTest {
        auto flow_var = flow_of(1, 2, 3, 4, 5);
        flow_var.flat_map_latest([](auto it) {
            return flow([it](auto& emit) {
                expect(it);
                emit(it);
                yield(); // Explicit cancellation check
                if (it != 5) expectUnreached();
                else expect(6);
            });
        }).collect([](auto) {});
        finish(7);
        // TODO: }
    }

    // TODO: @Test
    void testSwitchRendevouzBuffer() {
        // TODO: runTest {
        auto flow_var = flow_of(1, 2, 3, 4, 5);
        flow_var.flat_map_latest([](auto it) {
            return flow([it](auto& emit) {
                emit(it);
                // Reach here every uneven element because of channel's unfairness
                expect(it);
            });
        }).buffer(0).on_each([](auto it) { expect(it + 1); })
            .collect([](auto) {});
        finish(7);
        // TODO: }
    }

    // TODO: @Test
    void testHangFlows() {
        // TODO: runTest {
        auto flow_var = as_flow(std::vector<int>{1, 2, 3, 4});
        auto result = flow_var.flat_map_latest([](int value) {
            return flow([value](auto& emit) {
                if (value != 4) hang([&]() { expect(value); });
                emit(42);
            });
        }).to_list();

        assertEquals(std::vector<int>{42}, result);
        finish(4);
        // TODO: }
    }

    // TODO: @Test
    void testEmptyFlow() {
        // TODO: runTest {
        assertNull(empty_flow<int>().flat_map_latest([](auto) { return flow_of(1); }).single_or_null());
        // TODO: }
    }

    // TODO: @Test
    void testFailureInTransform() {
        // TODO: runTest {
        auto flow_var = flow_of(1, 2).flat_map_latest([](int value) {
            return flow([value](auto& emit) {
                if (value == 1) {
                    emit(1);
                    hang([&]() { expect(1); });
                } else {
                    expect(2);
                    throw TestException();
                }
            });
        });
        assertFailsWith<TestException>(flow_var);
        finish(3);
        // TODO: }
    }

    // TODO: @Test
    void testFailureDownstream() {
        // TODO: runTest {
        auto flow_var = flow_of(1).flat_map_latest([](int value) {
            return flow([value](auto& emit) {
                expect(1);
                emit(value);
                expect(2);
                hang([&]() { expect(4); });
            });
        }).flow_on(NamedDispatchers("downstream")).on_each([](auto it) {
            expect(3);
            throw TestException();
        });
        assertFailsWith<TestException>(flow_var);
        finish(5);
        // TODO: }
    }

    // TODO: @Test
    void testFailureUpstream() {
        // TODO: runTest {
        auto flow_var = flow([](auto& emit) {
            expect(1);
            emit(1);
            yield();
            expect(3);
            throw TestException();
        }).flat_map_latest<int, long>([](auto it) {
            return flow([it](auto& emit) {
                expect(2);
                hang([&]() {
                    expect(4);
                });
            });
        });
        assertFailsWith<TestException>(flow_var);
        finish(5);
        // TODO: }
    }

    // TODO: @Test
    void testTake() {
        // TODO: runTest {
        auto flow_var = flow_of(1, 2, 3, 4, 5).flat_map_latest([](auto it) { return flow_of(it); });
        assertEquals(std::vector<int>{1}, flow_var.take(1).to_list());
        // TODO: }
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
