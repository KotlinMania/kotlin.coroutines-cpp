/**
 * Transliterated from: kotlinx-coroutines-core/common/src/selects/SelectOld.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.selects
 *
 * Upstream Kotlin file declares two `@PublishedApi internal` classes —
 * `SelectBuilderImpl<R>` and `UnbiasedSelectBuilderImpl<R>` — plus the legacy `selectOld`
 * and `selectUnbiasedOld` suspend builders. Both classes wrap a
 * `CancellableContinuationImpl` and stage a `doSelect()` invocation through
 * `CoroutineScope(context).launch(start = UNDISPATCHED)`.
 */

#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"
#include "kotlinx/coroutines/selects/SelectUnbiased.hpp"

#include <exception>
#include <functional>
#include <memory>
#include <utility>

namespace kotlinx::coroutines::selects {

namespace {

/**
 * Upstream:
 *   private fun <T> CancellableContinuation<T>.resumeUndispatched(result: T) {
 *       val dispatcher = context[CoroutineDispatcher]
 *       if (dispatcher != null) dispatcher.resumeUndispatched(result) else resume(result)
 *   }
 */
template <typename T>
void resume_undispatched(CancellableContinuation<T>* cont, T result) {
    auto ctx = cont->get_context();
    auto* element = ctx ? ctx->get(CoroutineDispatcher::type_key()) : nullptr;
    auto* dispatcher = dynamic_cast<CoroutineDispatcher*>(element);
    if (dispatcher) {
        dispatcher->resume_undispatched(cont, std::move(result));
    } else {
        cont->resume(std::move(result));
    }
}

/**
 * Upstream:
 *   private fun CancellableContinuation<*>.resumeUndispatchedWithException(exception: Throwable) {
 *       val dispatcher = context[CoroutineDispatcher]
 *       if (dispatcher != null) dispatcher.resumeUndispatchedWithException(exception)
 *       else resumeWithException(exception)
 *   }
 */
template <typename T>
void resume_undispatched_with_exception(CancellableContinuation<T>* cont,
                                        std::exception_ptr exception) {
    auto ctx = cont->get_context();
    auto* element = ctx ? ctx->get(CoroutineDispatcher::type_key()) : nullptr;
    auto* dispatcher = dynamic_cast<CoroutineDispatcher*>(element);
    if (dispatcher) {
        dispatcher->resume_undispatched_with_exception(cont, exception);
    } else {
        cont->resume_with_exception(exception);
    }
}

} // namespace

/**
 * Upstream:
 *   @PublishedApi
 *   internal class SelectBuilderImpl<R>(uCont: Continuation<R>)
 *       : SelectImplementation<R>(uCont.context) {
 *       private val cont = CancellableContinuationImpl(uCont.intercepted(), MODE_CANCELLABLE)
 *       ...
 *   }
 */
template <typename R>
class SelectBuilderImpl : public SelectImplementation<R> {
public:
    explicit SelectBuilderImpl(std::shared_ptr<Continuation<R>> u_cont)
        : SelectImplementation<R>(u_cont->get_context()),
          cont_(std::make_shared<CancellableContinuationImpl<R>>(
              u_cont->intercepted(), MODE_CANCELLABLE)) {}

    /**
     * Upstream:
     *   @PublishedApi internal fun getResult(): Any? {
     *       if (cont.isCompleted) return cont.getResult()
     *       CoroutineScope(context).launch(start = CoroutineStart.UNDISPATCHED) {
     *           val result = try { doSelect() } catch (e: Throwable) {
     *               cont.resumeUndispatchedWithException(e); return@launch
     *           }
     *           cont.resumeUndispatched(result)
     *       }
     *       return cont.getResult()
     *   }
     */
    void* get_result() {
        if (cont_->is_completed()) return cont_->get_result();
        CoroutineScope scope(this->get_context());
        scope.launch(CoroutineStart::UNDISPATCHED, [this]() {
            try {
                R result = this->do_select();
                resume_undispatched<R>(cont_.get(), std::move(result));
            } catch (...) {
                resume_undispatched_with_exception<R>(cont_.get(), std::current_exception());
            }
        });
        return cont_->get_result();
    }

