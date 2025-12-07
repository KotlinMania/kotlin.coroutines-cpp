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

// Placeholder implementation
std::shared_ptr<SharingStarted> SharingStarted::Eagerly() { return nullptr; }
std::shared_ptr<SharingStarted> SharingStarted::Lazily() { return nullptr; }
std::shared_ptr<SharingStarted> SharingStarted::WhileSubscribed(int64_t stopTimeoutMillis, int64_t replayExpirationMillis) { return nullptr; }

} // namespace flow
} // namespace coroutines
} // namespace kotlinx