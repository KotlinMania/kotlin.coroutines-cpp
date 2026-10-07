// port-lint: source kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt
#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.intrinsics
 *
 * Concrete failure helpers and the already-created continuation entry live in
 * Cancellable.cpp. Public generic callable entries are instantiated in this header.
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <functional>
#include <memory>
#include "kotlinx/coroutines/intrinsics/IntrinsicsNative.hpp"

namespace kotlinx::coroutines::intrinsics {

// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:52-64
void dispatcher_failure(Continuation<void*>* completion, std::exception_ptr exception);
// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:44-50
void run_safely(Continuation<void*>* completion, std::function<void()> block);
// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:33-36
void start_coroutine_cancellable(Continuation<void*>* continuation, Continuation<void*>* fatal_completion);
// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:15-17
void start_coroutine_cancellable(ErasedSuspendFunction block, std::shared_ptr<Continuation<void*>> completion);
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:112-116
void start_coroutine(ErasedSuspendFunction block, std::shared_ptr<Continuation<void*>> completion);
// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Undispatched.kt:13-31
void start_coroutine_undispatched(ErasedSuspendFunction block, std::shared_ptr<Continuation<void*>> completion);

// NOTE(port): The raw completion ABI borrows. Only a real frame's existing owner
// can be recovered; shared_ptr overloads explicitly retain other owned completions.
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:16-27
template <typename T>
std::shared_ptr<Continuation<T>> retain_start_completion(Continuation<T>* completion) {
    if constexpr (std::is_same_v<T, void*>) return internal::retain_continuation(completion);
    else return std::shared_ptr<Continuation<T>>(completion, [](Continuation<T>*) {});
}
// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:15-17
template <typename T>
void start_coroutine_cancellable(std::function<void*(Continuation<T>*)> block,
            std::shared_ptr<Continuation<T>> completion) {
    start_coroutine_cancellable(erase_suspend_function<T>(std::move(block)), to_void_continuation(std::move(completion)));
}
// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:15-17
template <typename T>
void start_coroutine_cancellable(std::function<void*(Continuation<T>*)> block, Continuation<T>* completion) {
    start_coroutine_cancellable<T>(std::move(block), retain_start_completion(completion));
}
// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:23-27
template <typename R, typename T>
void start_coroutine_cancellable(std::function<void*(R, Continuation<T>*)> block, R receiver,
            std::shared_ptr<Continuation<T>> completion) {
    std::function<void*(Continuation<T>*)> bound =
        [block = std::move(block), receiver = std::move(receiver)](Continuation<T>* frame) {
            return block(receiver, frame);
        };
    start_coroutine_cancellable<T>(std::move(bound), std::move(completion));
}
// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:23-27
template <typename R, typename T>
void start_coroutine_cancellable(std::function<void*(R, Continuation<T>*)> block, R receiver, Continuation<T>* completion) {
    start_coroutine_cancellable<R, T>(std::move(block), std::move(receiver), retain_start_completion(completion));
}
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:112-116
template <typename T>
void start_coroutine(std::function<void*(Continuation<T>*)> block,
            std::shared_ptr<Continuation<T>> completion) {
    start_coroutine(erase_suspend_function<T>(std::move(block)), to_void_continuation(std::move(completion)));
}
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:112-116
template <typename T>
void start_coroutine(std::function<void*(Continuation<T>*)> block, Continuation<T>* completion) {
    start_coroutine<T>(std::move(block), retain_start_completion(completion));
}
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:125-130
template <typename R, typename T>
void start_coroutine(std::function<void*(R, Continuation<T>*)> block, R receiver,
            std::shared_ptr<Continuation<T>> completion) {
    std::function<void*(Continuation<T>*)> bound =
        [block = std::move(block), receiver = std::move(receiver)](Continuation<T>* frame) {
            return block(receiver, frame);
        };
    start_coroutine<T>(std::move(bound), std::move(completion));
}
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:125-130
template <typename R, typename T>
void start_coroutine(std::function<void*(R, Continuation<T>*)> block, R receiver, Continuation<T>* completion) {
    start_coroutine<R, T>(std::move(block), std::move(receiver), retain_start_completion(completion));
}
// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Undispatched.kt:13-31
template <typename T>
void start_coroutine_undispatched(std::function<void*(Continuation<T>*)> block,
            std::shared_ptr<Continuation<T>> completion) {
    start_coroutine_undispatched(erase_suspend_function<T>(std::move(block)), to_void_continuation(std::move(completion)));
}
// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Undispatched.kt:13-31
template <typename T>
void start_coroutine_undispatched(std::function<void*(Continuation<T>*)> block, Continuation<T>* completion) {
    start_coroutine_undispatched<T>(std::move(block), retain_start_completion(completion));
}
// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Undispatched.kt:13-31
template <typename R, typename T>
void start_coroutine_undispatched(std::function<void*(R, Continuation<T>*)> block, R receiver,
            std::shared_ptr<Continuation<T>> completion) {
    std::function<void*(Continuation<T>*)> bound =
        [block = std::move(block), receiver = std::move(receiver)](Continuation<T>* frame) {
            return block(receiver, frame);
        };
    start_coroutine_undispatched<T>(std::move(bound), std::move(completion));
}
// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Undispatched.kt:13-31
template <typename R, typename T>
void start_coroutine_undispatched(std::function<void*(R, Continuation<T>*)> block, R receiver, Continuation<T>* completion) {
    start_coroutine_undispatched<R, T>(std::move(block), std::move(receiver), retain_start_completion(completion));
}
} // namespace kotlinx::coroutines::intrinsics
