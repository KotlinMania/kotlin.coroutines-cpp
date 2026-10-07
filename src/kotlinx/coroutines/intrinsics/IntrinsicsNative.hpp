// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt
/** Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt */
#pragma once
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include <functional>

namespace kotlin::coroutines::native::internal {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/DebugProbes.kt:44-46
inline std::shared_ptr<kotlinx::coroutines::Continuation<void*>> probe_coroutine_created(
    std::shared_ptr<kotlinx::coroutines::Continuation<void*>> completion) { return completion; }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/DebugProbes.kt:58-60
void probe_coroutine_resumed(kotlinx::coroutines::Continuation<void*>* frame);
}

namespace kotlinx::coroutines::intrinsics {
// NOTE(port): A lowered C++ callable receives the actual Native-style frame owner.
using ErasedSuspendFunction = std::function<void*(std::shared_ptr<Continuation<void*>>)>;
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:221-263
std::shared_ptr<Continuation<void*>> create_coroutine_from_suspend_function(
    std::shared_ptr<Continuation<void*>> completion, ErasedSuspendFunction block);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:142-152
std::shared_ptr<Continuation<void*>> create_coroutine_unintercepted(
    ErasedSuspendFunction block, std::shared_ptr<Continuation<void*>> completion);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:142-152
std::shared_ptr<Continuation<void*>> create_coroutine_unintercepted(
    BaseContinuationImpl& block, std::shared_ptr<Continuation<void*>> completion);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:177-189
std::shared_ptr<Continuation<void*>> create_coroutine_unintercepted(
    BaseContinuationImpl& block, void* receiver, std::shared_ptr<Continuation<void*>> completion);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:26-34,296-324
void* start_coroutine_unintercepted_or_return(
    ErasedSuspendFunction block, std::shared_ptr<Continuation<void*>> completion);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:296-324
std::shared_ptr<Continuation<void*>> wrap_with_continuation_impl(
    std::shared_ptr<Continuation<void*>> completion);

// NOTE(port): The implicit Native wrapper owns its typed ABI argument through suspension.
void retain_suspend_argument(const std::shared_ptr<Continuation<void*>>& frame, std::shared_ptr<void> argument);

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:26-34,61-72
// NOTE(port): Retain the typed ABI projection with its callable through suspension.
// The receiving ContinuationVoidAdapter owns unboxing and deletion of result boxes.
template <typename T>
ErasedSuspendFunction erase_suspend_function(std::function<void*(Continuation<T>*)> block) {
    return [block = std::move(block)](std::shared_ptr<Continuation<void*>> frame) {
        auto projection = internal::result_box_completion<T>(frame);
        if constexpr (!std::is_same_v<T, void*>) retain_suspend_argument(frame, projection);
        return block(projection.get());
    };
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:142-152
template <typename T>
std::shared_ptr<Continuation<void*>> create_coroutine_unintercepted(
    std::function<void*(Continuation<T>*)> block, std::shared_ptr<Continuation<T>> completion) {
    return create_coroutine_unintercepted(erase_suspend_function<T>(std::move(block)),
                                         to_void_continuation(std::move(completion)));
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:177-189
template <typename R, typename T>
std::shared_ptr<Continuation<void*>> create_coroutine_unintercepted(
    std::function<void*(R, Continuation<T>*)> block, R receiver,
    std::shared_ptr<Continuation<T>> completion) {
    std::function<void*(Continuation<T>*)> bound =
        [block = std::move(block), receiver = std::move(receiver)](Continuation<T>* frame) {
            return block(receiver, frame);
        };
    return create_coroutine_unintercepted<T>(std::move(bound), std::move(completion));
}
} // namespace kotlinx::coroutines::intrinsics
