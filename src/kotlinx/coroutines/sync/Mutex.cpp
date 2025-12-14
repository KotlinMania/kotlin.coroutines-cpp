/**
 * @file Mutex.cpp
 * @brief Mutex factory function implementation.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/sync/Mutex.kt
 */

#include "kotlinx/coroutines/sync/Mutex.hpp"
#include "kotlinx/coroutines/sync/MutexImpl.hpp"

namespace kotlinx {
namespace coroutines {
namespace sync {

Mutex* create_mutex(bool locked) {
    return create_mutex_impl(locked);
}

std::shared_ptr<Mutex> make_mutex(bool locked) {
    return make_mutex_impl(locked);
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
