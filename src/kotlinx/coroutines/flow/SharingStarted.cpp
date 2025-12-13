/**
 * @file SharingStarted.cpp
 * @brief Implementation of SharingStarted.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/flow/SharingStarted.hpp`.
 */

#include "kotlinx/coroutines/flow/SharingStarted.hpp"

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            // TODO: Implement actual SharingStarted strategies
            SharingStarted *SharingStarted::eagerly() { return nullptr; }
            SharingStarted *SharingStarted::lazily() { return nullptr; }

            SharingStarted *SharingStarted::while_subscribed(long long stop_timeout_millis,
                                                            long long replay_expiration_millis) { return nullptr; }
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx
