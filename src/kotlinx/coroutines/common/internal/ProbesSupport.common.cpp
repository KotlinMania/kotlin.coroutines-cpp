// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/ProbesSupport.common.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: expect inline functions need platform-specific implementations
// TODO: Continuation<T> needs C++ equivalent
// TODO: Coroutine probing/debugging support needs design

namespace kotlinx {
    namespace coroutines {
        namespace internal {
            // Forward declaration
            template<typename T>
            class Continuation;

            // TODO: expect inline function - needs platform-specific implementation
            template<typename T>
            inline Continuation<T> *probe_coroutine_created(Continuation<T> *completion) {
                return completion;
            }

            // TODO: expect inline function - needs platform-specific implementation
            template<typename T>
            inline void probe_coroutine_resumed(Continuation<T> *completion) {
                // No-op in common implementation
            }
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx