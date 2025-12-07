// Original: kotlinx-coroutines-core/common/test/EmptyContext.kt
// TODO: Transliterated from Kotlin - needs C++ implementation
// TODO: Handle suspend functions and coroutine intrinsics
// TODO: Implement probeCoroutineCreated and probeCoroutineResumed

#include <functional>

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.internal.probeCoroutineCreated
// TODO: import kotlinx.coroutines.internal.probeCoroutineResumed
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.coroutines.intrinsics.*

template<typename T>
T with_empty_context(std::function<T()> block) {
    // TODO: suspend function implementation
    // TODO: suspendCoroutine { cont ->
    //     block.startCoroutineUnintercepted(Continuation(EmptyCoroutineContext) { cont.resumeWith(it) })
    // }
}

/**
 * Use this function to restart a coroutine directly from inside of suspendCoroutine,
 * when the code is already in the context of this coroutine.
 * It does not use ContinuationInterceptor and does not update the context of the current thread.
 */
template<typename T>
void start_coroutine_unintercepted(std::function<T()> func, Continuation<T>* completion) {
    // TODO: const auto actual_completion = probeCoroutineCreated(completion);
    // TODO: const auto value = try {
    //     probeCoroutineResumed(actual_completion);
    //     startCoroutineUninterceptedOrReturn(actual_completion);
    // } catch (const std::exception& e) {
    //     actual_completion->resume_with_exception(e);
    //     return;
    // }
    // TODO: if (value !== COROUTINE_SUSPENDED) {
    //     // @Suppress("UNCHECKED_CAST")
    //     actual_completion->resume(static_cast<T>(value));
    // }
}

} // namespace coroutines
} // namespace kotlinx
