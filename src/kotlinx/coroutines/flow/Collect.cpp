// port-lint: source kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.flow
 *
 * Terminal flow operators: collect, launchIn, collectIndexed, collectLatest, emitAll.
 * The templated entry points (collect / launch_in / etc.) live in the matching header
 * (flow/Flow.hpp + flow/Collect.hpp); this translation unit owns the non-templated
 * NopCollector and the check_index_overflow helper.
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/Collect.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include <functional>
#include <stdexcept>

#include "kotlinx/coroutines/flow/internal/NopCollector.hpp"

namespace kotlinx {
    namespace coroutines {
        namespace flow {

            /**
 * Helper to check for index overflow.
 */
            inline int check_index_overflow(int index) {
                if (index < 0) {
                    throw std::overflow_error("Index overflow has happened");
                }
                return index;
            }

            // Note: Template functions are declared in headers.
            // The implementations here are for documentation and non-template helpers only.
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx
namespace kotlinx::coroutines::flow::internal {
namespace {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:211-213
class MapLatestActionFrame final : public ContinuationImpl {
public:
    MapLatestActionFrame(FlowCollector<Unit>* collector,
                        std::function<void*(Continuation<void*>*)> action,
                        Continuation<void*>* completion)
        : ContinuationImpl(std::shared_ptr<Continuation<void*>>(
              completion, [](Continuation<void*>*) {})),
          collector_(collector), action_(std::move(action)) {}

    void retain() { self_ref_ = shared_from_this(); }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:82-97; flow/operators/Merge.kt:211-213
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            coroutine_yield(this, action_(this));
            coroutine_yield(this, collector_->emit(Unit{}, this));
            self_ref_.reset();
            coroutine_end(this)
        } catch (...) {
            self_ref_.reset();
            throw;
        }
    }

private:
    void* _label = nullptr;
    FlowCollector<Unit>* collector_;
    std::function<void*(Continuation<void*>*)> action_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:82-97
class CollectLatestFrame final : public ContinuationImpl {
public:
    CollectLatestFrame(std::shared_ptr<Flow<Unit>> mapped, Continuation<void*>* completion)
        : ContinuationImpl(std::shared_ptr<Continuation<void*>>(
              completion, [](Continuation<void*>*) {})), mapped_(std::move(mapped)) {}

    void retain() { self_ref_ = shared_from_this(); }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:82-97; flow/operators/Merge.kt:211-213
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            coroutine_yield(this, mapped_->collect(&nop_, completion ? this : nullptr));
            self_ref_.reset();
            coroutine_end(this)
        } catch (...) {
            self_ref_.reset();
            throw;
        }
    }

private:
    void* _label = nullptr;
    std::shared_ptr<Flow<Unit>> mapped_;
    NopCollector<Unit> nop_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};

} // namespace

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:211-213
void* map_latest_action(FlowCollector<Unit>* collector,
                       std::function<void*(Continuation<void*>*)> action,
                       Continuation<void*>* completion) {
    auto frame = std::make_shared<MapLatestActionFrame>(collector, std::move(action), completion);
    frame->retain();
    return frame->start(Result<void*>::success(nullptr));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:82-97
void* collect_latest_impl(std::shared_ptr<Flow<Unit>> mapped, Continuation<void*>* completion) {
    auto frame = std::make_shared<CollectLatestFrame>(std::move(mapped), completion);
    frame->retain();
    return frame->start(Result<void*>::success(nullptr));
}

} // namespace kotlinx::coroutines::flow::internal
