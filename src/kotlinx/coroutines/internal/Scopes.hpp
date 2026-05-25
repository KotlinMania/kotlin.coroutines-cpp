#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/Scopes.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Upstream Scopes.kt declares two open classes:
 *   - ScopeCoroutine<in T>: declared here as the C++ port also keeps it template-shaped.
 *     The fuller implementation continues to live in ScopeCoroutine.hpp/.cpp; this header
 *     captures the line-for-line surface that matches the upstream file boundary.
 *   - ContextScope: a thin CoroutineScope wrapper around a captured CoroutineContext.
 */

#include "kotlinx/coroutines/AbstractCoroutine.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"

#include <memory>
#include <string>

namespace kotlinx::coroutines::internal {

/**
 * Upstream:
 *   internal class ContextScope(context: CoroutineContext) : CoroutineScope {
 *       override val coroutineContext: CoroutineContext = context
 *       override fun toString(): String = "CoroutineScope(coroutineContext=$coroutineContext)"
 *   }
 */
class ContextScope : public CoroutineScope {
public:
    explicit ContextScope(std::shared_ptr<CoroutineContext> context)
        : context_(std::move(context)) {}

    std::shared_ptr<CoroutineContext> coroutine_context() const override { return context_; }

    std::string to_string() const {
        return std::string("CoroutineScope(coroutineContext=") +
               (context_ ? context_->to_string() : std::string("null")) + ")";
    }

private:
    std::shared_ptr<CoroutineContext> context_;
};

} // namespace kotlinx::coroutines::internal
