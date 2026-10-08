// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt
/** Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt */
#include "kotlinx/coroutines/intrinsics/IntrinsicsNative.hpp"
#include <mutex>

namespace kotlin::coroutines::intrinsics {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:201-202
std::shared_ptr<Continuation<void*>> intercepted(std::shared_ptr<Continuation<void*>> continuation) {
    auto frame = std::dynamic_pointer_cast<ContinuationImpl>(continuation);
    return frame ? frame->intercepted() : continuation;
}


namespace {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:228-244
class RestrictedCreatedContinuation final : public RestrictedContinuationImpl {
public:
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:228-244
    RestrictedCreatedContinuation(std::shared_ptr<Continuation<void*>> completion, ErasedSuspendFunction block)
        : RestrictedContinuationImpl(std::move(completion)), block_(std::move(block)) {}
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:228-244
    void* invoke_suspend(Result<void*> result) override {
        switch (label_) {
            case 0: {
                label_ = 1;
                result.get_or_throw(); // Rethrow if trying to start with an exception.
                auto block = block_; // Keep captures alive through inline completion.
                return block(shared_from_this()); // May return or suspend.
            }
            case 1:
                label_ = 2;
                return result.get_or_throw(); // Result of the suspended block.
            default:
                throw std::logic_error("This coroutine had already completed");
        }
    }
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:51-53
    // NOTE(port): Native GC reclaims captures after the actual frame terminates.
    void release_intercepted() override {
        RestrictedContinuationImpl::release_intercepted();
        block_ = nullptr;
    }
private:
    int label_ = 0;
    ErasedSuspendFunction block_;
};
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:246-262
class CreatedContinuation final : public ContinuationImpl {
public:
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:246-262
    CreatedContinuation(std::shared_ptr<Continuation<void*>> completion, ErasedSuspendFunction block)
        : ContinuationImpl(std::move(completion)), block_(std::move(block)) {}
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:246-262
    void* invoke_suspend(Result<void*> result) override {
        switch (label_) {
            case 0: {
                label_ = 1;
                result.get_or_throw(); // Rethrow if trying to start with an exception.
                auto block = block_; // Keep captures alive through inline completion.
                return block(shared_from_this()); // May return or suspend.
            }
            case 1:
                label_ = 2;
                return result.get_or_throw(); // Result of the suspended block.
            default:
                throw std::logic_error("This coroutine had already completed");
        }
    }
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:51-53
    // NOTE(port): Native GC reclaims captures after the actual frame terminates.
    void release_intercepted() override {
        ContinuationImpl::release_intercepted();
        block_ = nullptr;
    }
private:
    int label_ = 0;
    ErasedSuspendFunction block_;
};
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:315-319
class RestrictedSimpleContinuation final : public RestrictedContinuationImpl {
public:
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:315-319
    explicit RestrictedSimpleContinuation(std::shared_ptr<Continuation<void*>> completion)
        : RestrictedContinuationImpl(std::move(completion)) {}
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:315-319
    void* invoke_suspend(Result<void*> result) override { return result.get_or_throw(); }
    // NOTE(port): Per-invocation typed argument ownership; copying the callable
    // never shares a mutable continuation argument between fresh computations.
    void retain_argument(std::shared_ptr<void> argument) {
        std::lock_guard<std::mutex> lock(ownership_mutex_);
        argument_owner_ = std::move(argument);
    }
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:26-34,315-319
    // NOTE(port): Native GC retains the implicit continuation and typed projection.
    // Own the actual frame only while suspended; an unstarted wrapper has no root.
    void* invoke(ErasedSuspendFunction block) {
        { std::lock_guard<std::mutex> lock(ownership_mutex_); block_ = block; }
        try {
            void* value = block(shared_from_this());
            if (is_coroutine_suspended(value)) {
                std::lock_guard<std::mutex> lock(ownership_mutex_);
                if (!completed_) suspended_owner_ = shared_from_this();
            } else {
                release_intercepted();
            }
            return value;
        } catch (...) {
            release_intercepted();
            throw;
        }
    }
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:51-53,109-115
    // NOTE(port): Release C++ capture/projection roots as Native GC does at termination.
    void release_intercepted() override {
        std::lock_guard<std::mutex> lock(ownership_mutex_);
        RestrictedContinuationImpl::release_intercepted();
        completed_ = true;
        argument_owner_.reset();
        block_ = nullptr;
        suspended_owner_.reset();
    }
private:
    std::mutex ownership_mutex_;
    bool completed_ = false;
    std::shared_ptr<void> argument_owner_;
    ErasedSuspendFunction block_;
    std::shared_ptr<BaseContinuationImpl> suspended_owner_;
};
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:321-324
class SimpleContinuation final : public ContinuationImpl {
public:
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:321-324
    explicit SimpleContinuation(std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)) {}
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:321-324
    void* invoke_suspend(Result<void*> result) override { return result.get_or_throw(); }
    // NOTE(port): Per-invocation typed argument ownership; copying the callable
    // never shares a mutable continuation argument between fresh computations.
    void retain_argument(std::shared_ptr<void> argument) {
        std::lock_guard<std::mutex> lock(ownership_mutex_);
        argument_owner_ = std::move(argument);
    }
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:26-34,321-324
    // NOTE(port): Native GC retains the implicit continuation and typed projection.
    // Own the actual frame only while suspended; an unstarted wrapper has no root.
    void* invoke(ErasedSuspendFunction block) {
        { std::lock_guard<std::mutex> lock(ownership_mutex_); block_ = block; }
        try {
            void* value = block(shared_from_this());
            if (is_coroutine_suspended(value)) {
                std::lock_guard<std::mutex> lock(ownership_mutex_);
                if (!completed_) suspended_owner_ = shared_from_this();
            } else {
                release_intercepted();
            }
            return value;
        } catch (...) {
            release_intercepted();
            throw;
        }
    }
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:51-53,109-115
    // NOTE(port): Release C++ capture/projection roots as Native GC does at termination.
    void release_intercepted() override {
        std::lock_guard<std::mutex> lock(ownership_mutex_);
        ContinuationImpl::release_intercepted();
        completed_ = true;
        argument_owner_.reset();
        block_ = nullptr;
        suspended_owner_.reset();
    }
private:
    std::mutex ownership_mutex_;
    bool completed_ = false;
    std::shared_ptr<void> argument_owner_;
    ErasedSuspendFunction block_;
    std::shared_ptr<BaseContinuationImpl> suspended_owner_;
};
} // namespace

