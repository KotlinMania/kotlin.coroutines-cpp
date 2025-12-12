#pragma once
/**
 * @file CoroutineContextUtils.hpp
 *
 * Transliterated from:
 * - kotlinx-coroutines-core/common/src/CoroutineContext.common.kt (expect declarations)
 * - kotlinx-coroutines-core/native/src/CoroutineContext.kt (native actuals)
 *
 * On native, these helpers are no-ops that just run the block.
 */

#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include <functional>
#include <string>
#include <typeinfo>

namespace kotlinx {
namespace coroutines {

/**
 * Kotlin: internal actual inline fun <T> withCoroutineContext(...)
 * Native actual is a no-op wrapper.
 */
template<typename R>
inline R with_coroutine_context(
    const std::shared_ptr<CoroutineContext>& /*context*/,
    void* /*count_or_element*/,
    std::function<R()> block
) {
    return block();
}

/**
 * Kotlin: internal actual inline fun <T> withContinuationContext(...)
 * Native actual is a no-op wrapper.
 */
template<typename R, typename T>
inline R with_continuation_context(
    const std::shared_ptr<Continuation<T>>& /*continuation*/,
    void* /*count_or_element*/,
    std::function<R()> block
) {
    return block();
}

/**
 * Kotlin: internal actual fun Continuation<*>.toDebugString(): String = toString()
 * We don't have a common to_string() for continuations, so fall back to RTTI name.
 */
inline std::string to_debug_string(const ContinuationBase* continuation) {
    if (!continuation) return "Continuation(nullptr)";
    return std::string("Continuation(") + typeid(*continuation).name() + ")";
}

} // namespace coroutines
} // namespace kotlinx
