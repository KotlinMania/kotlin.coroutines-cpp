#include "../core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/channels/BufferOverflow.kt
//
// TODO: Map Kotlin enum class to C++ enum class

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * A strategy for buffer overflow handling in channels and flows that
 * controls what is going to be sacrificed on buffer overflow:
 *
 * - SUSPEND — the upstream that is sending or emitting a value is **suspended** while the buffer is full.
 * - DROP_OLDEST — **the oldest** value in the buffer is dropped on overflow, and the new value is added,
 *   all without suspending.
 * - DROP_LATEST — the buffer remains unchanged on overflow, and the value that we were going to add
 *   gets discarded, all without suspending.
 */
enum class BufferOverflow {
    /**
     * Suspend until free space appears in the buffer.
     *
     * Use this to create backpressure, forcing the producers to slow down creation of new values in response to
     * consumers not being able to process the incoming values in time.
     * SUSPEND is a good choice when all elements must eventually be processed.
     */
    SUSPEND,

    /**
     * Drop **the oldest** value in the buffer on overflow, add the new value to the buffer, do not suspend.
     *
     * Use this in scenarios when only the last few values are important and skipping the processing of severely
     * outdated ones is desirable.
     */
    DROP_OLDEST,

    /**
     * Leave the buffer unchanged on overflow, dropping the value that we were going to add, do not suspend.
     *
     * This option can be used in rare advanced scenarios where all elements that are expected to enter the buffer are
     * equal, so it is not important which of them get thrown away.
     */
    DROP_LATEST
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
