#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/internal/ProbesSupport.kt
//
// TODO: actual keyword - platform-specific implementation
// TODO: @Suppress annotation
// TODO: inline functions
// TODO: Continuation interface

namespace kotlinx {
namespace coroutines {
namespace {

// TODO: Remove imports, fully qualify or add includes:
// import kotlin.coroutines.*

// TODO: @Suppress("NOTHING_TO_INLINE")
// TODO: actual inline function
template<typename T>
inline Continuation<T>* probe_coroutine_created(Continuation<T>* completion) {
    return completion;
}

// TODO: @Suppress("NOTHING_TO_INLINE")
// TODO: actual inline function
template<typename T>
inline void probe_coroutine_resumed(Continuation<T>* completion) {
    // Empty implementation
}

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
