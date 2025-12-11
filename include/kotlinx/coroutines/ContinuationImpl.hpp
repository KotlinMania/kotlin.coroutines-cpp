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
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Result.hpp"

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
 */
class BaseContinuationImpl : public Continuation<void*>,
                              public std::enable_shared_from_this<BaseContinuationImpl> {
public:
    std::shared_ptr<Continuation<void*>> completion;

    explicit BaseContinuationImpl(std::shared_ptr<Continuation<void*>> completion_)
        : completion(std::move(completion_)) {}

    virtual ~BaseContinuationImpl() = default;

    void resume_with(Result<void*> result) override final {
        BaseContinuationImpl* current = this;
        Result<void*> param = std::move(result);

        while (true) {
            auto* completion_ptr = current->completion.get();
            if (!completion_ptr) {
                // throw std::runtime_error("Trying to resume continuation without completion");
                // Or just return if top level?
                return;
            }

            Result<void*> outcome;
            try {
                void* suspend_result = current->invoke_suspend(param);

                if (intrinsics::is_coroutine_suspended(suspend_result)) {
                    return; 
                }

                outcome = Result<void*>::success(suspend_result);
            } catch (...) {
                outcome = Result<void*>::failure(std::current_exception());
            }

            current->release_intercepted();

            auto* base_completion = dynamic_cast<BaseContinuationImpl*>(completion_ptr);
            if (base_completion) {
                current = base_completion;
                param = std::move(outcome);
            } else {
                completion_ptr->resume_with(std::move(outcome));
                return;
            }
        }
    }

    virtual void* invoke_suspend(Result<void*> result) = 0;

    virtual void release_intercepted() {}

    virtual std::shared_ptr<Continuation<void>> create(
        std::shared_ptr<Continuation<void*>> /*completion*/
    ) {
        throw std::runtime_error("create(Continuation) has not been overridden");
    }

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

class RestrictedContinuationImpl : public BaseContinuationImpl {
public:
    explicit RestrictedContinuationImpl(std::shared_ptr<Continuation<void*>> completion)
        : BaseContinuationImpl(std::move(completion)) {}

    std::shared_ptr<CoroutineContext> get_context() const override {
        return EmptyCoroutineContext::instance();
    }
};

class ContinuationImpl : public BaseContinuationImpl {
public:
    ContinuationImpl(
        std::shared_ptr<Continuation<void*>> completion,
        std::shared_ptr<CoroutineContext> context
    ) : BaseContinuationImpl(std::move(completion)),
        context_(std::move(context)) {}

    explicit ContinuationImpl(std::shared_ptr<Continuation<void*>> completion)
        : BaseContinuationImpl(completion),
          context_(completion ? completion->get_context() : nullptr) {}

    std::shared_ptr<CoroutineContext> get_context() const override {
        return context_ ? context_ : EmptyCoroutineContext::instance();
    }

    std::shared_ptr<Continuation<void*>> intercepted() {
        if (!intercepted_) {
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
        intercepted_ = nullptr;
    }

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

    void resume_with(kotlinx::coroutines::Result<void*> /*result*/) override {
        throw std::runtime_error("This continuation is already complete");
    }
};

} // namespace coroutines
} // namespace kotlinx
