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
    explicit SafeCollectorBase(std::shared_ptr<CoroutineContext> collectContext);
    virtual ~SafeCollectorBase() = default;

    const std::shared_ptr<CoroutineContext>& get_collect_context() const { return collect_context_; }
    int get_collect_context_size() const { return collect_context_size_; }

protected:
    void check_context(const CoroutineContext& currentContext);
    
    std::shared_ptr<CoroutineContext> collect_context_;
    int collect_context_size_;
    std::shared_ptr<CoroutineContext> last_emission_context_;
};

/**
 * SafeCollector that ensures flow invariants and context preservation.
 *
 * This wrapper collector ensures that emissions happen in the correct context
 * and provides exception transparency guarantees. It wraps a downstream collector
 * and validates context before forwarding emissions.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/SafeCollector.common.kt:11-20
 *                 and kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt:7-28
 */
template <typename T>
class SafeCollector : public FlowCollector<T>, public SafeCollectorBase {
public:
    /**
     * Creates a SafeCollector wrapping the given downstream collector.
     *
     * @param downstream The collector to wrap and protect
     * @param collectContext The context in which collection started
     *
     * Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt:7-10
     */
    SafeCollector(FlowCollector<T>* downstream, std::shared_ptr<CoroutineContext> collectContext)
        : SafeCollectorBase(std::move(collectContext)), downstream_(downstream) {}

    /**
     * Emits a value after validating the execution context.
     *
     * Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt:16-24
     */
    void* emit(T value, Continuation<void*>* continuation) override {
        auto current_context = continuation ? continuation->get_context() : collect_context_;
        if (!current_context) {
            current_context = EmptyCoroutineContext::instance();
        }
        context_ensure_active(*current_context);
        if (last_emission_context_.get() != current_context.get()) {
            check_context(*current_context);
            last_emission_context_ = current_context;
        }
        return downstream_->emit(std::move(value), continuation);
    }

    /**
     * Releases any intercepted continuation resources.
     *
     * Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt:26-27
     */
    void release_intercepted() {
    }

private:
    FlowCollector<T>* downstream_;
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

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Flow.kt:223-230
template<typename T>
inline void* AbstractFlow<T>::collect(FlowCollector<T>* collector, Continuation<void*>* continuation) {
    // NOTE(port): The frame owns SafeCollector through collectSafely and its finally block.
    class CollectFrame final : public ContinuationImpl {
    public:
        CollectFrame(AbstractFlow<T>* flow, FlowCollector<T>* collector, Continuation<void*>* completion)
            : ContinuationImpl(std::shared_ptr<Continuation<void*>>(completion, [](Continuation<void*>*) {})),
              flow_(flow), safe_collector_(std::make_shared<internal::SafeCollector<T>>(
                  collector, completion ? completion->get_context() : EmptyCoroutineContext::instance())) {}

        void retain() { self_ref_ = shared_from_this(); }

        // Transliterated from: kotlinx-coroutines-core/common/src/flow/Flow.kt:223-230
        void* invoke_suspend(Result<void*> result) override {
            try {
                coroutine_begin(this)
                coroutine_yield(this, flow_->collect_safely(safe_collector_.get(), this));
                safe_collector_->release_intercepted();
                self_ref_.reset();
                coroutine_end(this)
            } catch (...) {
                safe_collector_->release_intercepted();
                self_ref_.reset();
                throw;
            }
        }

    private:
        void* _label = nullptr;
        AbstractFlow<T>* flow_;
        std::shared_ptr<internal::SafeCollector<T>> safe_collector_;
        std::shared_ptr<BaseContinuationImpl> self_ref_;
    };
    auto frame = std::make_shared<CollectFrame>(this, collector, continuation);
    frame->retain();
    return frame->invoke_suspend(Result<void*>::success(nullptr));
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
