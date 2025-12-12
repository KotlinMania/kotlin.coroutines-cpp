/**
 * @file BroadcastChannel.cpp
 * @brief Implementation of BroadcastChannel.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/channels/BroadcastChannel.hpp`.
 */

#include "kotlinx/coroutines/channels/BroadcastChannel.hpp"

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            // Template implementation is in the header.

            // Explicit instantiation for common types to ensure compilation validity and linkage.
            template class BroadcastChannel<int>;
            template class BroadcastChannel<std::string>;
            // Add other types as needed by the application.
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx