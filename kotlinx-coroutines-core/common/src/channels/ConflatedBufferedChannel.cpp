/**
 * @file ConflatedBufferedChannel.cpp
 * @brief Implementation of ConflatedBufferedChannel.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/channels/ConflatedBufferedChannel.hpp`.
 */

#include "../../../../include/kotlinx/coroutines/channels/ConflatedBufferedChannel.hpp"

namespace kotlinx {
namespace coroutines {
namespace channels {

// Template implementation is in the header.

// Explicit instantiation for common types to ensure compilation validity and linkage.
template class ConflatedBufferedChannel<int>;
template class ConflatedBufferedChannel<std::string>;

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
