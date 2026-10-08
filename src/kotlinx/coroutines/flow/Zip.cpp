/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt
 */
// port-lint: source kotlinx-coroutines-core/common/src/flow/operators/Zip.kt
#include "kotlinx/coroutines/flow/Zip.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include <utility>

namespace kotlinx::coroutines::flow::internal {
namespace {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:28-30,254-260
// NOTE(port): Lowered frame for the two suspension points in emit(transform(it)).
class CombineEmitContinuation final : public ContinuationImpl {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:28-30,254-260
    CombineEmitContinuation(std::function<void*(Continuation<void*>*)> transform,
        std::function<void*(void*, Continuation<void*>*)> emit,
        std::function<void(void*)> delete_result, Continuation<void*>* completion)
        : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
          transform_(std::move(transform)), emit_(std::move(emit)),
          delete_result_(std::move(delete_result)) {}
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:28-30,254-260
    void retain() { self_ref_ = shared_from_this(); }
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:28-30,254-260
    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)
        coroutine_yield_value(this, result, std::function(transform_)(this), result_box_);
        coroutine_yield(this, emit_result());
        coroutine_end(this)
    }
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:28-30,254-260
    // NOTE(port): Release completed spills even when the actual frame remains externally retained.
    void release_intercepted() override {
        auto self = std::move(self_ref_);
        if (result_box_) delete_result_(std::exchange(result_box_, nullptr));
        transform_ = {};
        emit_ = {};
        delete_result_ = {};
        ContinuationImpl::release_intercepted();
    }
private:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:28-30,254-260
    void* emit_result() { return std::function(emit_)(std::exchange(result_box_, nullptr), this); }
    void* _label = nullptr;
    void* result_box_ = nullptr;
    std::function<void*(Continuation<void*>*)> transform_;
    std::function<void*(void*, Continuation<void*>*)> emit_;
    std::function<void(void*)> delete_result_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};
} // namespace

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:28-30,254-260
void* emit_combine_result(std::function<void*(Continuation<void*>*)> transform,
    std::function<void*(void*, Continuation<void*>*)> emit,
    std::function<void(void*)> delete_result, Continuation<void*>* completion) {
    auto frame = std::make_shared<CombineEmitContinuation>(std::move(transform), std::move(emit),
                                                         std::move(delete_result), completion);
    frame->retain();
    return frame->start(Result<void*>::success(nullptr));
}
} // namespace kotlinx::coroutines::flow::internal
