/**
 * @file CoroutineStateMachine.hpp
 * @brief Base infrastructure for coroutine state machines using computed gotos.
 *
 * This file provides the foundation for state machines that match Kotlin/Native's
 * pattern using void* labels and computed gotos (indirectbr in LLVM IR).
 *
 * The Clang plugin (KotlinxSuspendPlugin) generates subclasses of these templates.
 * For library suspend functions (yield, delay, await), we use manual implementations.
 *
 * Pattern from Kotlin/Native:
 *   val labelField = coroutineClass.addField("label", symbols.nativePtrType, true)
 *   // dispatch: if (label == null) goto start; else goto *label (indirectbr)
 */

#pragma once

#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <functional>
#include <memory>

namespace kotlinx {
namespace coroutines {

/**
 * SuspendLambda - Base class for suspend lambda state machines.
 *
 * Transliterated from: internal abstract class SuspendLambda in ContinuationImpl.kt
 *
 * This is similar to ContinuationImpl but adds the arity property for
 * distinguishing different lambda types.
 */
class SuspendLambda : public ContinuationImpl {
public:
    int arity_;  // Number of parameters the original lambda takes

    SuspendLambda(int arity, std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)), arity_(arity) {}

    int get_arity() const { return arity_; }
};

/**
 * RestrictedSuspendLambda - Base for suspend lambdas in restricted contexts.
 *
 * Transliterated from: internal abstract class RestrictedSuspendLambda in ContinuationImpl.kt
 */
class RestrictedSuspendLambda : public RestrictedContinuationImpl {
public:
    int arity_;

    RestrictedSuspendLambda(int arity, std::shared_ptr<Continuation<void*>> completion)
        : RestrictedContinuationImpl(std::move(completion)), arity_(arity) {}

    int get_arity() const { return arity_; }
};

/**
 * Helper macro for state machine computed goto dispatch.
 *
 * This pattern matches Kotlin/Native's IrSuspendableExpression lowering:
 *   if (_label == nullptr) goto __kxs_start;
 *   goto *_label;  // computed goto (indirectbr in LLVM)
 *
 * Usage:
 *   void* invoke_suspend(Result<void*> result) override {
 *       KXS_COROUTINE_DISPATCH(_label);
 *   __kxs_start:
 *       // normal execution...
 *       _label = &&__kxs_resume0;
 *       { ... suspend call ... }
 *   __kxs_resume0:
 *       // after first suspension...
 *   }
 */
#if defined(__GNUC__) || defined(__clang__)
    // GCC/Clang support labels-as-values extension
    #define KXS_COROUTINE_DISPATCH(label) \
        if ((label) == nullptr) goto __kxs_start; \
        goto *(label);

    #define KXS_SUSPEND_POINT(label_field, resume_label) \
        (label_field) = &&resume_label;

    #define KXS_HAS_COMPUTED_GOTO 1
#else
    // Fallback for compilers without computed goto (MSVC)
    // Uses switch/case dispatch - less efficient but portable
    #define KXS_COROUTINE_DISPATCH(label) \
        switch (reinterpret_cast<intptr_t>(label)) { \
            case 0: goto __kxs_start;

    #define KXS_SUSPEND_POINT(label_field, resume_label) \
        /* In switch mode, we'd need case labels */

    #define KXS_HAS_COMPUTED_GOTO 0
#endif

/**
 * Coroutine state machine for blocks passed to launch/async.
 *
 * This wraps a user-provided std::function and executes it in a way that
 * can properly handle suspension. Since std::function can't itself suspend
 * (it's not a suspend lambda), this handles the case where the block calls
 * suspend functions that return COROUTINE_SUSPENDED.
 *
 * Template Parameters:
 *   R - Receiver type (CoroutineScope* for launch/async)
 *   T - Return type of the block
 */
template<typename R, typename T>
class BlockStateMachine : public ContinuationImpl {
public:
    // State machine label - void* for computed goto (nativePtrType in Kotlin)
    void* _label = nullptr;

    // Spilled variables
    R receiver_;
    std::function<T(R)> block_;

    // Result storage
    Result<void*> result_;

    BlockStateMachine(
        std::function<T(R)> block,
        R receiver,
        std::shared_ptr<Continuation<void*>> completion
    ) : ContinuationImpl(std::move(completion)),
        receiver_(receiver),
        block_(std::move(block)) {}

    /**
     * invoke_suspend - The state machine entry point.
     *
     * This is where the magic happens. For simple non-suspending blocks,
     * we just run them directly. For blocks that call suspend functions,
     * those functions will return COROUTINE_SUSPENDED and we propagate it.
     *
     * The key insight: In C++ without the plugin, user blocks ARE NOT
     * transformed into state machines. They just call suspend functions.
     * Those suspend functions (yield, await, delay) ARE state machines
     * that register callbacks and return COROUTINE_SUSPENDED.
     */
    void* invoke_suspend(Result<void*> result) override {
        result_ = std::move(result);

        // Check for exception from previous resumption
        if (result_.is_failure()) {
            std::rethrow_exception(result_.exception_or_null());
        }

        // Run the block - if it contains suspend calls that actually suspend,
        // the current implementation just runs them synchronously.
        // TODO: When Clang plugin generates proper state machines, this will
        // be replaced with computed goto dispatch.
        try {
            if constexpr (std::is_void_v<T>) {
                block_(receiver_);
                return nullptr;  // Unit/void completion
            } else {
                T value = block_(receiver_);
                // Box the value (store in Result for later retrieval)
                // For now, cast to void* - proper boxing needed for non-pointer types
                if constexpr (std::is_pointer_v<T>) {
                    return static_cast<void*>(value);
                } else {
                    // Need to box non-pointer types
                    // This is a simplification - real impl needs proper boxing
                    auto* boxed = new T(std::move(value));
                    return static_cast<void*>(boxed);
                }
            }
        } catch (...) {
            std::rethrow_exception(std::current_exception());
        }
    }
};

/**
 * Helper to create and start a coroutine state machine from a block.
 *
 * This is what invoke() should use instead of running blocks directly.
 *
 * @param block The suspend block to execute
 * @param receiver The receiver (typically CoroutineScope*)
 * @param completion The continuation to resume when done
 * @return shared_ptr to the state machine (which is also a Continuation)
 */
template<typename R, typename T>
std::shared_ptr<BlockStateMachine<R, T>> create_coroutine(
    std::function<T(R)> block,
    R receiver,
    std::shared_ptr<Continuation<void*>> completion
) {
    return std::make_shared<BlockStateMachine<R, T>>(
        std::move(block),
        receiver,
        std::move(completion)
    );
}

/**
 * Start a coroutine by invoking its state machine with initial result.
 *
 * @param coroutine The state machine to start
 * @return void* - either the result value or COROUTINE_SUSPENDED
 */
template<typename R, typename T>
void* start_coroutine(std::shared_ptr<BlockStateMachine<R, T>> coroutine) {
    // Initial invocation with success(Unit) - matches Kotlin's invoke pattern
    return coroutine->invoke_suspend(Result<void*>::success(nullptr));
}

} // namespace coroutines
} // namespace kotlinx
