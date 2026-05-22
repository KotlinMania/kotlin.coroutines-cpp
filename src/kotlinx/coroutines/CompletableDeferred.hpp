#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/CompletableDeferred.kt
 *
 * Kotlin file header (translated):
 *   @file:Suppress("DEPRECATION_ERROR")
 *   package kotlinx.coroutines
 */

#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Deferred.hpp"
#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"

#include <exception>
#include <memory>
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
 * Concrete implementation of [CompletableDeferred].
 *
 * Upstream:
 *   @OptIn(InternalForInheritanceCoroutinesApi::class)
 *   @Suppress("UNCHECKED_CAST")
 *   private class CompletableDeferredImpl<T>(parent: Job?) : JobSupport(true), CompletableDeferred<T> {
 *       init { initParentJob(parent) }
 *       override val onCancelComplete get() = true
 *       override fun getCompleted(): T = getCompletedInternal() as T
 *       override suspend fun await(): T = awaitInternal() as T
 *       override val onAwait: SelectClause1<T> get() = onAwaitInternal as SelectClause1<T>
 *       override fun complete(value: T): Boolean = makeCompleting(value)
 *       override fun completeExceptionally(exception: Throwable): Boolean =
 *           makeCompleting(CompletedExceptionally(exception))
 *   }
 */
template <typename T>
class CompletableDeferredImpl : public JobSupport, public CompletableDeferred<T> {
public:
    explicit CompletableDeferredImpl(std::shared_ptr<Job> parent) : JobSupport(true) {
        this->init_parent_job(std::move(parent));
    }

    bool on_cancel_complete() const override { return true; }

    /** Upstream: override fun getCompleted(): T = getCompletedInternal() as T */
    T get_completed() const override {
        return *static_cast<T*>(this->get_completed_internal());
    }

    /** Upstream: override suspend fun await(): T = awaitInternal() as T */
    void* await(Continuation<void*>* continuation) override {
        return this->await_internal(continuation);
    }

    /** Upstream: override val onAwait: SelectClause1<T> get() = onAwaitInternal as SelectClause1<T> */
    selects::SelectClause1<T>& on_await() override {
        return reinterpret_cast<selects::SelectClause1<T>&>(this->on_await_internal());
    }

    /** Upstream: override fun complete(value: T): Boolean = makeCompleting(value) */
    bool complete(T value) override {
        return this->make_completing(new T(std::move(value)));
    }

    /**
     * Upstream:
     *   override fun completeExceptionally(exception: Throwable): Boolean =
     *       makeCompleting(CompletedExceptionally(exception))
     */
    bool complete_exceptionally(std::exception_ptr exception) override {
        return this->make_completing(new CompletedExceptionally(exception));
    }
};

/**
 * Creates a [CompletableDeferred] in an _active_ state.
 *
 * Upstream:
 *   @Suppress("FunctionName")
 *   public fun <T> CompletableDeferred(parent: Job? = null): CompletableDeferred<T> =
 *       CompletableDeferredImpl(parent)
 */
template <typename T>
inline std::shared_ptr<CompletableDeferred<T>> make_completable_deferred(
    std::shared_ptr<Job> parent = nullptr) {
    return std::make_shared<CompletableDeferredImpl<T>>(std::move(parent));
}

/**
 * Creates an already _completed_ [CompletableDeferred] with a given [value].
 *
 * Upstream:
 *   @Suppress("FunctionName")
 *   public fun <T> CompletableDeferred(value: T): CompletableDeferred<T> =
 *       CompletableDeferredImpl<T>(null).apply { complete(value) }
 */
template <typename T>
inline std::shared_ptr<CompletableDeferred<T>> make_completable_deferred(T value) {
    auto deferred = std::make_shared<CompletableDeferredImpl<T>>(nullptr);
    deferred->complete(std::move(value));
    return deferred;
}

} // namespace kotlinx::coroutines
