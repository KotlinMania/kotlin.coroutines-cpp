// port-lint: source kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt
 *
 * Public generic terminal operators are in Collect.hpp. Concrete lowering
 * frames for collect_latest and its map_latest action are implemented here.
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/Collect.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include <functional>
#include <stdexcept>

#include "kotlinx/coroutines/flow/internal/NopCollector.hpp"

namespace kotlinx::coroutines::flow::internal {
namespace {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:211-213
class MapLatestActionFrame final : public ContinuationImpl {
public:
    MapLatestActionFrame(FlowCollector<Unit>* collector,
                        std::function<void*(Continuation<void*>*)> action,
                        Continuation<void*>* completion)
        : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
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
        : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
          mapped_(std::move(mapped)) {}

    void retain() { self_ref_ = shared_from_this(); }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:82-97; flow/operators/Merge.kt:211-213
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            coroutine_yield(this, mapped_->collect(&nop_, this));
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
