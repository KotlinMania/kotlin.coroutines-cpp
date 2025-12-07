// Transliterated from: reactive/kotlinx-coroutines-reactor/src/Mono.cpp

// @file:Suppress("INVISIBLE_REFERENCE", "INVISIBLE_MEMBER")

// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <kotlinx/coroutines/reactive.hpp>
// TODO: #include <org/reactivestreams/publisher.hpp>
// TODO: #include <reactor/core/publisher.hpp>
// TODO: #include <kotlin/coroutines/continuation.hpp>
// TODO: #include <kotlinx/coroutines/internal.hpp>

namespace kotlinx { namespace coroutines { namespace reactor {

/**
 * Creates a cold [mono][Mono] that runs a given [block] in a coroutine and emits its result.
 * Every time the returned mono is subscribed, it starts a new coroutine.
 * If the result of [block] is `null`, [MonoSink.success] is invoked without a value.
 * Unsubscribing cancels the running coroutine.
 *
 * Coroutine context can be specified with [context] argument.
 * If the context does not have any dispatcher nor any other [ContinuationInterceptor], then [Dispatchers.Default] is used.
 *
 * @throws IllegalArgumentException if the provided [context] contains a [Job] instance.
 */
template<typename T>
// TODO: implement coroutine suspension
Mono<T> mono(
    CoroutineContext context = EmptyCoroutineContext,
    std::function<T*(CoroutineScope&)> block
) {
    if (context[Job{}] != nullptr) {
        throw std::invalid_argument("Mono context cannot contain job in it. "
                "Its lifecycle should be managed via Disposable handle. Had " + context.to_string());
    }
    return mono_internal(GlobalScope, context, block);
}

/**
 * Awaits the single value from the given [Mono] without blocking the thread and returns the resulting value, or, if
 * this publisher has produced an error, throws the corresponding exception. If the Mono completed without a value,
 * `null` is returned.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Subscription] and resumes with [CancellationException].
 */
template<typename T>
// TODO: implement coroutine suspension
T* await_single_or_null(Mono<T>* mono) {
    return suspend_cancellable_coroutine<T*>([mono](CancellableContinuation<T*>& cont) {
        mono->inject_coroutine_context(cont.context())->subscribe(new class : public Subscriber<T> {
        private:
            T* value = nullptr;

        public:
            void on_subscribe(Subscription* s) override {
                cont.invoke_on_cancellation([s]() { s->cancel(); });
                s->request(INT64_MAX);
            }

            void on_complete() override {
                cont.resume(value);
                value = nullptr;
            }

            void on_next(T t) override {
                // We don't return the value immediately because the process that emitted it may not be finished yet.
                // Resuming now could lead to race conditions between emitter and the awaiting code.
                value = new T(t);
            }

            void on_error(Throwable* error) override {
                cont.resume_with_exception(error);
            }
        });
    });
}

/**
 * Awaits the single value from the given [Mono] without blocking the thread and returns the resulting value, or,
 * if this Mono has produced an error, throws the corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Subscription] and resumes with [CancellationException].
 *
 * @throws NoSuchElementException if the Mono does not emit any value
 */
// TODO: consider using https://github.com/Kotlin/kotlinx.coroutines/issues/2607 once that lands
template<typename T>
// TODO: implement coroutine suspension
T await_single(Mono<T>* mono) {
    T* result = await_single_or_null(mono);
    if (result == nullptr) {
        throw std::out_of_range("No such element");
    }
    return *result;
}

template<typename T>
// TODO: implement coroutine suspension
Mono<T> mono_internal(
    CoroutineScope* scope, // support for legacy mono in scope
    CoroutineContext context,
    std::function<T*(CoroutineScope&)> block
) {
    return Mono<T>::create([scope, context, block](MonoSink<T>* sink) {
        ReactorContext reactor_context = context.extend_reactor_context(sink->current_context());
        CoroutineContext new_context = scope->new_coroutine_context(context + reactor_context);
        MonoCoroutine<T>* coroutine = new MonoCoroutine<T>(new_context, sink);
        sink->on_dispose(coroutine);
        coroutine->start(CoroutineStart::kDefault, coroutine, block);
    });
}

template<typename T>
class MonoCoroutine : public AbstractCoroutine<T>, public Disposable {
private:
    MonoSink<T>* sink;
    // @Volatile
    bool disposed = false;

public:
    MonoCoroutine(CoroutineContext parent_context, MonoSink<T>* sink_param)
        : AbstractCoroutine<T>(parent_context, false, true), sink(sink_param), disposed(false) {
    }

