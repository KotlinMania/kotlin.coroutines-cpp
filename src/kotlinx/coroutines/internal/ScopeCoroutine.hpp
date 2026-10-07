#pragma once
// port-lint: source internal/Scopes.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/internal/Scopes.kt */
#include "kotlinx/coroutines/AbstractCoroutine.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/CompletedValue.hpp"
#include "kotlinx/coroutines/CompletionState.hpp"
#include <memory>
#include <typeinfo>
#include <optional>
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/internal/CoroutineStackFrame.hpp"
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"

namespace kotlinx {
namespace coroutines {

// Forward declaration - defined in Timeout.hpp
class TimeoutCancellationException;

/**
 * Checks if the current exception is a TimeoutCancellationException from the given coroutine.
 * Returns true if it IS our own timeout (should NOT rethrow), false otherwise.
 *
 * Kotlin: private fun ScopeCoroutine<*>.notOwnTimeout(cause: Throwable): Boolean =
 *     cause !is TimeoutCancellationException || cause.coroutine !== this
 *
 * This is the inverse: returns true if it IS own timeout.
 * Implemented in Scopes.cpp to avoid circular dependency with Timeout.hpp.
 */
bool is_own_timeout_exception(std::exception_ptr ex, const void* coroutine_ptr);

namespace internal {

// Transliterated from: kotlinx-coroutines-core/common/src/internal/Scopes.kt:11-14
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/SafeCollector.common.kt:92-97
// NOTE(port): C++ template specializations share this erased type identity for
// Kotlin's ScopeCoroutine<*> check. Only actual ScopeCoroutine construction can create it.
class ScopeCoroutineBase {
public:
    virtual ~ScopeCoroutineBase() = default;
private:
    template <typename T> friend class ScopeCoroutine;
    ScopeCoroutineBase() = default;
};

/**
 * This is a coroutine instance that is created by [coroutineScope] builder.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/internal/Scopes.kt:11-37
template <typename T>
class ScopeCoroutine : public AbstractCoroutine<T>, public CoroutineStackFrame, public ScopeCoroutineBase {
public:
    std::shared_ptr<Continuation<T>> u_cont; // unintercepted continuation

    // Transliterated from: kotlinx-coroutines-core/common/src/internal/Scopes.kt:11-14
    ScopeCoroutine(std::shared_ptr<CoroutineContext> context, std::shared_ptr<Continuation<T>> uCont)
        : AbstractCoroutine<T>(context, true, true), u_cont(uCont) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/internal/Scopes.kt:16-17
    CoroutineStackFrame* get_caller_frame() const override {
        if (auto boxed = std::dynamic_pointer_cast<ResultBoxCompletion<T>>(u_cont))
            return dynamic_cast<CoroutineStackFrame*>(boxed->completion().get());
        return dynamic_cast<CoroutineStackFrame*>(u_cont.get());
    }
    // Transliterated from: kotlinx-coroutines-core/common/src/internal/Scopes.kt:17-17
    StackTraceElement* get_stack_trace_element() const override { return nullptr; }

    // Transliterated from: kotlinx-coroutines-core/common/src/internal/Scopes.kt:19-19
    bool is_scoped_coroutine() const final override { return true; }

    // Transliterated from: kotlinx-coroutines-core/common/src/internal/Scopes.kt:21-24
    void after_completion(JobState* state) override {
        auto lifetime = this->JobSupport::shared_from_this();
        erased_completion_.reset();
        auto result = kotlinx::coroutines::recover_result<T>(state, u_cont.get());
        resume_cancellable_with(intercepted_u_cont(), std::move(result));
    }

    /**
     * Invoked when a scoped coroutine was completed in an undispatched manner directly
     * at the place of its start because it never suspended.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/internal/Scopes.kt:30-31
    virtual void after_completion_undispatched() {
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/internal/Scopes.kt:33-36
    void after_resume(JobState* state) override {
        // Resume direct because scope is already in the correct context
        // Kotlin: uCont.resumeWith(recoverResult(state, uCont))
        auto lifetime = this->JobSupport::shared_from_this();
        erased_completion_.reset();
        u_cont->resume_with(kotlinx::coroutines::recover_result<T>(state, u_cont.get()));
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Undispatched.kt:41-43
    void* start_undispatched_or_return(std::function<void*(Continuation<void*>*)> block) {
        return start_undspatched(true, [block = std::move(block)](std::shared_ptr<Continuation<void*>> completion) {
            return block(completion.get());
        });
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Undispatched.kt:41-43
    // NOTE(port): Preserve the actual owning completion at C++ builder boundaries.
    void* start_undispatched_or_return(std::function<void*(std::shared_ptr<Continuation<void*>>)> block) {
        return start_undspatched(true, std::move(block));
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Undispatched.kt:48-50
    void* start_undispatched_or_return_ignore_timeout(std::function<void*(Continuation<void*>*)> block) {
        return start_undspatched(false, [block = std::move(block)](std::shared_ptr<Continuation<void*>> completion) {
            return block(completion.get());
        });
    }

    // NOTE(port): Ordinary C++ block adapter; completion still waits for real child jobs.
    void* start_undispatched_or_return_ignore_timeout(
        std::shared_ptr<ScopeCoroutine<T>> receiver, std::function<T(CoroutineScope&)> block) {
        return start_undispatched_or_return_ignore_timeout(
            [receiver = std::move(receiver), block = std::move(block)](Continuation<void*>*) -> void* {
                if constexpr (std::is_same_v<T, void*>) return block(*receiver);
                else return new T(block(*receiver));
            });
    }

protected:
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:201-202
    // NOTE(port): Intercept the original erased frame, then retain its actual dispatcher for typed delivery.
    std::shared_ptr<Continuation<T>> intercepted_u_cont() {
        if constexpr (std::is_same_v<T, void*>) return intrinsics::intercepted(u_cont);
        else {
            if (auto boxed = std::dynamic_pointer_cast<ResultBoxCompletion<T>>(u_cont)) {
                auto delegate = intercepted_delegate(boxed->completion());
                if (delegate.dispatcher)
                    return std::make_shared<DispatchedContinuation<T>>(
                        delegate.dispatcher, result_box_completion<T>(std::move(delegate.continuation)));
            }
            return u_cont;
        }
    }

    // NOTE(port): Retains the erased typed adapter until completion; Kotlin GC
    // retains the continuation supplied to startCoroutineUninterceptedOrReturn.
    std::shared_ptr<Continuation<void*>> erased_completion_;

private:
    // Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Undispatched.kt:61-95
    void* start_undspatched(bool always_rethrow, std::function<void*(std::shared_ptr<Continuation<void*>>)> block) {
        this->init_parent_job_if_needed();
        auto continuation = std::dynamic_pointer_cast<Continuation<T>>(this->JobSupport::shared_from_this());
        auto erased = to_void_continuation<T>(continuation);
        if constexpr (!std::is_same_v<T, void*>) erased_completion_ = erased;
        void* result = nullptr;
        std::exception_ptr block_failure;
        try {
            result = block(erased);
        } catch (const DispatchException& error) {
            // Special codepath for failing CoroutineDispatcher: rethrow immediately
            // without waiting for children to indicate something is wrong.
            erased_completion_.reset();
            this->make_completing(new CompletedExceptionally(error.cause));
            std::rethrow_exception(error.cause);
        } catch (...) {
            block_failure = std::current_exception();
        }

        // (1) The block suspended. (2) It completed but has active children.
        // (3) It completed successfully or exceptionally, with no children left.
        if (intrinsics::is_coroutine_suspended(result)) return result;
        JobState* proposed;
        if (block_failure) proposed = new CompletedExceptionally(block_failure);
        else if constexpr (std::is_same_v<T, void*>) proposed = new CompletedValue<T>(result);
        else if constexpr (std::is_same_v<T, Unit>) {
            std::unique_ptr<Unit> value(static_cast<Unit*>(result));
            proposed = new CompletedValue<Unit>(Unit{});
        } else {
            std::unique_ptr<T> value(static_cast<T*>(result));
            proposed = new CompletedValue<T>(std::move(*value));
        }
        auto completing = this->make_completing_once(proposed);
        if (completing == JobSupport::CompletingResult::COMPLETING)
            return intrinsics::get_COROUTINE_SUSPENDED();
        erased_completion_.reset();
        after_completion_undispatched();
        auto* state = this->get_state_for_await();
        if (auto* failed = dynamic_cast<CompletedExceptionally*>(state)) {
            // Native recoverStackTrace returns the original exception.
            if (always_rethrow || !is_own_timeout_exception(failed->cause, this))
                std::rethrow_exception(failed->cause);
            if (block_failure) std::rethrow_exception(block_failure);
            state = proposed;
        }
        auto value = kotlinx::coroutines::recover_result<T>(state, u_cont.get()).get_or_throw();
        if constexpr (std::is_same_v<T, void*>) return value;
        else if constexpr (std::is_same_v<T, Unit>) return nullptr;
        else return new T(std::move(value));
    }

};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx

