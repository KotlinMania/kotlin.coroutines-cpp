/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 *
 * Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt
 */

#pragma once

#include <memory>
#include <exception>
#include <stdexcept>
#include <string>

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Result.hpp"

namespace kotlinx {
namespace coroutines {

// Forward declarations
class BaseContinuationImpl;
class ContinuationImpl;
class RestrictedContinuationImpl;
class CoroutineDispatcher;

// Type alias for Any? equivalent - we use void* for type-erased values
using AnyResult = Result<void*>;

/**
 * BaseContinuationImpl - the core of coroutine machinery.
 *
 * This is the base class that all compiler-generated coroutine state machines inherit from.
 * The key method is resume_with() which runs the invoke_suspend() loop.
 *
 * Transliterated from: internal abstract class BaseContinuationImpl in ContinuationImpl.kt
 *
 * This is the base class that all compiler-generated coroutine state machines inherit from.
 * The key method is resume_with() which runs the invoke_suspend() loop.
 *
 * Transliterated from: internal abstract class BaseContinuationImpl in ContinuationImpl.kt
 */
class BaseContinuationImpl : public Continuation<void*>,
                              public std::enable_shared_from_this<BaseContinuationImpl> {
public:
    // This is `public val` so that it is private on JVM and cannot be modified by untrusted code, yet
    // it has a public getter (since even untrusted code is allowed to inspect its call stack).
    std::shared_ptr<Continuation<void*>> completion;

    explicit BaseContinuationImpl(std::shared_ptr<Continuation<void*>> completion_)
        : completion(std::move(completion_)) {}

    virtual ~BaseContinuationImpl() = default;

    // This implementation is final. This fact is used to unroll resumeWith recursion.
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:21-45
    void resume_with(Result<void*> result) override final;

    virtual void* invoke_suspend(Result<void*> result) = 0;

    // NOTE(port): Direct ABI entry returns its result to the caller rather than
    // resume_with. Release cached interception on both terminating entry paths,
    // as resume_with does, to break the ownership cycle that Kotlin GC collects.
    void* start(Result<void*> result);

    virtual void release_intercepted() {
        // does nothing here, overridden in ContinuationImpl
    }

    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:55-57
    virtual std::shared_ptr<Continuation<void*>> create(std::shared_ptr<Continuation<void*>> completion);
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:59-61
    virtual std::shared_ptr<Continuation<void*>> create(void* value, std::shared_ptr<Continuation<void*>> completion);

    std::string to_string() const {
        return "Continuation @ BaseContinuationImpl";
    }
};

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:75-88
class RestrictedContinuationImpl : public BaseContinuationImpl {
public:
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:75-85
    explicit RestrictedContinuationImpl(std::shared_ptr<Continuation<void*>> completion);

    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:87-88
    std::shared_ptr<CoroutineContext> get_context() const override;
};

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:95-115
class ContinuationImpl : public BaseContinuationImpl {
public:
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:95-98
    ContinuationImpl(
        std::shared_ptr<Continuation<void*>> completion,
        std::shared_ptr<CoroutineContext> context
    );

    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:99-99
    explicit ContinuationImpl(std::shared_ptr<Continuation<void*>> completion);

    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:101-102
    std::shared_ptr<CoroutineContext> get_context() const override;

    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:104-107
    std::shared_ptr<Continuation<void*>> intercepted();

protected:
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:109-115
    void release_intercepted() override;

private:
    std::shared_ptr<CoroutineContext> context_;
    std::shared_ptr<Continuation<void*>> intercepted_;
};

class CompletedContinuation : public kotlinx::coroutines::Continuation<void*> {
public:
    static std::shared_ptr<CompletedContinuation> instance() {
        static auto instance_ = std::make_shared<CompletedContinuation>();
        return instance_;
    }

    std::shared_ptr<kotlinx::coroutines::CoroutineContext> get_context() const override {
        throw std::runtime_error("This continuation is already complete");
    }

    void resume_with(kotlinx::coroutines::Result<void*> result) override {
        throw std::runtime_error("This continuation is already complete");
    }
};

namespace internal {
// NOTE(port): Keep typed cancellable results until dispatch cancellation checks
// have run, then box them at the Continuation<void*> ABI boundary.
struct InterceptedDelegate {
    std::shared_ptr<Continuation<void*>> continuation;
    std::shared_ptr<CoroutineDispatcher> dispatcher;
};
std::shared_ptr<Continuation<void*>> retain_continuation(Continuation<void*>* continuation);
InterceptedDelegate intercepted_delegate(std::shared_ptr<Continuation<void*>> continuation);
} // namespace internal

namespace intrinsics {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:201-202
std::shared_ptr<Continuation<void*>> intercepted(std::shared_ptr<Continuation<void*>> continuation);
} // namespace intrinsics

} // namespace coroutines
} // namespace kotlinx
