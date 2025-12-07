#pragma once

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * A strategy for buffer overflow handling in channels and flows that
 * controls what is going to be sacrificed on buffer overflow.
 */
enum class BufferOverflow {
    /**
     * Suspend until free space appears in the buffer.
     */
    SUSPEND,

    /**
     * Drop the oldest value in the buffer on overflow, add the new value to the buffer, do not suspend.
     */
    DROP_OLDEST,

    /**
     * Leave the buffer unchanged on overflow, dropping the value that we were going to add, do not suspend.
     */
    DROP_LATEST
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
