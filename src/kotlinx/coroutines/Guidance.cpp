// port-lint: source Guidance.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/Guidance.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 *
 * Upstream provides two `@Deprecated(level = ERROR)` overloads of `launch` and `async`
 * that exist only to give compile-time guidance to users who try to call those builders
 * without a CoroutineScope receiver. Both throw at runtime ("Should never be called").
 *
 * The C++ port keeps both as runtime-throwing stubs so the same calls compile and fail
 * loudly with the same upstream message. `@Deprecated`, `@Suppress`, and
 * `@kotlin.internal.LowPriorityInOverloadResolution` have no C++ analogues — the
 * compile-time error mechanism is a documentation-only concern here.
 */

#include <functional>
#include <memory>
#include <stdexcept>

namespace kotlinx::coroutines {

struct CoroutineContext;
struct CoroutineScope;
struct CoroutineStart;
struct Job;
template <typename T>
class Deferred;

/**
 * Upstream:
 *   @Deprecated(level = DeprecationLevel.ERROR)
 *   public fun CoroutineScope?.launch(...): Job =
 *       error("Should never be called, was introduced to help with incomplete code")
 */
Job *launch(const std::shared_ptr<CoroutineContext>& /*context*/,
            const std::shared_ptr<CoroutineStart>& /*start*/,
            std::function<void(CoroutineScope &)> /*block*/) {
    throw std::logic_error(
        "Should never be called, was introduced to help with incomplete code");
}

/**
 * Upstream:
 *   @Deprecated(level = DeprecationLevel.ERROR)
 *   public fun <T> CoroutineScope?.async(...): Deferred<T> =
 *       error("Should never be called, was introduced to help with incomplete code")
 */
template <typename T>
Deferred<T> *async(const std::shared_ptr<CoroutineContext>& /*context*/,
                   const std::shared_ptr<CoroutineStart>& /*start*/,
                   std::function<T(CoroutineScope &)> /*block*/) {
    throw std::logic_error(
        "Should never be called, was introduced to help with incomplete code");
}

} // namespace kotlinx::coroutines
