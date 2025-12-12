#pragma once

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <functional>
#include <memory>
#include <stdexcept>

namespace kotlinx {
namespace coroutines {
namespace intrinsics {

// Forward decl
void dispatcher_failure(std::shared_ptr<Continuation<void*>> completion, std::exception_ptr e);

/**
 * Runs given block and completes completion with its exception if it occurs.
 */
template <typename T>
inline void run_safely(std::shared_ptr<Continuation<T>> completion, std::function<void()> block) {
    try {
        block();
    } catch (...) {
        completion->resume_with(Result<T>::failure(std::current_exception()));
    }
}
/**
 * Adapter to allow proper type erasure for BaseContinuationImpl.
 * Wraps a Continuation<T> and presents it as Continuation<void*>.
 * Performs crude casting/unboxing - assumes compatible value representation.
 */
template<typename T>
class TypeErasureAdapter : public Continuation<void*> {
    std::shared_ptr<Continuation<T>> target_;
public:
    explicit TypeErasureAdapter(std::shared_ptr<Continuation<T>> target) 
        : target_(std::move(target)) {}

    std::shared_ptr<CoroutineContext> get_context() const override {
        return target_->get_context();
    }

    void resume_with(Result<void*> result) override {
        if (result.is_success()) {
            // Unbox/Cast void* to T
            void* ptr = result.get_or_throw();
            if constexpr (std::is_pointer_v<T>) {
                target_->resume_with(Result<T>::success(static_cast<T>(ptr)));
            } else if constexpr (std::is_same_v<T, int>) {
                 // Special case for tests boxing int
                target_->resume_with(Result<T>::success(static_cast<T>(reinterpret_cast<intptr_t>(ptr))));
            } else {
                 // Fallback reinterpret
                 // This is technically unsafe but allows compilation for now.
                 // Ideally we need a trait for Unboxing strategy.
                 target_->resume_with(Result<T>::success(reinterpret_cast<T>(ptr))); // Error if T is not castable
            }
        } else {
            target_->resume_with(Result<T>::failure(result.exception_or_null()));
        }
    }
};

// Void specialization - pass through
template<>
class TypeErasureAdapter<void*> : public Continuation<void*> {
    std::shared_ptr<Continuation<void*>> target_;
public:
    explicit TypeErasureAdapter(std::shared_ptr<Continuation<void*>> target) 
        : target_(std::move(target)) {}

    std::shared_ptr<CoroutineContext> get_context() const override {
        return target_->get_context();
    }

    void resume_with(Result<void*> result) override {
        target_->resume_with(std::move(result));
    }
};

template <typename T>
std::shared_ptr<Continuation<void*>> make_erased(std::shared_ptr<Continuation<T>> c) {
    if constexpr (std::is_same_v<T, void*>) {
        return c;
    } else {
        return std::make_shared<TypeErasureAdapter<T>>(c);
    }
}

// ------------------------------------------------------------------
// Internal Helpers for createCoroutineUnintercepted equivalent
// ------------------------------------------------------------------

// A simple ContinuationImpl that runs a block when resumed.
// This mimics the state machine creation for a simple suspend lambda.
template <typename T>
class LambdaContinuation : public ContinuationImpl {
    std::function<void*(Continuation<T>*)> block_;
    std::shared_ptr<Continuation<T>> completion_;

public:
    LambdaContinuation(
        std::function<void*(Continuation<T>*)> block,
        std::shared_ptr<Continuation<T>> completion
    ) : ContinuationImpl(make_erased(completion), completion->get_context()),
        block_(block), completion_(completion) {}

