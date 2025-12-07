// Transliterated from: reactive/kotlinx-coroutines-reactor/src/Convert.cpp

// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <kotlinx/coroutines/channels.hpp>
// TODO: #include <reactor/core/publisher.hpp>
// TODO: #include <kotlin/coroutines/continuation.hpp>

namespace kotlinx { namespace coroutines { namespace reactor {

/**
 * Converts this job to the hot reactive mono that signals
 * with [success][MonoSink.success] when the corresponding job completes.
 *
 * Every subscriber gets the signal at the same time.
 * Unsubscribing from the resulting mono **does not** affect the original job in any way.
 *
 * **Note: This is an experimental api.** Conversion of coroutines primitives to reactive entities may change
 *    in the future to account for the concept of structured concurrency.
 *
 * @param context -- the coroutine context from which the resulting mono is going to be signalled
 */
// TODO: implement coroutine suspension
Mono<Unit> as_mono(Job* job, CoroutineContext context) {
    return mono(context, [job]() {
        return job->join();
    });
}

/**
 * Converts this deferred value to the hot reactive mono that signals
 * [success][MonoSink.success] or [error][MonoSink.error].
 *
 * Every subscriber gets the same completion value.
 * Unsubscribing from the resulting mono **does not** affect the original deferred value in any way.
 *
 * **Note: This is an experimental api.** Conversion of coroutines primitives to reactive entities may change
 *    in the future to account for the concept of structured concurrency.
 *
 * @param context -- the coroutine context from which the resulting mono is going to be signalled
 */
template<typename T>
// TODO: implement coroutine suspension
Mono<T> as_mono(Deferred<T*>* deferred, CoroutineContext context) {
    return mono(context, [deferred]() {
        return deferred->await();
    });
}

/**
 * Converts a stream of elements received from the channel to the hot reactive flux.
 *
 * Every subscriber receives values from this channel in a **fan-out** fashion. If the are multiple subscribers,
 * they'll receive values in a round-robin way.
 * @param context -- the coroutine context from which the resulting flux is going to be signalled
 * @suppress
 */
// @Deprecated message = "Deprecated in the favour of consumeAsFlow()",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("this.consumeAsFlow().asFlux(context)", imports = ["kotlinx.coroutines.flow.consumeAsFlow"])
template<typename T>
// TODO: implement coroutine suspension
Flux<T> as_flux(ReceiveChannel<T>* channel, CoroutineContext context = EmptyCoroutineContext) {
    return flux(context, [channel](auto& producer_scope) {
        for (auto t : *channel) {
            producer_scope.send(t);
        }
    });
}

} } } // namespace kotlinx::coroutines::reactor

// TODO: Semantic implementation tasks:
// 1. Implement Job type and join() method
// 2. Implement Deferred<T> type and await() method
// 3. Implement ReceiveChannel<T> type and iteration
// 4. Implement Mono<T> and mono() builder function
// 5. Implement Flux<T> and flux() builder function
// 6. Implement CoroutineContext and EmptyCoroutineContext
// 7. Implement Unit type
// 8. Implement coroutine suspension mechanism
// 9. Handle nullable types (T?) properly with std::optional or pointers
// 10. Implement ProducerScope and send() method
