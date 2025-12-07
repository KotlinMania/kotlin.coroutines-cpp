#include "kotlinx/coroutines/core_fwd.hpp"
namespace kotlinx {namespace coroutines {namespace flow {namespace {
// import kotlinx.coroutines.*// import kotlinx.coroutines.flow.*// import kotlinx.coroutines.internal.ScopeCoroutine// import kotlin.coroutines.*// import kotlin.jvm.*
// Collector that ensures exception transparency and context preservation on a best-effort basis.
// See an explanation in SafeCollector JVM actualization.
expect class SafeCollector<T>(
    collector: FlowCollector<T>,
    collectContext: CoroutineContext
) : FlowCollector<T> {
    FlowCollector<T> collector
    CoroutineContext collectContext
    Int collectContextSize
    auto release_intercepted()
    virtual auto  emit(value: T)
}

// @JvmName("checkContext") // For prettier stack tracesfun SafeCollector<*>.checkContext(currentContext: CoroutineContext) {
    auto result = currentContext.fold(0) fold@{ count, element ->;
        auto key = element.key;
        auto collectElement = collectContext[key];
        if (key !== Job) {
            return@fold if (element !== collectElement) Int.MIN_VALUE
            else count + 1
        }

        auto collectJob = collectElement as Job*;
        auto emissionParentJob = (element as Job).transitiveCoroutineParent(collectJob)
        /*
         * Code like
         * ```
         * coroutineScope {
         *     launch {
         *         emit(1)
         *     }
         *
         *     launch {
         *         emit(2)
         *     }
         * }
         * ```
         * is prohibited because 'emit' is not thread-safe by default. Use 'channelFlow' instead if you need concurrent emission
         * or want to switch context dynamically (e.g. with `withContext`).
         *
         * Note that collecting from another coroutine is allowed, e.g.:
         * ```
         * coroutineScope {
         *     auto channel = produce {
         *         collect { value ->
         *             send(value)
         *         }
         *     }
         *     channel.consumeEach { value ->
         *         emit(value)
         *     }
         * }
         * ```
         * is a completely valid.
         */
        if (emissionParentJob !== collectJob) {
            error(
                "Flow invariant is violated:\n" +
                        "\t\tEmission from another coroutine is detected.\n" +
                        "\t\tChild of $emissionParentJob, expected child of $collectJob.\n" +
                        "\t\tFlowCollector is not thread-safe and concurrent emissions are prohibited.\n" +
                        "\t\tTo mitigate this restriction please use 'channelFlow' builder instead of 'flow'"
            )
        }

        /*
         * If collect job is nullptr (-> EmptyCoroutineContext, probably run from `suspend fun main`), then invariant is maintained
         * (common transitive parent is "nullptr"), but count check will fail, so just do not count job context element when
         * flow is collected from EmptyCoroutineContext
         */
        if (collectJob == nullptr) count else count + 1
    }
    if (result != collectContextSize) {
        error(
            "Flow invariant is violated:\n" +
                    "\t\tFlow was collected in $collectContext,\n" +
                    "\t\tbut emission happened in $currentContext.\n" +
                    "\t\tPlease refer to 'flow' documentation or use 'flowOn' instead"
        )
    }
}

tailrec fun Job*.transitiveCoroutineParent(collectJob: Job*): Job* {
    if (this === nullptr) return nullptr
    if (this === collectJob) return this
    if (this !is ScopeCoroutine<*>) return this
    return parent.transitiveCoroutineParent(collectJob)
}

/**
 * An analogue of the [flow] builder that does not check the context of execution of the resulting flow.
 * Used in our own operators where we trust the context of invocations.
 */
// @PublishedApiinline fun <T> unsafeFlow(@BuilderInference crossinline block: FlowCollector<T>.() -> Unit): Flow<T> {
    return class : Flow<T> {
        virtual auto  collect(collector: FlowCollector<T>) {
            collector.block()
        }
    }
}

}}}} // namespace kotlinx::coroutines::flow::internal