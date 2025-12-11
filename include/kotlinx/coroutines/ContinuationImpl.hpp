/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 *
 * C++ transliteration of kotlin/coroutines/native/internal/ContinuationImpl.kt
 */

#pragma once

#include <memory>
#include <exception>
#include <stdexcept>
#include <string>

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

namespace kotlinx {
namespace coroutines {

// Forward declarations
class BaseContinuationImpl;
class ContinuationImpl;
class RestrictedContinuationImpl;

// Type alias for Any? equivalent - we use void* for type-erased values
using AnyResult = Result<void*>;

/**
 * BaseContinuationImpl - the core of coroutine machinery.
 *
 * This is the base class that all compiler-generated coroutine state machines inherit from.
 * The key method is resume_with() which runs the invoke_suspend() loop.
 *
 * Transliterated from: internal abstract class BaseContinuationImpl in ContinuationImpl.kt
 */
class BaseContinuationImpl : public Continuation<void*>,
                              public std::enable_shared_from_this<BaseContinuationImpl> {
public:
    /**
     * The completion continuation - called when this coroutine completes.
     * This forms the chain of continuations (call stack).
     *
     * Transliterated from: public val completion: Continuation<Any?>?
     */
    std::shared_ptr<Continuation<void*>> completion;

    explicit BaseContinuationImpl(std::shared_ptr<Continuation<void*>> completion_)
        : completion(std::move(completion_)) {}

    virtual ~BaseContinuationImpl() = default;

    /**
     * Resumes the execution of the corresponding coroutine.
     * This is the heart of the coroutine machinery - it runs the state machine loop.
     *
     * The loop unrolls recursion to make saner stack traces on resume.
     *
     * Transliterated from: public final override fun resumeWith(result: Result<Any?>)
     */
    void resume_with(Result<void*> result) override final {
        // This loop unrolls recursion in current.resumeWith(param)
        BaseContinuationImpl* current = this;
        Result<void*> param = std::move(result);

        while (true) {
            auto* completion_ptr = current->completion.get();
            if (!completion_ptr) {
                throw std::runtime_error("Trying to resume continuation without completion");
            }

            Result<void*> outcome;
            try {
                void* suspend_result = current->invoke_suspend(param);

                // Check if coroutine suspended
                if (intrinsics::is_coroutine_suspended(suspend_result)) {
                    return;  // Suspended - exit the loop
                }

                outcome = Result<void*>::success(suspend_result);
            } catch (...) {
                outcome = Result<void*>::failure(std::current_exception());
            }

            // Release intercepted continuation (state machine instance is terminating)
            current->release_intercepted();

            // Check if completion is also a BaseContinuationImpl (unroll recursion)
            auto* base_completion = dynamic_cast<BaseContinuationImpl*>(completion_ptr);
            if (base_completion) {
                // Unrolling recursion via loop
                current = base_completion;
                param = std::move(outcome);
            } else {
                // Top-level completion reached -- invoke and return
                completion_ptr->resume_with(std::move(outcome));
                return;
            }
        }
    }

    /**
     * The state machine method - called by resume_with() to execute one step.
     * Returns the result value, or COROUTINE_SUSPENDED if suspended.
     *
     * This is overridden by compiler-generated coroutine classes.
     *
     * Transliterated from: protected abstract fun invokeSuspend(result: Result<Any?>): Any?
     */
    virtual void* invoke_suspend(Result<void*> result) = 0;

    /**
     * Release the intercepted continuation.
     * Overridden in ContinuationImpl.
     *
     * Transliterated from ContinuationImpl.kt: protected open fun releaseIntercepted()
     */
    virtual void release_intercepted() {
        // Does nothing here, overridden in ContinuationImpl
    }

    /**
     * Create a new instance of this coroutine.
     * Used for restarting coroutines.
     *
     * Transliterated from: public open fun create(completion: Continuation<*>): Continuation<Unit>
     */
    virtual std::shared_ptr<Continuation<void>> create(
        std::shared_ptr<Continuation<void*>> /*completion*/
    ) {
        throw std::runtime_error("create(Continuation) has not been overridden");
    }

    /**
     * Create a new instance with a value argument.
     *
     * Transliterated from: public open fun create(value: Any?, completion: Continuation<*>): Continuation<Unit>
     */
    virtual std::shared_ptr<Continuation<void>> create(
        void* /*value*/,
        std::shared_ptr<Continuation<void*>> /*completion*/
    ) {
        throw std::runtime_error("create(Any?, Continuation) has not been overridden");
    }

    std::string to_string() const {
        return "Continuation @ BaseContinuationImpl";
    }
};

/**
 * RestrictedContinuationImpl - for restricted suspend functions.
 * These must have EmptyCoroutineContext.
 *
 * Transliterated from: internal abstract class RestrictedContinuationImpl
 */
class RestrictedContinuationImpl : public BaseContinuationImpl {
public:
    explicit RestrictedContinuationImpl(std::shared_ptr<Continuation<void*>> completion)
        : BaseContinuationImpl(std::move(completion)) {
        // In Kotlin: require(completion.context === EmptyCoroutineContext)
        // We skip this check for simplicity, but it could be added
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        return EmptyCoroutineContext::instance();
    }
};

/**
 * ContinuationImpl - for regular suspend functions.
 * Adds context management and continuation interception.
 *
 * Transliterated from: internal abstract class ContinuationImpl
 */
class ContinuationImpl : public BaseContinuationImpl {
public:
    ContinuationImpl(
        std::shared_ptr<Continuation<void*>> completion,
        std::shared_ptr<CoroutineContext> context
    ) : BaseContinuationImpl(std::move(completion)),
        context_(std::move(context)) {}

