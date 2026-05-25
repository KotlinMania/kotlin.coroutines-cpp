#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/Unconfined.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 */

#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Runnable.hpp"

#include <memory>
#include <stdexcept>
#include <string>

namespace kotlinx::coroutines {

class YieldContext;

/**
 * A coroutine dispatcher that is not confined to any specific thread.
 *
 * Upstream:
 *   internal object Unconfined : CoroutineDispatcher() {
 *       override fun limitedParallelism(parallelism: Int, name: String?): CoroutineDispatcher
 *       override fun isDispatchNeeded(context: CoroutineContext): Boolean = false
 *       override fun dispatch(context: CoroutineContext, block: Runnable) { ... }
 *       override fun toString(): String = "Dispatchers.Unconfined"
 *   }
 *
 * Kotlin `object` → C++ singleton. The default constructor is private and the only access
 * path is [instance].
 */
class Unconfined : public CoroutineDispatcher {
public:
    static Unconfined& instance() {
        static Unconfined singleton;
        return singleton;
    }

    /**
     * Upstream:
     *   override fun limitedParallelism(parallelism: Int, name: String?): CoroutineDispatcher {
     *       throw UnsupportedOperationException("limitedParallelism is not supported for Dispatchers.Unconfined")
     *   }
     */
    CoroutineDispatcher& limited_parallelism(int /*parallelism*/, const std::string* /*name*/) override {
        throw std::logic_error("limitedParallelism is not supported for Dispatchers.Unconfined");
    }

    /** Upstream: override fun isDispatchNeeded(context: CoroutineContext): Boolean = false */
    bool is_dispatch_needed(const CoroutineContext& /*context*/) override { return false; }

    /**
     * Upstream:
     *   override fun dispatch(context: CoroutineContext, block: Runnable) {
     *       val yieldContext = context[YieldContext]
     *       if (yieldContext != null) { yieldContext.dispatcherWasUnconfined = true; return }
     *       throw UnsupportedOperationException("Dispatchers.Unconfined.dispatch ...")
     *   }
     */
    void dispatch(const CoroutineContext& context, Runnable& block) override;

    /** Upstream: override fun toString(): String = "Dispatchers.Unconfined" */
    std::string to_string() const override { return "Dispatchers.Unconfined"; }

private:
    Unconfined() = default;
};

/**
 * Used to detect calls to [Unconfined.dispatch] from [yield] function.
 *
 * Upstream:
 *   @PublishedApi
 *   internal class YieldContext : AbstractCoroutineContextElement(Key) {
 *       companion object Key : CoroutineContext.Key<YieldContext>
 *       @JvmField var dispatcherWasUnconfined = false
 *   }
 *
 * The `@JvmField` annotation only affects JVM bytecode emission and is dropped here.
 */
class YieldContext : public AbstractCoroutineContextElement {
public:
    /** Upstream: companion object Key : CoroutineContext.Key<YieldContext> */
    static const CoroutineContext::Key* type_key();

    YieldContext() : AbstractCoroutineContextElement(type_key()) {}

    bool dispatcher_was_unconfined = false;
};

} // namespace kotlinx::coroutines
