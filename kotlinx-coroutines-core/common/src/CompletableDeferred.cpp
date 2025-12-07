// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/CompletableDeferred.kt
//
// TODO: @file:Suppress - no C++ equivalent
// TODO: Result<T> needs custom implementation
// TODO: fold method on Result needs implementation

#include "kotlinx/coroutines/core_fwd.hpp"

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.selects.* - use includes

/**
 * A [Deferred] that can be completed via functions [complete] or [cancel][Job.cancel].
 *
 * Note that the [complete] function returns `false` when this deferred value is already complete or completing,
 * while [cancel][Job.cancel] returns `true` as long as the deferred is still _cancelling_ and the corresponding
 * exception is incorporated into the final [completion exception][getCompletionExceptionOrNull].
 *
 * An instance of completable deferred can be created by `CompletableDeferred()` function in _active_ state.
 *
 * All functions on this struct are **thread-safe** and can
 * be safely invoked from concurrent coroutines without external synchronization.
 */
// TODO: @OptIn(ExperimentalSubclassOptIn::class) - no C++ equivalent
// TODO: @SubclassOptInRequired(InternalForInheritanceCoroutinesApi::class) - no C++ equivalent
template<typename T>
class CompletableDeferred : public Deferred<T> {
public:
    /**
     * Completes this deferred value with a given [value]. The result is `true` if this deferred was
     * completed as a result of this invocation and `false` otherwise (if it was already completed).
     *
     * Subsequent invocations of this function have no effect and always produce `false`.
     *
     * This function transitions this deferred into _completed_ state if it was not completed or cancelled yet.
     * However, if this deferred has children, then it transitions into _completing_ state and becomes _complete_
     * once all its children are [complete][isCompleted]. See [Job] for details.
     */
    virtual bool complete(T value) = 0;

    /**
     * Completes this deferred value exceptionally with a given [exception]. The result is `true` if this deferred was
     * completed as a result of this invocation and `false` otherwise (if it was already completed).
     *
     * Subsequent invocations of this function have no effect and always produce `false`.
     *
     * This function transitions this deferred into _cancelled_ state if it was not completed or cancelled yet.
     * However, that if this deferred has children, then it transitions into _cancelling_ state and becomes _cancelled_
     * once all its children are [complete][isCompleted]. See [Job] for details.
     */
    virtual bool complete_exceptionally(Throwable* exception) = 0;
};

/**
 * Completes this deferred value with the value or exception in the given [result]. Returns `true` if this deferred
 * was completed as a result of this invocation and `false` otherwise (if it was already completed).
 *
 * Subsequent invocations of this function have no effect and always produce `false`.
 *
 * This function transitions this deferred in the same ways described by [CompletableDeferred.complete] and
 * [CompletableDeferred.completeExceptionally].
 */
// TODO: Extension function
template<typename T>
bool complete_with(CompletableDeferred<T>* deferred, Result<T> result) {
    // TODO: result.fold({ complete(it) }, { completeExceptionally(it) })
    // Placeholder implementation
    if (result.isSuccess()) {
        return deferred->complete(result.getValue());
    } else {
        return deferred->complete_exceptionally(result.getException());
    }
}

/**
 * Creates a [CompletableDeferred] in an _active_ state.
 * It is optionally a child of a [parent] job.
 */
// TODO: @Suppress("FunctionName") - naming convention, no C++ equivalent
template<typename T>
CompletableDeferred<T>* create_completable_deferred(Job* parent = nullptr) {
    return new CompletableDeferredImpl<T>(parent);
}

/**
 * Creates an already _completed_ [CompletableDeferred] with a given [value].
 */
// TODO: @Suppress("FunctionName") - no C++ equivalent
template<typename T>
CompletableDeferred<T>* create_completable_deferred(T value) {
    auto* deferred = new CompletableDeferredImpl<T>(nullptr);
    deferred->complete(value);
    return deferred;
}

/**
 * Concrete implementation of [CompletableDeferred].
 */
// TODO: @OptIn(InternalForInheritanceCoroutinesApi::class) - no C++ equivalent
// TODO: @Suppress("UNCHECKED_CAST") - no C++ equivalent
// TODO: class - access control
template<typename T>
class CompletableDeferredImpl : public JobSupport, public CompletableDeferred<T> {
private:
    Job* parent_job;

public:
    CompletableDeferredImpl(Job* parent)
        : JobSupport(true), parent_job(parent) {
        init_parent_job(parent);
    }

    bool get_on_cancel_complete() override {
        return true;
    }

    T get_completed() override {
        // TODO: getCompletedInternal() as T - cast
        return static_cast<T>(this->get_completed_internal());
    }

    // TODO: suspend fun
    T await() override {
        // TODO: awaitInternal() as T - cast
        return static_cast<T>(this->await_internal());
    }

    // TODO: auto onAwait property
    SelectClause1<T>* get_on_await() override {
        // TODO: onAwaitInternal as SelectClause1<T> - cast
        return static_cast<SelectClause1<T>*>(this->on_await_internal);
    }

    bool complete(T value) override {
        return this->make_completing(value);
    }

    bool complete_exceptionally(Throwable* exception) override {
        return this->make_completing(new CompletedExceptionally(exception));
    }
};

} // namespace coroutines
} // namespace kotlinx