    /**
     * Constructor that inherits context from completion.
     */
    explicit ContinuationImpl(std::shared_ptr<Continuation<void*>> completion)
        : BaseContinuationImpl(completion),
          context_(completion ? completion->get_context() : nullptr) {}

    std::shared_ptr<CoroutineContext> get_context() const override {
        if (!context_) {
            throw std::runtime_error("Context is null");
        }
        return context_;
    }

    /**
     * Get the intercepted continuation.
     * Lazily intercepts this continuation via ContinuationInterceptor.
     *
     * Transliterated from: public fun intercepted(): Continuation<Any?>
     */
    std::shared_ptr<Continuation<void*>> intercepted() {
        if (!intercepted_) {
            /*
             * TODO: STUB - Continuation interception not implemented
             *
             * Kotlin source: ContinuationImpl.intercepted() in ContinuationImpl.kt
             *
             * What's missing:
             * - Should get ContinuationInterceptor from context:
             *   val interceptor = context[ContinuationInterceptor]
             * - If interceptor exists, call: interceptor.interceptContinuation(this)
             * - This wraps the continuation in a DispatchedContinuation for proper
             *   thread/dispatcher handling
             *
             * Current behavior: Returns self without interception - all coroutines
             *   run on the calling thread regardless of dispatcher in context
             * Correct behavior: Wrap in DispatchedContinuation that dispatches
             *   resume calls through the context's dispatcher
             *
             * Dependencies:
             * - ContinuationInterceptor interface implementation
             * - DispatchedContinuation class
             * - CoroutineDispatcher.interceptContinuation()
             */
            intercepted_ = std::dynamic_pointer_cast<Continuation<void*>>(shared_from_this());
            if (!intercepted_) {
                intercepted_ = std::shared_ptr<Continuation<void*>>(
                    shared_from_this(),
                    static_cast<Continuation<void*>*>(this)
                );
            }
        }
        return intercepted_;
    }

protected:
    void release_intercepted() override {
        auto intercepted = intercepted_;
        if (intercepted && intercepted.get() != this) {
            /*
             * TODO: STUB - Intercepted continuation release not implemented
             *
             * Kotlin source: ContinuationImpl.releaseIntercepted() in ContinuationImpl.kt
             *
             * What's missing:
             * - Should call: context[ContinuationInterceptor]?.releaseInterceptedContinuation(intercepted)
             * - This allows the dispatcher to clean up any resources associated with
             *   the intercepted continuation (e.g., remove from dispatch queue)
             *
             * Current behavior: Just nullifies the pointer without notifying dispatcher
             * Correct behavior: Notify interceptor to release resources, then nullify
             *
             * Impact: Minor - may cause resource leaks in dispatchers that track continuations
             */
        }
        intercepted_ = nullptr;
    }

private:
    std::shared_ptr<CoroutineContext> context_;
    std::shared_ptr<Continuation<void*>> intercepted_;
};

/**
 * Sentinel object for completed continuations.
 *
 * Transliterated from: internal object CompletedContinuation
 */
class CompletedContinuation : public Continuation<void*> {
public:
    static std::shared_ptr<CompletedContinuation> instance() {
        static auto instance_ = std::make_shared<CompletedContinuation>();
        return instance_;
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        throw std::runtime_error("This continuation is already complete");
    }

    void resume_with(Result<void*> /*result*/) override {
        throw std::runtime_error("This continuation is already complete");
    }
};

} // namespace coroutines
} // namespace kotlinx
