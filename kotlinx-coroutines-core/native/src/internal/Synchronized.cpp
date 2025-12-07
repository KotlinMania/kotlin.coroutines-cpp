#include <functional>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/internal/Synchronized.kt
//
// TODO: actual typealias for platform types
// TODO: actual inline function
// TODO: @InternalCoroutinesApi annotation
// TODO: kotlinx.atomicfu.locks API

namespace kotlinx {
namespace coroutines {
namespace {

// TODO: Remove imports, fully qualify or add includes:
// import kotlinx.cinterop.*
// import kotlinx.coroutines.*
// import kotlinx.atomicfu.locks.withLock as withLock2

/**
 * @suppress **This an API and should not be used from general code.**
 */
// TODO: @InternalCoroutinesApi
// TODO: actual typealias
using SynchronizedObject = kotlinx::atomicfu::locks::SynchronizedObject;

/**
 * @suppress **This an API and should not be used from general code.**
 */
// TODO: @InternalCoroutinesApi
// TODO: actual inline function
template<typename T>
inline T synchronized_impl(SynchronizedObject& lock, std::function<T()> block) {
    // TODO: lock.withLock2(block)
    return block();
}

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
