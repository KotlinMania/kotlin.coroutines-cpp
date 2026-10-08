// port-lint: source kotlinx-coroutines-core/native/src/CoroutineContext.kt
#pragma once
/**
 * @file CoroutineContextUtils.hpp
 *
 * Transliterated from:
 * - kotlinx-coroutines-core/common/src/CoroutineContext.common.kt (expect declarations)
 * - kotlinx-coroutines-core/native/src/CoroutineContext.kt (native actuals)
 *
 * The Native context wrappers directly invoke their block.
 */

#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include <string>
#include <typeinfo>

namespace kotlinx {
namespace coroutines {

// No debugging facilities on native
// countOrElement -- pre-cached value for ThreadContext.kt
// NOTE(port): Native does not read the context/cache arguments. Keep their
// source names as comments; accept the inline block without type erasure.
// Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:43-43
template<typename R, typename Block>
inline R with_coroutine_context(
    const std::shared_ptr<CoroutineContext>& /* context */,
    void* /* count_or_element */,
    Block&& block
) {
    return block();
}

// Transliterated from: kotlinx-coroutines-core/common/src/CoroutineContext.common.kt:24-26
// NOTE(port): The same Native inline-block projection applies here.
// Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:44-44
template<typename R, typename T, typename Block>
inline R with_continuation_context(
    const std::shared_ptr<Continuation<T>>& /* continuation */,
    void* /* count_or_element */,
    Block&& block
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
