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
SharingStarted* SharingStarted::Eagerly() { return nullptr; }
SharingStarted* SharingStarted::Lazily() { return nullptr; }
SharingStarted* SharingStarted::WhileSubscribed(long long stop_timeout_millis, long long replay_expiration_millis) { return nullptr; }

} // namespace flow
} // namespace coroutines
} // namespace kotlinx