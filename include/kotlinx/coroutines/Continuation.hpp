#pragma once
/**
 * @file Continuation.hpp
 * @brief Continuation interface for kotlinx.coroutines
 *
 * This file provides the Continuation<T> interface that matches Kotlin's
 * kotlin.coroutines.Continuation interface.
 */

#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include <memory>
#include <functional>

namespace kotlinx {
namespace coroutines {

// Legacy compatibility - ContinuationBase for older code
class ContinuationBase {
public:
    virtual ~ContinuationBase() = default;
};

/**
 * Interface representing a continuation after a suspension point that returns a value of type `T`.
 *
 * Transliterated from: interface Continuation<in T> in kotlin.coroutines.Continuation
 */
template<typename T>
class Continuation {
public:
    virtual ~Continuation() = default;

    /**
     * The context of the coroutine that corresponds to this continuation.
     */
    virtual std::shared_ptr<CoroutineContext> get_context() const = 0;

    /**
     * Resumes the execution of the corresponding coroutine passing a successful or failed [result]
     * as the return value of the last suspension point.
     */
    virtual void resume_with(Result<T> result) = 0;
};

/**
 * Resumes the execution of the corresponding coroutine passing [value] as the return value.
 */
template<typename T>
inline void resume(Continuation<T>& continuation, T value) {
    continuation.resume_with(Result<T>::success(std::move(value)));
}

/**
 * Specialization for void - resume with Unit.
 */
inline void resume(Continuation<void>& continuation) {
    continuation.resume_with(Result<void>::success());
}

/**
 * Resumes with an exception.
 */
template<typename T>
inline void resume_withException(Continuation<T>& continuation, std::exception_ptr exception) {
    continuation.resume_with(Result<T>::failure(exception));
}

/**
 * Functional continuation implementation.
 */
template<typename T>
class FunctionalContinuation : public Continuation<T> {
public:
    FunctionalContinuation(
        std::shared_ptr<CoroutineContext> context,
        std::function<void(Result<T>)> resume_withFn
    ) : context_(std::move(context)), resume_withFn_(std::move(resume_withFn)) {}

    std::shared_ptr<CoroutineContext> get_context() const override {
        return context_;
    }

    void resume_with(Result<T> result) override {
        resume_withFn_(std::move(result));
    }

private:
    std::shared_ptr<CoroutineContext> context_;
    std::function<void(Result<T>)> resume_withFn_;
};

/**
 * Factory function for creating functional continuations.
 */
template<typename T>
std::shared_ptr<Continuation<T>> make_continuation(
    std::shared_ptr<CoroutineContext> context,
    std::function<void(Result<T>)> resume_withFn
) {
    return std::make_shared<FunctionalContinuation<T>>(
        std::move(context),
        std::move(resume_withFn)
    );
}

/**
 * Empty coroutine context singleton.
 */
class EmptyCoroutineContext : public CoroutineContext {
public:
    static std::shared_ptr<EmptyCoroutineContext> instance() {
        static auto instance_ = std::make_shared<EmptyCoroutineContext>();
        return instance_;
    }

    std::shared_ptr<Element> get(Key* /*key*/) const override {
        return nullptr;
    }
};

} // namespace coroutines
} // namespace kotlinx

// Alias to kotlin namespace for compatibility with transliterated code
namespace kotlin {
namespace coroutines {

template<typename T>
using Continuation = ::kotlinx::coroutines::Continuation<T>;

using EmptyCoroutineContext = ::kotlinx::coroutines::EmptyCoroutineContext;

template<typename T>
using FunctionalContinuation = ::kotlinx::coroutines::FunctionalContinuation<T>;

template<typename T>
using Result = ::kotlinx::coroutines::Result<T>;

using CoroutineContext = ::kotlinx::coroutines::CoroutineContext;

template<typename T>
inline void resume(Continuation<T>& c, T value) {
    ::kotlinx::coroutines::resume(c, std::move(value));
}

inline void resume(Continuation<void>& c) {
    ::kotlinx::coroutines::resume(c);
}

template<typename T>
inline void resume_withException(Continuation<T>& c, std::exception_ptr e) {
    ::kotlinx::coroutines::resume_withException(c, e);
}

template<typename T>
std::shared_ptr<Continuation<T>> make_continuation(
    std::shared_ptr<CoroutineContext> context,
    std::function<void(Result<T>)> resume_withFn
) {
    return ::kotlinx::coroutines::make_continuation<T>(
        std::move(context),
        std::move(resume_withFn)
    );
}

} // namespace coroutines
} // namespace kotlin
