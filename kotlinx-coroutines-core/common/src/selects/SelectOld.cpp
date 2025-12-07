#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/selects/SelectOld.kt
//
// TODO: This is a mechanical syntax transliteration. The following Kotlin constructs need proper C++ implementation:
// - suspend functions (marked but not implemented as C++20 coroutines)
// - inline functions with crossinline parameters
// - Continuation<T> type (Kotlin coroutine continuation)
// - suspendCoroutineUninterceptedOrReturn primitive
// - CancellableContinuationImpl
// - CoroutineScope, CoroutineStart.UNDISPATCHED
// - CoroutineDispatcher and context management
// - @PublishedApi, @OptIn, @ExperimentalStdlibApi annotations (kept as comments)

namespace kotlinx {
namespace coroutines {
namespace selects {

// import kotlinx.coroutines.*
// import kotlin.coroutines.*
// import kotlin.coroutines.intrinsics.*

/*
 * For binary compatibility, we need to maintain the previous `select` implementations.
 * Thus, we keep [SelectBuilderImpl] and [UnbiasedSelectBuilderImpl] and implement the
 * functions marked with `@PublishedApi`.
 *
 * We keep the old `select` functions as [selectOld] and [selectUnbiasedOld] for test purpose.
 */

// @PublishedApi
template<typename R>
class SelectBuilderImpl : SelectImplementation<R> {
private:
    CancellableContinuationImpl<R> cont;

public:
    // uCont: unintercepted delegate continuation
    explicit SelectBuilderImpl(Continuation<R>& u_cont)
        : SelectImplementation<R>(u_cont.context),
          cont(u_cont.intercepted(), kModeCancellable) {}

    // @PublishedApi
    void* get_result() {
        // In the current `select` design, the [select] and [selectUnbiased] functions
        // do not wrap the operation in `suspendCoroutineUninterceptedOrReturn` and
        // suspend explicitly via [doSelect] call, which returns the final result.
        // However, [doSelect] is a suspend function, so it cannot be invoked directly.
        // In addition, the `select` builder is eligible to throw an exception, which
        // should be handled properly.
        //
        // As a solution, we:
        // 1) check whether the `select` building is already completed with exception, finishing immediately in this case;
        // 2) create a CancellableContinuationImpl with the provided unintercepted continuation as a delegate;
        // 3) wrap the [doSelect] call in an additional coroutine, which we launch in UNDISPATCHED mode;
        // 4) resume the created CancellableContinuationImpl after the [doSelect] invocation completes;
        // 5) use CancellableContinuationImpl.getResult() as a result of this function.
        if (cont.is_completed()) return cont.get_result();

        // TODO: CoroutineScope and launch not implemented
        auto scope = CoroutineScope(this->context);
        scope.launch(/* start = */ CoroutineStart::kUndispatched, [this]() {
            // TODO: suspend function call
            R result;
            try {
                result = this->do_select();
            } catch (std::exception& e) {
                cont.resume_undispatched_with_exception(e);
                return;
            }
            cont.resume_undispatched(result);
        });
        return cont.get_result();
    }

    // @PublishedApi
    void handle_builder_exception(std::exception& e) {
        cont.resume_with_exception(e); // will be thrown later via `cont.getResult()`
    }
};

// @PublishedApi
template<typename R>
class UnbiasedSelectBuilderImpl : UnbiasedSelectImplementation<R> {
private:
    CancellableContinuationImpl<R> cont;

public:
    // uCont: unintercepted delegate continuation
    explicit UnbiasedSelectBuilderImpl(Continuation<R>& u_cont)
        : UnbiasedSelectImplementation<R>(u_cont.context),
          cont(u_cont.intercepted(), kModeCancellable) {}

    // @PublishedApi
    void* init_select_result() {
        // Here, we do the same trick as in [SelectBuilderImpl].
        if (cont.is_completed()) return cont.get_result();

        // TODO: CoroutineScope and launch not implemented
        auto scope = CoroutineScope(this->context);
        scope.launch(/* start = */ CoroutineStart::kUndispatched, [this]() {
            // TODO: suspend function call
            R result;
            try {
                result = this->do_select();
            } catch (std::exception& e) {
                cont.resume_undispatched_with_exception(e);
                return;
            }
            cont.resume_undispatched(result);
        });
        return cont.get_result();
    }

    // @PublishedApi
    void handle_builder_exception(std::exception& e) {
        cont.resume_with_exception(e);
    }
};

/*
 * This is the old version of `select`. It should work to guarantee binary compatibility.
 *
 * Internal note:
 * We do test it manually by changing the implementation of **new** select with the following:
 * ```
 * inline fun <R> select(crossinline builder: SelectBuilder<R>.() -> Unit): R {
 *     contract {
 *         callsInPlace(builder, InvocationKind.EXACTLY_ONCE)
 *     }
 *     return selectOld(builder)
 * }
 * ```
 *
 * These signatures are not used by the already compiled code, but their body is.
 */
// @PublishedApi
template<typename R, typename BuilderFunc>
R select_old(BuilderFunc&& builder) {
    // TODO: suspend function semantics not implemented
    // TODO: suspendCoroutineUninterceptedOrReturn not directly translatable
    return suspend_coroutine_unintercepted_or_return<R>([&builder](Continuation<R>& u_cont) -> void* {
        SelectBuilderImpl<R> scope(u_cont);
        try {
            builder(scope);
        } catch (std::exception& e) {
            scope.handle_builder_exception(e);
        }
        return scope.get_result();
    });
}

// This is the old version of `selectUnbiased`. It should work to guarantee binary compatibility.
// @PublishedApi
template<typename R, typename BuilderFunc>
R select_unbiased_old(BuilderFunc&& builder) {
    // TODO: suspend function semantics not implemented
    return suspend_coroutine_unintercepted_or_return<R>([&builder](Continuation<R>& u_cont) -> void* {
        UnbiasedSelectBuilderImpl<R> scope(u_cont);
        try {
            builder(scope);
        } catch (std::exception& e) {
            scope.handle_builder_exception(e);
        }
        return scope.init_select_result();
    });
}

// @OptIn(ExperimentalStdlibApi::class)
template<typename T>
void resume_undispatched(CancellableContinuation<T>& cont, T result) {
    // TODO: context[CoroutineDispatcher] not directly translatable
    auto* dispatcher = cont.context.template get<CoroutineDispatcher>();
    if (dispatcher != nullptr) {
        dispatcher->resume_undispatched(result);
    } else {
        cont.resume(result);
    }
}

// @OptIn(ExperimentalStdlibApi::class)
void resume_undispatched_with_exception(CancellableContinuation<void*>& cont, std::exception& exception) {
    // TODO: context[CoroutineDispatcher] not directly translatable
    auto* dispatcher = cont.context.template get<CoroutineDispatcher>();
    if (dispatcher != nullptr) {
        dispatcher->resume_undispatched_with_exception(exception);
    } else {
        cont.resume_with_exception(exception);
    }
}

} // namespace selects
} // namespace coroutines
} // namespace kotlinx