// NOTE(port): Ownership bridge for the typed argument of the actual Native simple wrapper.
void retain_suspend_argument(const std::shared_ptr<Continuation<void*>>& frame, std::shared_ptr<void> argument) {
    if (auto restricted = std::dynamic_pointer_cast<RestrictedSimpleContinuation>(frame))
        restricted->retain_argument(std::move(argument));
    else
        std::static_pointer_cast<SimpleContinuation>(frame)->retain_argument(std::move(argument));
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:221-263
std::shared_ptr<Continuation<void*>> create_coroutine_from_suspend_function(
    std::shared_ptr<Continuation<void*>> completion, ErasedSuspendFunction block) {
    auto context = completion->get_context();
    if (context == EmptyCoroutineContext::instance())
        return std::make_shared<RestrictedCreatedContinuation>(std::move(completion), std::move(block));
    return std::make_shared<CreatedContinuation>(std::move(completion), std::move(block));
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:142-152
std::shared_ptr<Continuation<void*>> create_coroutine_unintercepted(
    ErasedSuspendFunction block, std::shared_ptr<Continuation<void*>> completion) {
    auto probe_completion = kotlin::coroutines::native::internal::probe_coroutine_created(std::move(completion));
    // NOTE(port): std::function is the callable-reference branch; generated frame
    // prototypes use the BaseContinuationImpl overload and their actual create().
    return create_coroutine_from_suspend_function(std::move(probe_completion),
        [block = std::move(block)](std::shared_ptr<Continuation<void*>> frame) {
            return start_coroutine_unintercepted_or_return(block, std::move(frame));
        });
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:142-152
std::shared_ptr<Continuation<void*>> create_coroutine_unintercepted(
    BaseContinuationImpl& block, std::shared_ptr<Continuation<void*>> completion) {
    return block.create(kotlin::coroutines::native::internal::probe_coroutine_created(std::move(completion)));
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:177-189
std::shared_ptr<Continuation<void*>> create_coroutine_unintercepted(
    BaseContinuationImpl& block, void* receiver, std::shared_ptr<Continuation<void*>> completion) {
    return block.create(receiver, kotlin::coroutines::native::internal::probe_coroutine_created(std::move(completion)));
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:296-324
std::shared_ptr<Continuation<void*>> wrap_with_continuation_impl(
    std::shared_ptr<Continuation<void*>> completion) {
    auto probe_completion = kotlin::coroutines::native::internal::probe_coroutine_created(std::move(completion));
    if (probe_completion->get_context() == EmptyCoroutineContext::instance())
        return std::make_shared<RestrictedSimpleContinuation>(std::move(probe_completion));
    return std::make_shared<SimpleContinuation>(std::move(probe_completion));
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:26-34,296-324
void* start_coroutine_unintercepted_or_return(
    ErasedSuspendFunction block, std::shared_ptr<Continuation<void*>> completion) {
    auto wrapped_completion = wrap_with_continuation_impl(std::move(completion));
    if (auto restricted = std::dynamic_pointer_cast<RestrictedSimpleContinuation>(wrapped_completion))
        return restricted->invoke(std::move(block));
    return std::static_pointer_cast<SimpleContinuation>(wrapped_completion)->invoke(std::move(block));
}
} // namespace kotlin::coroutines::intrinsics
