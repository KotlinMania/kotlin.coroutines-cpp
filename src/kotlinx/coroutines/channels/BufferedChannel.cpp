/**
 * @file BufferedChannel.cpp
 * @brief Implementation of BufferedChannel.
 *
 * The BufferedChannel implementation is now entirely in the header file.
 * This file provides explicit template instantiations for common types.
 */

#include "../../../../include/kotlinx/coroutines/channels/BufferedChannel.hpp"

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            // Explicit instantiations for common types
            template class BufferedChannel<int>;
            template class BufferedChannel<std::string>;
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx