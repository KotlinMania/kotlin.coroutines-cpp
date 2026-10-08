#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/SafeCollector.common.kt
 *                 and kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt
 */

#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include <memory>
#include <functional>
#include <string>
#include <utility>

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

/**
 * Traverses parent coroutines while the job is a scoped coroutine.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/SafeCollector.common.kt:92-97
 */
std::shared_ptr<Job> transitive_coroutine_parent(
    std::shared_ptr<Job> current_job,
    const std::shared_ptr<Job>& collect_job
);

/**
 * Base class for SafeCollector containing non-generic context validation logic.
 * Moved to .cpp file to reduce template bloat.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/SafeCollector.common.kt:11-20
 *                 and kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt:7-14
 */
class SafeCollectorBase {
public:
    explicit SafeCollectorBase(std::shared_ptr<CoroutineContext> collect_context);
    virtual ~SafeCollectorBase() = default;

    // Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt:9-9
    const std::shared_ptr<CoroutineContext>& get_collect_context() const { return collect_context_; }
    // Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt:13-13
    int get_collect_context_size() const { return collect_context_size_; }

protected:
    void check_context(const CoroutineContext& current_context);
    
    std::shared_ptr<CoroutineContext> collect_context_;
    int collect_context_size_;
    std::shared_ptr<CoroutineContext> last_emission_context_;
};

/**
 * SafeCollector that ensures flow invariants and context preservation.
 *
 * This Native wrapper validates the emission context and checks cancellation
 * before forwarding each value to the downstream collector.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/SafeCollector.common.kt:11-20
 *                 and kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt:7-28
 */
template <typename T>
class SafeCollector final : public FlowCollector<T>, public SafeCollectorBase {
public:
    /**
     * Creates a SafeCollector wrapping the given downstream collector.
     *
     * @param collector The collector to wrap and protect
     * @param collect_context The context in which collection started
     *
     * Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt:7-10
     */
    SafeCollector(FlowCollector<T>* collector, std::shared_ptr<CoroutineContext> collect_context)
        : SafeCollectorBase(std::move(collect_context)), collector_(collector) {}

    // Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt:8-8
    FlowCollector<T>* get_collector() const { return collector_; }

    /**
     * Emits a value after validating the execution context.
     *
     * Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt:16-24
     */
    void* emit(T value, Continuation<void*>* continuation) override {
        auto current_context = continuation->get_context();
        context_ensure_active(*current_context);
        if (last_emission_context_.get() != current_context.get()) {
            check_context(*current_context);
            last_emission_context_ = current_context;
        }
        return collector_->emit(std::move(value), continuation);
    }

    /**
     * Releases any intercepted continuation resources.
     *
     * Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt:26-27
     */
    void release_intercepted() {
    }

private:
    FlowCollector<T>* collector_;
};

/**
 * An analogue of the [flow] builder that does not check the context of execution of the resulting flow.
 * Used in our own operators where we trust the context of invocations.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/SafeCollector.common.kt:104-110
 */
template <typename T>
std::shared_ptr<Flow<T>> unsafe_flow(std::function<void*(FlowCollector<T>*, Continuation<void*>*)> block) {
    class UnsafeFlowImpl : public Flow<T> {
        std::function<void*(FlowCollector<T>*, Continuation<void*>*)> block_;
    public:
        explicit UnsafeFlowImpl(std::function<void*(FlowCollector<T>*, Continuation<void*>*)> b)
            : block_(std::move(b)) {}

        void* collect(FlowCollector<T>* collector, Continuation<void*>* continuation) override {
            return block_(collector, continuation);
        }
    };
    return std::make_shared<UnsafeFlowImpl>(std::move(block));
}

template <typename T>
inline std::shared_ptr<Flow<T>> unsafe_flow(std::function<void(FlowCollector<T>*)> block) {
    return unsafe_flow<T>([block = std::move(block)](FlowCollector<T>* collector, Continuation<void*>*) -> void* {
        block(collector);
        return nullptr;
    });
}

} // namespace internal

using internal::unsafe_flow;


} // namespace flow
} // namespace coroutines
} // namespace kotlinx
