#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/CompletableDeferred.kt
 *
 * Kotlin file header (translated):
 *   @file:Suppress("DEPRECATION_ERROR")
 *   package kotlinx.coroutines
 */

#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/CompletedValue.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Deferred.hpp"
#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"

#include <exception>
#include <memory>
#include <stdexcept>
#include <utility>

namespace kotlinx::coroutines {

/**
 * A [Deferred] that can be completed via public functions [complete] or [cancel][Job.cancel].
 *
 * Note that the [complete] function returns `false` when this deferred value is already complete
 * or completing, while [cancel][Job.cancel] returns `true` as long as the deferred is still
 * _cancelling_ and the corresponding exception is incorporated into the final
 * [completion exception][get_completion_exception_or_null].
 *
 * An instance of completable deferred can be created by `CompletableDeferred()` function in
 * _active_ state.
 *
 * All functions on this interface are **thread-safe** and can be safely invoked from concurrent
 * coroutines without external synchronization.
 *
 * Upstream:
 *   @OptIn(ExperimentalSubclassOptIn::class)
 *   @SubclassOptInRequired(InternalForInheritanceCoroutinesApi::class)
 *   public interface CompletableDeferred<T> : Deferred<T> {
 *       public fun complete(value: T): Boolean
 *       public fun completeExceptionally(exception: Throwable): Boolean
 *   }
 */
template <typename T>
class CompletableDeferred : public Deferred<T> {
public:
    ~CompletableDeferred() override = default;

    /**
     * Upstream: public fun complete(value: T): Boolean
     *
     * Completes this deferred value with a given [value]. The result is `true` if this deferred
     * was completed as a result of this invocation and `false` otherwise (if it was already
     * completed).
     */
    virtual bool complete(T value) = 0;

    /**
     * Upstream: public fun completeExceptionally(exception: Throwable): Boolean
     *
     * Completes this deferred value exceptionally with a given [exception]. The result is
     * `true` if this deferred was completed as a result of this invocation and `false`
     * otherwise (if it was already completed).
     */
    virtual bool complete_exceptionally(std::exception_ptr exception) = 0;
};

/**
 * Completes this deferred value with the value or exception in the given [result].
 *
 * Upstream:
 *   public fun <T> CompletableDeferred<T>.completeWith(result: Result<T>): Boolean =
 *       result.fold({ complete(it) }, { completeExceptionally(it) })
 */
template <typename T>
inline bool complete_with(CompletableDeferred<T>* deferred, Result<T> result) {
    if (result.is_success()) {
        return deferred->complete(result.get_or_throw());
    }
    return deferred->complete_exceptionally(result.exception_or_null());
}

/**
 * Active deferred backed by the job state machine. Factories register the parent
 * after shared ownership is established, before returning the deferred.
 * Successful completion stores a typed value in a polymorphic job-state box.
 */
template <typename T>
class CompletableDeferredImpl : public JobSupport, public CompletableDeferred<T> {
public:
    CompletableDeferredImpl() : JobSupport(true) {}
    using JobSupport::init_parent_job;

    bool get_on_cancel_complete() const override { return true; }

    std::exception_ptr get_completion_exception_or_null() const override {
        return JobSupport::get_completion_exception_or_null();
    }

    /** Returns a copy of the completed value, or throws for failure or incomplete state. */
    T get_completed() const override {
        auto* state = this->get_completed_internal();
        if (auto* value = dynamic_cast<CompletedValue<T>*>(state)) {
            return value->value;
        }
        throw std::logic_error("Unexpected completion state");
    }

    /** Upstream: override suspend fun await(): T = awaitInternal() as T */
    void* await(Continuation<void*>* continuation) override {
        return this->await_internal(continuation);
    }

    T await_blocking() override {
        void* state = this->await_internal_blocking();
        if (auto* ex = dynamic_cast<CompletedExceptionally*>(static_cast<JobState*>(state))) {
            std::rethrow_exception(ex->cause);
        }
        return this->get_completed();
    }

    /** Upstream: override val onAwait: SelectClause1<T> get() = onAwaitInternal as SelectClause1<T> */
    selects::SelectClause1<T>& on_await() override {
        if (!on_await_clause_) {
            on_await_clause_ = std::make_unique<selects::SelectClause1Impl<T>>(
                this,
                [](void* clause_object, void* select, void* param) {
                    static_cast<JobSupport*>(static_cast<CompletableDeferredImpl<T>*>(clause_object))->on_await_internal_reg_func(select, param);
                },
                [](void* clause_object, void* param, void* result) -> void* {
                    return JobSupport::on_await_internal_process_res_func(clause_object, param, result);
                }
            );
        }
        return *on_await_clause_;
    }

    /** Completes the job with a typed value; an already completed job is unchanged. */
    bool complete(T value) override {
        return this->make_completing(new CompletedValue<T>(std::move(value)));
    }

    /**
     * Upstream:
     *   override fun completeExceptionally(exception: Throwable): Boolean =
     *       makeCompleting(CompletedExceptionally(exception))
     */
    bool complete_exceptionally(std::exception_ptr exception) override {
        return this->make_completing(new CompletedExceptionally(exception));
    }

private:
    std::unique_ptr<selects::SelectClause1Impl<T>> on_await_clause_;
};

/** Creates an active deferred, optionally attached to a parent job. */
template <typename T>
inline std::shared_ptr<CompletableDeferred<T>> make_completable_deferred(
    std::shared_ptr<Job> parent = nullptr) {
    auto deferred = std::make_shared<CompletableDeferredImpl<T>>();
    deferred->init_parent_job(std::move(parent));
    return deferred;
}

/** Creates a parentless deferred already completed with the given value. */
template <typename T>
inline std::shared_ptr<CompletableDeferred<T>> make_completable_deferred(T value) {
    auto deferred = std::make_shared<CompletableDeferredImpl<T>>();
    deferred->init_parent_job(nullptr);
    deferred->complete(std::move(value));
    return deferred;
}

} // namespace kotlinx::coroutines
