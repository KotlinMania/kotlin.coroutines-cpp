// Original file: kotlinx-coroutines-core/common/test/flow/operators/BufferTest.kt
//
// TODO: Mechanical C++ transliteration - Requires comprehensive updates:
// - Import test framework headers
// - Implement suspend functions as regular functions
// - Map Flow operators to C++ equivalents
// - Implement min function from <algorithm>
// - Map wrapperDispatcher() to C++ equivalent
// - Handle assertFailsWith exception testing

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.channels.*
// TODO: import kotlin.math.*
// TODO: import kotlin.test.*

/**
 * A _behavioral_ test for buffering that is introduced by the [buffer] operator to test that it is
 * implemented properly and that adjacent [buffer] calls are fused properly.
 */
class BufferTest : public TestBase {
private:
    static constexpr int n = 200; // number of elements to emit for test
    static constexpr int default_buffer_size = 64; // expected default buffer size (per docs)

    // Use capacity == -1 to check case of "no buffer"
    void check_buffer(int capacity, auto op) {
        // TODO: runTest {
        expect(1);
        /*
           Channels perform full rendezvous. Sender does not suspend when there is a suspended receiver and vice-versa.
           Thus, perceived batch size is +2 from capacity.
         */
        int batch_size = capacity + 2;
        flow([&](auto& emit) {
            for (int i = 0; i < n; ++i) {
                int batch_no = i / batch_size;
                int batch_idx = i % batch_size;
                expect(batch_no * batch_size * 2 + batch_idx + 2);
                emit(i);
            }
        })
            .op() // insert user-defined operator
            .collect([&](int i) {
                int batch_no = i / batch_size;
                int batch_idx = i % batch_size;
                // last batch might have smaller size
                int k = std::min((batch_no + 1) * batch_size, n) - batch_no * batch_size;
                expect(batch_no * batch_size * 2 + k + batch_idx + 2);
            });
        finish(2 * n + 2);
        // TODO: }
    }

public:
    // TODO: @Test
    // capacity == -1 to checkBuffer means "no buffer" -- emits / collects are sequentially ordered
    void testBaseline() {
        check_buffer(-1, [](auto& flow) { return flow; });
    }

    // TODO: @Test
    void testBufferDefault() {
        check_buffer(default_buffer_size, [](auto& flow) {
            return flow.buffer();
        });
    }

    // TODO: @Test
    void testBufferRendezvous() {
        check_buffer(0, [](auto& flow) {
            return flow.buffer(0);
        });
    }

    // TODO: @Test
    void testBuffer1() {
        check_buffer(1, [](auto& flow) {
            return flow.buffer(1);
        });
    }

    // TODO: @Test
    void testBuffer2() {
        check_buffer(2, [](auto& flow) {
            return flow.buffer(2);
        });
    }

    // TODO: @Test
    void testBuffer3() {
        check_buffer(3, [](auto& flow) {
            return flow.buffer(3);
        });
    }

    // TODO: @Test
    void testBuffer00Fused() {
        check_buffer(0, [](auto& flow) {
            return flow.buffer(0).buffer(0);
        });
    }

    // TODO: @Test
    void testBuffer01Fused() {
        check_buffer(1, [](auto& flow) {
            return flow.buffer(0).buffer(1);
        });
    }

    // TODO: @Test
    void testBuffer11Fused() {
        check_buffer(2, [](auto& flow) {
            return flow.buffer(1).buffer(1);
        });
    }

    // TODO: @Test
    void testBuffer111Fused() {
        check_buffer(3, [](auto& flow) {
            return flow.buffer(1).buffer(1).buffer(1);
        });
    }

    // TODO: @Test
    void testBuffer123Fused() {
        check_buffer(6, [](auto& flow) {
            return flow.buffer(1).buffer(2).buffer(3);
        });
    }

    // TODO: @Test
    // multiple calls to buffer() create one channel of default size
    void testBufferDefaultTwiceFused() {
        check_buffer(default_buffer_size, [](auto& flow) {
            return flow.buffer().buffer();
        });
    }

    // TODO: @Test
    // explicit buffer takes precedence of default buffer on fuse
    void testBufferDefaultBufferFused() {
        check_buffer(7, [](auto& flow) {
            return flow.buffer().buffer(7);
        });
    }

    // TODO: @Test
    // explicit buffer takes precedence of default buffer on fuse
    void testBufferBufferDefaultFused() {
        check_buffer(8, [](auto& flow) {
            return flow.buffer(8).buffer();
        });
    }

    // TODO: @Test
    // flowOn operator does not use buffer when dispatches does not change
    void testFlowOnNameNoBuffer() {
        check_buffer(-1, [](auto& flow) {
            return flow.flow_on(CoroutineName("Name"));
        });
    }

    // TODO: @Test
    // flowOn operator uses default buffer size when dispatcher changes
    void testFlowOnDispatcherBufferDefault() {
        check_buffer(default_buffer_size, [](auto& flow) {
            return flow.flow_on(wrapper_dispatcher());
        });
    }

    // TODO: @Test
    // flowOn(...).buffer(n) sets explicit buffer size to n
    void testFlowOnDispatcherBufferFused() {
        check_buffer(5, [](auto& flow) {
            return flow.flow_on(wrapper_dispatcher()).buffer(5);
        });
    }

    // TODO: @Test
    // buffer(n).flowOn(...) sets explicit buffer size to n
    void testBufferFlowOnDispatcherFused() {
        check_buffer(6, [](auto& flow) {
            return flow.buffer(6).flow_on(wrapper_dispatcher());
        });
    }

    // TODO: @Test
    // flowOn(...).buffer(n) sets explicit buffer size to n
    void testFlowOnNameBufferFused() {
        check_buffer(7, [](auto& flow) {
            return flow.flow_on(CoroutineName("Name")).buffer(7);
        });
    }

    // TODO: @Test
    // buffer(n).flowOn(...) sets explicit buffer size to n
    void testBufferFlowOnNameFused() {
        check_buffer(8, [](auto& flow) {
            return flow.buffer(8).flow_on(CoroutineName("Name"));
        });
    }

    // TODO: @Test
    // multiple flowOn/buffer all fused together
    void testBufferFlowOnMultipleFused() {
        check_buffer(12, [](auto& flow) {
            return flow.flow_on(wrapper_dispatcher()).buffer(3)
                .flow_on(CoroutineName("Name")).buffer(4)
                .flow_on(wrapper_dispatcher()).buffer(5);
        });
    }

    // TODO: @Test
    void testCancellation() {
        // TODO: runTest {
        auto result = flow([](auto& emit) {
            emit(1);
            emit(2);
            emit(3);
            expectUnreached();
            emit(4);
        }).buffer(0)
            .take(2)
            .to_list();
        assertEquals(std::vector<int>{1, 2}, result);
        // TODO: }
    }

    // TODO: @Test
    void testFailsOnIllegalArguments() {
        auto flow_var = emptyFlow<int>();
        assertFailsWith<IllegalArgumentException>([&]() { flow_var.buffer(-3); });
        assertFailsWith<IllegalArgumentException>([&]() { flow_var.buffer(INT_MIN); });
        assertFailsWith<IllegalArgumentException>([&]() {
            flow_var.buffer(Channel::kConflated, BufferOverflow::kDropLatest);
        });
        assertFailsWith<IllegalArgumentException>([&]() {
            flow_var.buffer(Channel::kConflated, BufferOverflow::kDropOldest);
        });
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