    void* invoke_suspend(Result<void*> result) override {
        // When started, we ignore the result (it's implicit Unit/void)
        // and just call the block with the completion continuation.
        if (result.is_failure()) {
            completion_->resume_with(Result<T>::failure(result.exception_or_null()));
            return nullptr;
        }
        return block_(completion_.get());
    }
};

// Helper concept to detect if a type has a create(completion) method
template <typename T, typename CompletionType>
concept HasCreate = requires(T t, std::shared_ptr<CompletionType> c) {
    { t.create(c) } -> std::convertible_to<std::shared_ptr<Continuation<void*>>>;
};

/**
 * Creates an unintercepted coroutine for the given suspend block.
 *
 * Implements the logic from IntrinsicsNative.kt:
 * If the block has a create() method (simulating a compiler-generated BaseContinuationImpl),
 * it uses that factory. Otherwise, it wraps the block in a LambdaContinuation.
 */
template <typename Functor, typename T>
std::shared_ptr<Continuation<void*>> create_coroutine_unintercepted(
    Functor&& block,
    std::shared_ptr<Continuation<T>> completion
) {
    if constexpr (HasCreate<Functor, Continuation<T>>) {
        return block.create(completion);
    } else {
        // Fallback: wrap in runtime continuation
        // TODO: Deprecate this path. All suspend functions should be compiled with the Clang plugin
        // which generates a state machine with a create() method.
        return std::make_shared<LambdaContinuation<T>>(std::forward<Functor>(block), completion);
    }
}

// A generic ContinuationImpl for suspend(R) -> T
template <typename R, typename T>
class ReceiverLambdaContinuation : public ContinuationImpl {
    std::function<void*(R, Continuation<T>*)> block_;
    R receiver_;
    std::shared_ptr<Continuation<T>> completion_;

public:
    ReceiverLambdaContinuation(
        std::function<void*(R, Continuation<T>*)> block,
        R receiver,
        std::shared_ptr<Continuation<T>> completion
    ) : ContinuationImpl(make_erased(completion), completion->get_context()),
        block_(block), receiver_(receiver), completion_(completion) {}

    void* invoke_suspend(Result<void*> result) override {
        if (result.is_failure()) {
            completion_->resume_with(Result<T>::failure(result.exception_or_null()));
            return nullptr;
        }
        return block_(receiver_, completion_.get());
    }
};

/**
 * Creates an unintercepted coroutine for the given suspend block with receiver.
 */
template <typename Functor, typename R, typename T>
std::shared_ptr<Continuation<void*>> create_coroutine_unintercepted(
    Functor&& block,
    R receiver,
    std::shared_ptr<Continuation<T>> completion
) {
    // Note: Receiver support for the "IR hook" path (create(receiver, completion))
    // can be added here if/when needed using a similar HasCreateReceiver concept.
    // For now, we default to the runtime wrapper for receivers as the primary use case
    // in current tests is simple functions.
    return std::make_shared<ReceiverLambdaContinuation<R, T>>(std::forward<Functor>(block), receiver, completion);
}

// ------------------------------------------------------------------
// startCoroutineCancellable
// ------------------------------------------------------------------

/**
 * Use this function to start coroutine in a cancellable way.
 * Supports both std::function and compiler-generated state machine factories.
 */
template <typename T, typename Functor>
void start_coroutine_cancellable(
    Functor&& block,
    std::shared_ptr<Continuation<T>> completion
) {
    run_safely<T>(completion, [&]() {
        // createCoroutineUnintercepted(completion)
        // Uses the appropriate factory (IR hook or runtime wrapper)
        auto coroutine = create_coroutine_unintercepted(std::forward<Functor>(block), completion);

        // .intercepted() - managed by ContinuationImpl
        auto intercepted = std::dynamic_pointer_cast<ContinuationImpl>(coroutine)->intercepted();

        // .resumeCancellableWith(Result.success(Unit))
        // We need to access resumeCancellableWith. In Kotlin it's an extension on Continuation.
        // It's checked if it's DispatchedContinuation.

        if (auto dc = std::dynamic_pointer_cast<internal::DispatchedContinuation<void*>>(intercepted)) {
             // Result.success(Unit) -> success(nullptr)
             dc->resume_cancellable_with(Result<void*>::success(nullptr));
        } else {
             intercepted->resume_with(Result<void*>::success(nullptr));
        }
    });
}

/**
 * Use this function to start coroutine in a cancellable way (with receiver).
 */
template <typename R, typename T, typename Functor>
void start_coroutine_cancellable(
    Functor&& block,
    R receiver,
    std::shared_ptr<Continuation<T>> completion
) {
    run_safely<T>(completion, [&]() {
        auto coroutine = create_coroutine_unintercepted(std::forward<Functor>(block), receiver, completion);
        auto intercepted = std::dynamic_pointer_cast<ContinuationImpl>(coroutine)->intercepted();

        if (auto dc = std::dynamic_pointer_cast<internal::DispatchedContinuation<void*>>(intercepted)) {
             dc->resume_cancellable_with(Result<void*>::success(nullptr));
        } else {
             intercepted->resume_with(Result<void*>::success(nullptr));
        }
    });
}

} // namespace intrinsics
} // namespace coroutines
} // namespace kotlinx
