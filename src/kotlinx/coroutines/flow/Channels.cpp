// port-lint: source kotlinx-coroutines-core/common/src/flow/Channels.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt
 */
#include "kotlinx/coroutines/flow/Channels.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include <utility>

namespace kotlinx::coroutines::flow::internal {
namespace {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
// NOTE(port): The source for-loop has two suspend points. Typed bindings carry
// its iterator and element; the concrete frame uses the existing Continuation ABI.
class EmitAllContinuation final : public ContinuationImpl {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
    EmitAllContinuation(std::function<void()> make_iterator,
                        std::function<void*(Continuation<void*>*)> has_next,
                        std::function<void*(Continuation<void*>*)> emit_next,
                        std::function<void()> finish_emit,
                        std::function<void(std::exception_ptr)> cancel_consumed,
                        bool consume, std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)),
          make_iterator_(std::move(make_iterator)), has_next_(std::move(has_next)),
          emit_next_(std::move(emit_next)), finish_emit_(std::move(finish_emit)),
          cancel_consumed_(std::move(cancel_consumed)), consume_(consume) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
    // NOTE(port): Retain the source suspended frame without a Kotlin GC root.
    void retain() { self_ref_ = shared_from_this(); }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            make_iterator_();
            while (true) {
                coroutine_yield_value(this, result, std::function(has_next_)(this), has_next_box_);
                // NOTE(port): The iterator transfers an owning bool box to this
                // consumer on both immediate and resumed has_next returns.
                more_ = *static_cast<bool*>(has_next_box_);
                delete static_cast<bool*>(has_next_box_);
                has_next_box_ = nullptr;
                if (!more_) break;
                coroutine_yield(this, std::function(emit_next_)(this));
                finish_emit_();
            }
        } catch (...) {
            cause_ = std::current_exception();
            if (consume_) cancel_consumed_(cause_);
            std::rethrow_exception(cause_);
        }
        if (consume_) cancel_consumed_(cause_);
        coroutine_end(this)
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
    // NOTE(port): Clearing terminated spills breaks the iterator's cancelled
    // continuation cycle and releases owned arguments even if the frame survives.
    void release_intercepted() override {
        auto self = std::move(self_ref_);
        make_iterator_ = {};
        has_next_ = {};
        emit_next_ = {};
        finish_emit_ = {};
        cancel_consumed_ = {};
        cause_ = nullptr;
        ContinuationImpl::release_intercepted();
    }

private:
    void* _label = nullptr;
    std::function<void()> make_iterator_;
    std::function<void*(Continuation<void*>*)> has_next_;
    std::function<void*(Continuation<void*>*)> emit_next_;
    std::function<void()> finish_emit_;
    std::function<void(std::exception_ptr)> cancel_consumed_;
    bool consume_;
    void* has_next_box_ = nullptr;
    bool more_ = false;
    std::exception_ptr cause_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};

} // namespace

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
void* emit_all_erased(
    std::function<void()> make_iterator,
    std::function<void*(Continuation<void*>*)> has_next,
    std::function<void*(Continuation<void*>*)> emit_next,
    std::function<void()> finish_emit,
    std::function<void(std::exception_ptr)> cancel_consumed,
    bool consume, std::shared_ptr<Continuation<void*>> completion) {
    auto frame = std::make_shared<EmitAllContinuation>(
        std::move(make_iterator), std::move(has_next), std::move(emit_next),
        std::move(finish_emit), std::move(cancel_consumed), consume, std::move(completion));
    frame->retain();
    return frame->start(Result<void*>::success(nullptr));
}

} // namespace kotlinx::coroutines::flow::internal
