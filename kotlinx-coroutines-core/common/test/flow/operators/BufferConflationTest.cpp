// Original file: kotlinx-coroutines-core/common/test/flow/operators/BufferConflationTest.kt
//
// TODO: Mechanical C++ transliteration - Requires comprehensive updates:
// - Import test framework headers
// - Implement suspend functions as regular functions
// - Map Flow operators to C++ equivalents
// - Implement BufferOverflow enum
// - Map Channel.CONFLATED constant
// - Handle function parameters with default values
// - Implement range operations (until, toList)

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.channels.*
// TODO: import kotlin.test.*

/**
 * A _behavioral_ test for conflation options that can be configured by the [buffer] operator to test that it is
 * implemented properly and that adjacent [buffer] calls are fused properly.
*/
class BufferConflationTest : public TestBase {
private:
    static constexpr int n = 100; // number of elements to emit for test

    void check_conflate(
        int capacity,
        BufferOverflow on_buffer_overflow, // TODO: = BufferOverflow.DROP_OLDEST,
        auto op // TODO: suspend Flow<Int>.() -> Flow<Int>
    ) {
        // TODO: runTest {
        expect(1);
        // emit all and conflate, then collect first & last
        auto expected_list = [&]() {
            if (on_buffer_overflow == BufferOverflow::kDropOldest) {
                // first item & capacity last ones
                std::vector<int> result;
                result.push_back(0);
                for (int i = n - capacity; i < n; ++i) {
                    result.push_back(i);
                }
                return result;
            } else if (on_buffer_overflow == BufferOverflow::kDropLatest) {
                // first & capacity following ones
                std::vector<int> result;
                for (int i = 0; i <= capacity; ++i) {
                    result.push_back(i);
                }
                return result;
            } else {
                // TODO: error("cannot happen")
                return std::vector<int>{};
            }
        }();

        flow([&](auto& emit) {
            for (int i = 0; i < n; ++i) {
                expect(i + 2);
                emit(i);
            }
        })
            .op()
            .collect([&](int i) {
                auto j = std::find(expected_list.begin(), expected_list.end(), i) - expected_list.begin();
                expect(n + 2 + j);
            });
        finish(n + 2 + expected_list.size());
        // TODO: }
    }

public:
    // TODO: @Test
    void testConflate() {
        check_conflate(1, BufferOverflow::kDropOldest, [](auto& flow) {
            return flow.conflate();
        });
    }

    // TODO: @Test
    void testBufferConflated() {
        check_conflate(1, BufferOverflow::kDropOldest, [](auto& flow) {
            return flow.buffer(Channel::kConflated);
        });
    }

    // TODO: @Test
    void testBufferDropOldest() {
        check_conflate(1, BufferOverflow::kDropOldest, [](auto& flow) {
            return flow.buffer(BufferOverflow::kDropOldest);
        });
    }

    // TODO: @Test
    void testBuffer0DropOldest() {
        check_conflate(1, BufferOverflow::kDropOldest, [](auto& flow) {
            return flow.buffer(0, BufferOverflow::kDropOldest);
        });
    }

    // TODO: @Test
    void testBuffer1DropOldest() {
        check_conflate(1, BufferOverflow::kDropOldest, [](auto& flow) {
            return flow.buffer(1, BufferOverflow::kDropOldest);
        });
    }

    // TODO: @Test
    void testBuffer10DropOldest() {
        check_conflate(10, BufferOverflow::kDropOldest, [](auto& flow) {
            return flow.buffer(10, BufferOverflow::kDropOldest);
        });
    }

    // TODO: @Test
    void testConflateOverridesBuffer() {
        check_conflate(1, BufferOverflow::kDropOldest, [](auto& flow) {
            return flow.buffer(42).conflate();
        });
    }

    // TODO: @Test
    // conflate().conflate() should work like a single conflate
    void testDoubleConflate() {
        check_conflate(1, BufferOverflow::kDropOldest, [](auto& flow) {
            return flow.conflate().conflate();
        });
    }

    // TODO: @Test
    void testConflateBuffer10Combine() {
        check_conflate(10, BufferOverflow::kDropOldest, [](auto& flow) {
            return flow.conflate().buffer(10);
        });
    }

    // TODO: @Test
    void testBufferDropLatest() {
        check_conflate(1, BufferOverflow::kDropLatest, [](auto& flow) {
            return flow.buffer(BufferOverflow::kDropLatest);
        });
    }

    // TODO: @Test
    void testBuffer0DropLatest() {
        check_conflate(1, BufferOverflow::kDropLatest, [](auto& flow) {
            return flow.buffer(0, BufferOverflow::kDropLatest);
        });
    }

    // TODO: @Test
    void testBuffer1DropLatest() {
        check_conflate(1, BufferOverflow::kDropLatest, [](auto& flow) {
            return flow.buffer(1, BufferOverflow::kDropLatest);
        });
    }

    // TODO: @Test
    // overrides previous buffer
    void testBufferDropLatestOverrideBuffer() {
        check_conflate(1, BufferOverflow::kDropLatest, [](auto& flow) {
            return flow.buffer(42).buffer(BufferOverflow::kDropLatest);
        });
    }

    // TODO: @Test
    // overrides previous conflate
    void testBufferDropLatestOverrideConflate() {
        check_conflate(1, BufferOverflow::kDropLatest, [](auto& flow) {
            return flow.conflate().buffer(BufferOverflow::kDropLatest);
        });
    }

    // TODO: @Test
    void testBufferDropLatestBuffer7Combine() {
        check_conflate(7, BufferOverflow::kDropLatest, [](auto& flow) {
            return flow.buffer(BufferOverflow::kDropLatest).buffer(7);
        });
    }

    // TODO: @Test
    void testConflateOverrideBufferDropLatest() {
        check_conflate(1, BufferOverflow::kDropOldest, [](auto& flow) {
            return flow.buffer(BufferOverflow::kDropLatest).conflate();
        });
    }

    // TODO: @Test
    void testBuffer3DropOldestOverrideBuffer8DropLatest() {
        check_conflate(3, BufferOverflow::kDropOldest, [](auto& flow) {
            return flow.buffer(8, BufferOverflow::kDropLatest)
                .buffer(3, BufferOverflow::kDropOldest);
        });
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
