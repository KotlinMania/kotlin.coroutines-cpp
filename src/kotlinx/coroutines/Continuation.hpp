// port-lint: source libraries/stdlib/src/kotlin/coroutines/Continuation.kt
#pragma once
/**
 * Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt
 * @file Continuation.hpp
 * @brief Continuation interface for kotlinx.coroutines
 *
 * This file provides the Continuation<T> interface that matches Kotlin's
 * kotlin.coroutines.Continuation interface.
 */

#include "kotlinx/coroutines/CoroutineImports.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/Unit.hpp"
#include <memory>
#include <functional>

namespace kotlin {
namespace coroutines {
// NOTE(port): Existing C++ Result/Unit carriers remain explicit dependencies.
using kotlinx::coroutines::Result;
using kotlinx::coroutines::Unit;

// Legacy compatibility - ContinuationBase for older code
class ContinuationBase {
public:
    virtual ~ContinuationBase() = default;
};

/**
 * Interface representing a continuation after a suspension point that returns a value of type `T`.
 */
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:16-27
template <typename T>
class Continuation {
public:
    virtual ~Continuation() = default;
/**
     * The context of the coroutine that corresponds to this continuation.
     */
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:20-20
    virtual std::shared_ptr<CoroutineContext> get_context() const = 0;
/**
     * Resumes the execution of the corresponding coroutine passing a successful or failed [result] as the
     * return value of the last suspension point.
     */
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:26-26
    virtual void resume_with(Result<T> result) = 0;
};

/**
 * Resumes the execution of the corresponding coroutine passing [value] as the return value of the last suspension point.
 */
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:44-45
template <typename T>
inline void resume(Continuation<T>& continuation, T value) {
    continuation.resume_with(Result<T>::success(std::move(value)));
}

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:44-45
// NOTE(port): C++ void represents Unit at an ordinary C++ boundary.
inline void resume(Continuation<void>& continuation) {
    continuation.resume_with(Result<void>::success());
}

/**
 * Resumes the execution of the corresponding coroutine so that the [exception] is re-thrown right after the
 * last suspension point.
 */
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:53-54
template <typename T>
inline void resume_with_exception(Continuation<T>& continuation, std::exception_ptr exception) {
    continuation.resume_with(Result<T>::failure(exception));
}

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:66-72
// NOTE(port): Named projection of the source factory's anonymous Continuation object.
template <typename T>
class FunctionalContinuation : public Continuation<T> {
public:
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:62-67
    FunctionalContinuation(std::shared_ptr<CoroutineContext> context,
                           std::function<void(Result<T>)> resume_with)
        : context_(std::move(context)), resume_with_(std::move(resume_with)) {}
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:67-68
    std::shared_ptr<CoroutineContext> get_context() const override { return context_; }
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:70-71
    void resume_with(Result<T> result) override { resume_with_(std::move(result)); }
private:
    std::shared_ptr<CoroutineContext> context_;
    std::function<void(Result<T>)> resume_with_;
};

/**
 * Creates a [Continuation] instance with the given [context] and implementation of [resumeWith] method.
 */
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:62-72
template <typename T>
std::shared_ptr<Continuation<T>> make_continuation(
    std::shared_ptr<CoroutineContext> context, std::function<void(Result<T>)> resume_with) {
    return std::make_shared<FunctionalContinuation<T>>(std::move(context), std::move(resume_with));
}

/**
 * Adapter that type-erases any Continuation<T> into a Continuation<void*>.
 */
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:16-27,62-72
// NOTE(port): Typed Continuation projection at the existing erased suspend ABI boundary.
class ContinuationVoidAdapter : public Continuation<void*> {
    std::function<std::shared_ptr<CoroutineContext>()> get_context_fn_;
    std::function<void(Result<void*>)> resume_fn_;
public:
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:62-72
    template<typename T>
    explicit ContinuationVoidAdapter(std::shared_ptr<Continuation<T>> cont) {
        get_context_fn_ = [cont]() { return cont->get_context(); };
        resume_fn_ = [cont](Result<void*> res) {
            if (res.is_failure()) {
                cont->resume_with(Result<T>::failure(res.exception_or_null()));
            } else {
                if constexpr (std::is_same_v<T, void*>) {
                    cont->resume_with(Result<void*>::success(res.get_or_throw()));
                } else if constexpr (std::is_void_v<T>) {
                    cont->resume_with(Result<void>::success());
                } else if constexpr (std::is_same_v<T, Unit>) {
                    // NOTE(port): Unit uses nullptr in the erased suspend ABI.
                    std::unique_ptr<Unit> box(static_cast<Unit*>(res.get_or_throw()));
                    cont->resume_with(Result<T>::success(Unit{}));
                } else {
                    // NOTE(port): The receiving adapter owns unbox and deletion.
                    std::unique_ptr<T> box(static_cast<T*>(res.get_or_throw()));
                    cont->resume_with(Result<T>::success(std::move(*box)));
                }
            }
        };
    }

    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:67-68
    std::shared_ptr<CoroutineContext> get_context() const override {
        return get_context_fn_();
    }

    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:70-71
    void resume_with(Result<void*> result) override { resume_fn_(std::move(result)); }
};

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:62-72
// NOTE(port): Preserve erased continuation identity; adapt only a different value type.
template<typename T>
inline std::shared_ptr<Continuation<void*>> to_void_continuation(std::shared_ptr<Continuation<T>> cont) {
    if constexpr (std::is_same_v<T, void*>) {
        return cont;
    } else {
        return std::make_shared<ContinuationVoidAdapter>(cont);
    }
}

namespace internal {
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:16-27,62-72
// NOTE(port): Retain the original compiler continuation for interception and stack-frame identity.
// Typed values stay owned until delivery; the erased caller owns the resulting box.
template <typename T>
class ResultBoxCompletion final : public Continuation<T> {
public:
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:62-72
    explicit ResultBoxCompletion(std::shared_ptr<Continuation<void*>> completion)
        : completion_(std::move(completion)) {}
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:67-68
    std::shared_ptr<CoroutineContext> get_context() const override { return completion_->get_context(); }
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:70-71
    void resume_with(Result<T> result) override {
        if (result.is_failure()) completion_->resume_with(Result<void*>::failure(result.exception_or_null()));
        else if constexpr ((std::is_same_v<T, Unit> || std::is_void_v<T>)) completion_->resume_with(Result<void*>::success(nullptr));
        else completion_->resume_with(Result<void*>::success(new T(result.get_or_throw())));
    }
    // NOTE(port): Access the actual unintercepted compiler frame behind the typed ABI projection.
    const std::shared_ptr<Continuation<void*>>& completion() const { return completion_; }
private:
    std::shared_ptr<Continuation<void*>> completion_;
};

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:62-72
template <typename T>
std::shared_ptr<Continuation<T>> result_box_completion(std::shared_ptr<Continuation<void*>> completion) {
    if constexpr (std::is_same_v<T, void*>) return completion;
    else return std::make_shared<ResultBoxCompletion<T>>(std::move(completion));
}
} // namespace internal

} // namespace coroutines
} // namespace kotlin


#include "kotlinx/coroutines/ContinuationImports.hpp"