    /** Upstream: @PublishedApi internal fun handleBuilderException(e: Throwable) { ... } */
    void handle_builder_exception(std::exception_ptr exception) {
        cont_->resume_with_exception(exception);
    }

private:
    std::shared_ptr<CancellableContinuationImpl<R>> cont_;
};

/**
 * Upstream:
 *   @PublishedApi
 *   internal class UnbiasedSelectBuilderImpl<R>(uCont: Continuation<R>)
 *       : UnbiasedSelectImplementation<R>(uCont.context) { ... }
 */
template <typename R>
class UnbiasedSelectBuilderImpl : public UnbiasedSelectImplementation<R> {
public:
    explicit UnbiasedSelectBuilderImpl(std::shared_ptr<Continuation<R>> u_cont)
        : UnbiasedSelectImplementation<R>(u_cont->get_context()),
          cont_(std::make_shared<CancellableContinuationImpl<R>>(
              u_cont->intercepted(), MODE_CANCELLABLE)) {}

    /** Upstream: @PublishedApi internal fun initSelectResult(): Any? { ... } */
    void* init_select_result() {
        if (cont_->is_completed()) return cont_->get_result();
        CoroutineScope scope(this->get_context());
        scope.launch(CoroutineStart::UNDISPATCHED, [this]() {
            try {
                R result = this->do_select();
                resume_undispatched<R>(cont_.get(), std::move(result));
            } catch (...) {
                resume_undispatched_with_exception<R>(cont_.get(), std::current_exception());
            }
        });
        return cont_->get_result();
    }

    /** Upstream: @PublishedApi internal fun handleBuilderException(e: Throwable) { ... } */
    void handle_builder_exception(std::exception_ptr exception) {
        cont_->resume_with_exception(exception);
    }

private:
    std::shared_ptr<CancellableContinuationImpl<R>> cont_;
};

/**
 * Upstream:
 *   @PublishedApi
 *   internal suspend inline fun <R> selectOld(crossinline builder: SelectBuilder<R>.() -> Unit): R {
 *       return suspendCoroutineUninterceptedOrReturn { uCont ->
 *           val scope = SelectBuilderImpl(uCont)
 *           try { builder(scope) } catch (e: Throwable) { scope.handleBuilderException(e) }
 *           scope.getResult()
 *       }
 *   }
 */
template <typename R>
[[suspend]]
void* select_old(
    std::function<void(SelectBuilder<R>&)> builder,
    std::shared_ptr<Continuation<R>> u_cont) {
    SelectBuilderImpl<R> scope(u_cont);
    try {
        builder(scope);
    } catch (...) {
        scope.handle_builder_exception(std::current_exception());
    }
    return scope.get_result();
}

/**
 * Upstream:
 *   @PublishedApi
 *   internal suspend inline fun <R> selectUnbiasedOld(crossinline builder: SelectBuilder<R>.() -> Unit): R =
 *       suspendCoroutineUninterceptedOrReturn { uCont ->
 *           val scope = UnbiasedSelectBuilderImpl(uCont)
 *           try { builder(scope) } catch (e: Throwable) { scope.handleBuilderException(e) }
 *           scope.initSelectResult()
 *       }
 */
template <typename R>
[[suspend]]
void* select_unbiased_old(
    std::function<void(SelectBuilder<R>&)> builder,
    std::shared_ptr<Continuation<R>> u_cont) {
    UnbiasedSelectBuilderImpl<R> scope(u_cont);
    try {
        builder(scope);
    } catch (...) {
        scope.handle_builder_exception(std::current_exception());
    }
    return scope.init_select_result();
}

} // namespace kotlinx::coroutines::selects
