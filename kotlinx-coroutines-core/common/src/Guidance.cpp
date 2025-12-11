#include <functional>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ (first-pass, mechanical syntax mapping)
// Original: kotlinx-coroutines-core/common/src/Guidance.kt
//
// TODO:
// - Deprecated annotations need to be represented as comments or static_assert
// - Suspend lambda types need coroutine infrastructure
// - EmptyCoroutineContext needs implementation
// - Default parameter values need overload functions

#include <stdexcept>

namespace kotlinx {
namespace coroutines {

struct CoroutineContext;
struct CoroutineScope;
struct CoroutineStart;
struct Job;
template<typename T> class Deferred;

/**
 * @suppress this is a function that should help users who are trying to use 'launch'
 * without the corresponding coroutine scope. It is not supposed to be called.
 */
// @Deprecated("'launch' can not be called without the corresponding coroutine scope. " +
//     "Consider wrapping 'launch' in 'coroutineScope { }', using 'runBlocking { }', " +
//     "or using some other 'CoroutineScope'", level = DeprecationLevel.ERROR)
// @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
// @kotlin.internal.LowPriorityInOverloadResolution
// TODO: suspend lambda parameter needs coroutine infrastructure
Job* launch(
    CoroutineContext& context, // = EmptyCoroutineContext
    CoroutineStart& start, // = CoroutineStart.DEFAULT
    std::function<void(CoroutineScope&)> block
) {
    throw std::logic_error("Should never be called, was introduced to help with incomplete code");
}

/**
 * @suppress this is a function that should help users who are trying to use 'launch'
 * without the corresponding coroutine scope. It is not supposed to be called.
 */
// @Deprecated("'async' can not be called without the corresponding coroutine scope. " +
//     "Consider wrapping 'async' in 'coroutineScope { }', using 'runBlocking { }', " +
//     "or using some other 'CoroutineScope'", level = DeprecationLevel.ERROR)
// @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
// @kotlin.internal.LowPriorityInOverloadResolution
// TODO: suspend lambda parameter needs coroutine infrastructure
template<typename T>
Deferred<T>* async(
    CoroutineContext& context, // = EmptyCoroutineContext
    CoroutineStart& start, // = CoroutineStart.DEFAULT
    std::function<T(CoroutineScope&)> block
) {
    throw std::logic_error("Should never be called, was introduced to help with incomplete code");
}

} // namespace coroutines
} // namespace kotlinx