    void on_completed(T value) override {
        if (value == nullptr) {
            sink->success();
        } else {
            sink->success(value);
        }
    }

    void on_cancelled(Throwable* cause, bool handled) override {
        /** Cancellation exceptions that were caused by [dispose], that is, came from downstream, are not errors. */
        Throwable* unwrapped_cause = unwrap(cause);
        if (get_cancellation_exception() != unwrapped_cause || !disposed) {
            try {
                /** If [sink] turns out to already be in a terminal state, this exception will be passed through the
                 * [Hooks.onOperatorError] hook, which is the way to signal undeliverable exceptions in Reactor. */
                sink->error(cause);
            } catch (std::exception& e) {
                // In case of improper error implementation or fatal exceptions
                cause->add_suppressed(e);
                handle_coroutine_exception(context, cause);
            }
        }
    }

    void dispose() override {
        disposed = true;
        cancel();
    }

    bool is_disposed() override {
        return disposed;
    }
};

/**
 * @suppress
 */
// @Deprecated
//     message = "CoroutineScope.mono is deprecated in favour of top-level mono",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("mono(context, block)")
// Since 1.3.0, will be error in 1.3.1 and hidden in 1.4.0
template<typename T>
// TODO: implement coroutine suspension
Mono<T> mono(
    CoroutineScope* scope,
    CoroutineContext context = EmptyCoroutineContext,
    std::function<T*(CoroutineScope&)> block
) {
    return mono_internal(scope, context, block);
}

/**
 * This is a lint function that was added already deprecated in order to guard against confusing usages on [Mono].
 * On [Publisher] instances other than [Mono], this function is not deprecated.
 *
 * Both [awaitFirst] and [awaitSingle] await the first value, or throw [NoSuchElementException] if there is none, but
 * the name [Mono.awaitSingle] better reflects the semantics of [Mono].
 *
 * For example, consider this code:
 * ```
 * myDbClient.findById(uniqueId).awaitFirst() // findById returns a `Mono`
 * ```
 * It looks like more than one value could be returned from `findById` and [awaitFirst] discards the extra elements,
 * when in fact, at most a single value can be present.
 *
 * @suppress
 */
// @Deprecated
//     message = "Mono produces at most one value, so the semantics of dropping the remaining elements are not useful. " +
//         "Please use awaitSingle() instead.",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("this.awaitSingle()")
// Warning since 1.5, error in 1.6
template<typename T>
// TODO: implement coroutine suspension
T await_first(Mono<T>* mono) {
    return await_single(mono);
}

/**
 * This is a lint function that was added already deprecated in order to guard against confusing usages on [Mono].
 * On [Publisher] instances other than [Mono], this function is not deprecated.
 *
 * Both [awaitFirstOrDefault] and [awaitSingleOrNull] await the first value, or return some special value if there
 * is none, but the name [Mono.awaitSingleOrNull] better reflects the semantics of [Mono].
 *
 * For example, consider this code:
 * ```
 * myDbClient.findById(uniqueId).awaitFirstOrDefault(default) // findById returns a `Mono`
 * ```
 * It looks like more than one value could be returned from `findById` and [awaitFirstOrDefault] discards the extra
 * elements, when in fact, at most a single value can be present.
 *
 * @suppress
 */
// @Deprecated
//     message = "Mono produces at most one value, so the semantics of dropping the remaining elements are not useful. " +
//         "Please use awaitSingleOrNull() instead.",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("this.awaitSingleOrNull() ?: default")
// Warning since 1.5, error in 1.6
template<typename T>
// TODO: implement coroutine suspension
T await_first_or_default(Mono<T>* mono, T default_value) {
    T* result = await_single_or_null(mono);
    return result != nullptr ? *result : default_value;
}

/**
 * This is a lint function that was added already deprecated in order to guard against confusing usages on [Mono].
 * On [Publisher] instances other than [Mono], this function is not deprecated.
 *
 * Both [awaitFirstOrNull] and [awaitSingleOrNull] await the first value, or return some special value if there
 * is none, but the name [Mono.awaitSingleOrNull] better reflects the semantics of [Mono].
 *
 * For example, consider this code:
 * ```
 * myDbClient.findById(uniqueId).awaitFirstOrNull() // findById returns a `Mono`
 * ```
 * It looks like more than one value could be returned from `findById` and [awaitFirstOrNull] discards the extra
 * elements, when in fact, at most a single value can be present.
 *
 * @suppress
 */
// @Deprecated
//     message = "Mono produces at most one value, so the semantics of dropping the remaining elements are not useful. " +
//         "Please use awaitSingleOrNull() instead.",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("this.awaitSingleOrNull()")
// Warning since 1.5, error in 1.6
template<typename T>
// TODO: implement coroutine suspension
T* await_first_or_null(Mono<T>* mono) {
    return await_single_or_null(mono);
}

/**
 * This is a lint function that was added already deprecated in order to guard against confusing usages on [Mono].
 * On [Publisher] instances other than [Mono], this function is not deprecated.
 *
 * Both [awaitFirstOrElse] and [awaitSingleOrNull] await the first value, or return some special value if there
 * is none, but the name [Mono.awaitSingleOrNull] better reflects the semantics of [Mono].
 *
 * For example, consider this code:
 * ```
 * myDbClient.findById(uniqueId).awaitFirstOrElse(defaultValue) // findById returns a `Mono`
 * ```
 * It looks like more than one value could be returned from `findById` and [awaitFirstOrElse] discards the extra
 * elements, when in fact, at most a single value can be present.
 *
 * @suppress
 */
// @Deprecated
//     message = "Mono produces at most one value, so the semantics of dropping the remaining elements are not useful. " +
//         "Please use awaitSingleOrNull() instead.",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("this.awaitSingleOrNull() ?: defaultValue()")
// Warning since 1.5, error in 1.6
template<typename T>
// TODO: implement coroutine suspension
T await_first_or_else(Mono<T>* mono, std::function<T()> default_value) {
    T* result = await_single_or_null(mono);
    return result != nullptr ? *result : default_value();
}

/**
 * This is a lint function that was added already deprecated in order to guard against confusing usages on [Mono].
 * On [Publisher] instances other than [Mono], this function is not deprecated.
 *
 * Both [awaitLast] and [awaitSingle] await the single value, or throw [NoSuchElementException] if there is none, but
 * the name [Mono.awaitSingle] better reflects the semantics of [Mono].
 *
 * For example, consider this code:
 * ```
 * myDbClient.findById(uniqueId).awaitLast() // findById returns a `Mono`
 * ```
 * It looks like more than one value could be returned from `findById` and [awaitLast] discards the initial elements,
 * when in fact, at most a single value can be present.
 *
 * @suppress
 */
// @Deprecated
//     message = "Mono produces at most one value, so the last element is the same as the first. " +
//         "Please use awaitSingle() instead.",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("this.awaitSingle()")
// Warning since 1.5, error in 1.6
template<typename T>
// TODO: implement coroutine suspension
T await_last(Mono<T>* mono) {
    return await_single(mono);
}

} } } // namespace kotlinx::coroutines::reactor

// TODO: Semantic implementation tasks:
// 1. Implement Mono<T> type and create() static method
// 2. Implement MonoSink<T> interface
// 3. Implement AbstractCoroutine<T> base class
// 4. Implement Disposable interface
// 5. Implement suspend_cancellable_coroutine() function
// 6. Implement CancellableContinuation<T> type
// 7. Implement Subscription interface
// 8. Implement Subscriber<T> interface
// 9. Implement unwrap() for exceptions
// 10. Implement get_cancellation_exception() method
// 11. Implement handle_coroutine_exception() function
// 12. Implement inject_coroutine_context() for Mono
// 13. Implement INT64_MAX constant
// 14. Handle @Volatile annotation with std::atomic if needed
// 15. Implement proper exception hierarchy
// 16. Handle nullable return types with std::optional or pointers
// 17. Implement CoroutineStart enum
// 18. Implement GlobalScope singleton
