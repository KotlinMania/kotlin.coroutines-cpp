/**
 * @file BufferedChannel.cpp
 * @brief Implementation of BufferedChannel.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/channels/BufferedChannel.hpp`.
 */

#include "kotlinx/coroutines/channels/BufferedChannel.hpp"

namespace kotlinx {
namespace coroutines {
namespace channels {

// Template implementation is in the header.

// Explicit instantiation for common types to ensure compilation validity and linkage.
template class BufferedChannel<int>;
template class BufferedChannel<std::string>;

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
