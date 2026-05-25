// port-lint: source CoroutineContext.common.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/CoroutineContext.common.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 *
 * Upstream declares the common `CoroutineContext` plus operator and helper API plus
 * `expect fun newCoroutineContext(...)` (one for `CoroutineScope.newCoroutineContext(CoroutineContext)`
 * and one for `CoroutineContext.newCoroutineContext(CoroutineContext)`), `expect inline fun
 * withCoroutineContext`, `expect inline fun withContinuationContext`, and
 * `expect fun Continuation<*>.toDebugString()`. The actuals live in the per-platform
 * subdirectories (`concurrent/`, `native/`, `js/`). This translation unit owns the
 * common-source bits: the `operator+` fold and the `coroutine_name` extension.
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/CoroutineName.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/context_impl.hpp"

#include <functional>
#include <memory>
#include <string>

namespace kotlinx::coroutines {

/**
 * Upstream:
 *   public operator fun CoroutineContext.plus(context: CoroutineContext): CoroutineContext =
 *       if (context === EmptyCoroutineContext) this else
 *           context.fold(this) { acc, element ->
 *               val removed = acc.minusKey(element.key)
 *               if (removed === EmptyCoroutineContext) element else CombinedContext(removed, element)
 *           }
 */
std::shared_ptr<CoroutineContext> CoroutineContext::operator+(
    std::shared_ptr<CoroutineContext> other) const {
    if (!other) return std::const_pointer_cast<CoroutineContext>(shared_from_this());

    return other->fold<std::shared_ptr<CoroutineContext>>(
        std::const_pointer_cast<CoroutineContext>(shared_from_this()),
        [](std::shared_ptr<CoroutineContext> acc, std::shared_ptr<Element> element) {
            auto removed = acc->minus_key(element->key());
            if (!removed) {
                return std::static_pointer_cast<CoroutineContext>(element);
            }
            return std::static_pointer_cast<CoroutineContext>(
                std::make_shared<CombinedContext>(removed, element));
        });
}

/**
 * Forward declarations for the `expect fun` family. Each platform target provides the
 * corresponding `actual` body (see native/CoroutineContext.cpp, concurrent/CoroutineContext.cpp).
 */
std::shared_ptr<CoroutineContext> new_coroutine_context(
    CoroutineScope* scope, std::shared_ptr<CoroutineContext> context);

std::shared_ptr<CoroutineContext> new_coroutine_context(
    std::shared_ptr<CoroutineContext> base_context,
    std::shared_ptr<CoroutineContext> added_context);

std::string to_debug_string(Continuation<void>* continuation);

/**
 * Upstream:
 *   internal expect inline fun <T> withCoroutineContext(
 *       context: CoroutineContext, countOrElement: Any?, block: () -> T): T
 *
 * The actual body is per-platform; this is the forwarding declaration so the
 * common-source call sites can name it.
 */
template <typename T>
T with_coroutine_context(std::shared_ptr<CoroutineContext> context,
                         void* count_or_element,
                         std::function<T()> block);

/**
 * Upstream:
 *   internal expect inline fun <T> withContinuationContext(
 *       continuation: Continuation<*>, countOrElement: Any?, block: () -> T): T
 */
template <typename T>
T with_continuation_context(Continuation<void>* continuation,
                            void* count_or_element,
                            std::function<T()> block);

/**
 * Upstream:
 *   internal actual val CoroutineContext.coroutineName: String?
 *
 * Upstream native returns null when no [CoroutineName] is present. The C++ port returns
 * an empty string in the absent case so callers can format unconditionally; the
 * `value().empty()` predicate is the equivalent of the upstream null check.
 */
std::string coroutine_name(const std::shared_ptr<CoroutineContext>& context) {
    if (!context) return "";
    auto element = context->get(CoroutineName::type_key);
    if (!element) return "";
    auto name_element = std::dynamic_pointer_cast<CoroutineName>(element);
    if (!name_element) return "";
    return name_element->name;
}

} // namespace kotlinx::coroutines
