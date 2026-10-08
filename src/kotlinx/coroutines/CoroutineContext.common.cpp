// port-lint: source CoroutineContext.common.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/CoroutineContext.common.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 *
 * Upstream declares the common helper API and
 * `expect fun newCoroutineContext(...)` (one for `CoroutineScope.newCoroutineContext(CoroutineContext)`
 * and one for `CoroutineContext.newCoroutineContext(CoroutineContext)`), `expect inline fun
 * withCoroutineContext`, `expect inline fun withContinuationContext`, and
 * `expect fun Continuation<*>.toDebugString()`. The actuals live in the per-platform
 * subdirectories (`concurrent/`, `native/`, `js/`). The stdlib context
 * composition algorithm is translated in context_impl.cpp.
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

// Transliterated from: kotlinx-coroutines-core/common/src/CoroutineContext.common.kt:27-27
std::optional<std::string> coroutine_name(const std::shared_ptr<CoroutineContext>& context);

} // namespace kotlinx::coroutines
