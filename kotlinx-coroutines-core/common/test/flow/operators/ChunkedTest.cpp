// Original file: kotlinx-coroutines-core/common/test/flow/operators/ChunkedTest.kt
//
// TODO: Mechanical C++ transliteration - Requires comprehensive updates:
// - Import test framework headers
// - Implement @OptIn(ExperimentalCoroutinesApi::class) equivalent
// - Map Flow operators to C++ equivalents
// - Map chunked() operator to C++ equivalent
// - Map joinToString() to C++ equivalent

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.channels.*
// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.test.*

// TODO: @OptIn(ExperimentalCoroutinesApi::class)
class ChunkedTest : public TestBase {
public:

    // TODO: @Test
    void testChunked() {
        // TODO: runTest {
        do_test(flow_of(1, 2, 3, 4, 5), 2, {{1, 2}, {3, 4}, {5}});
        do_test(flow_of(1, 2, 3, 4, 5), 3, {{1, 2, 3}, {4, 5}});
        do_test(flow_of(1, 2, 3, 4), 2, {{1, 2}, {3, 4}});
        do_test(flow_of(1), 3, {{1}});
        // TODO: }
    }

private:
    template<typename T>
    void do_test(Flow<T> flow_var, int chunk_size, std::vector<std::vector<T>> expected) {
        assertEquals(expected, flow_var.chunked(chunk_size).to_list());
        assertEquals(flow_var.to_list().chunked(chunk_size), flow_var.chunked(chunk_size).to_list());
    }

public:
    // TODO: @Test
    void testEmpty() {
        // TODO: runTest {
        do_test(empty_flow<int>(), 1, {});
        do_test(empty_flow<int>(), 2, {});
        // TODO: }
    }

    // TODO: @Test
    void testChunkedCancelled() {
        // TODO: runTest {
        auto result = flow([](auto& emit) {
            expect(1);
            emit(1);
            emit(2);
            expect(2);
        }).chunked(1).buffer().take(1).to_list();
        assertEquals(std::vector<std::vector<int>>{{1}}, result);
        finish(3);
        // TODO: }
    }

    // TODO: @Test
    void testChunkedCancelledWithSuspension() {
        // TODO: runTest {
        auto result = flow([](auto& emit) {
            expect(1);
            emit(1);
            yield();
            expectUnreached();
            emit(2);
        }).chunked(1).buffer().take(1).to_list();
        assertEquals(std::vector<std::vector<int>>{{1}}, result);
        finish(2);
        // TODO: }
    }

    // TODO: @Test
    void testChunkedDoesNotIgnoreCancellation() {
        // TODO: runTest {
        expect(1);
        auto result = flow([](auto& emit) {
            coroutine_scope([&]() {
                launch([&]() {
                    hang([&]() { expect(2); });
                });
                yield();
                emit(1);
                emit(2);
            });
        }).chunked(1).take(1).to_list();
        assertEquals(std::vector<std::vector<int>>{{1}}, result);
        finish(3);
        // TODO: }
    }

    // TODO: @Test
    void testIae() {
        assertFailsWith<IllegalArgumentException>([&]() { empty_flow<int>().chunked(-1); });
        assertFailsWith<IllegalArgumentException>([&]() { empty_flow<int>().chunked(0); });
        assertFailsWith<IllegalArgumentException>([&]() { empty_flow<int>().chunked(INT_MIN); });
        assertFailsWith<IllegalArgumentException>([&]() { empty_flow<int>().chunked(INT_MIN + 1); });
    }

    // TODO: @Test
    void testSample() {
        // TODO: runTest {
        auto result = flow_of("a", "b", "c", "d", "e")
            .chunked(2)
            .map([](auto it) { return join_to_string(it, ""); })
            .to_list();
        assertEquals(std::vector<std::string>{"ab", "cd", "e"}, result);
        // TODO: }
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
