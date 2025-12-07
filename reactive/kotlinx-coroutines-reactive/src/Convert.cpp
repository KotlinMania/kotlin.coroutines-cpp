// Transliterated from: reactive/kotlinx-coroutines-reactive/src/Convert.kt
// TODO: Implement semantic correctness for channel conversion

namespace kotlinx {
namespace coroutines {
namespace reactive {

// TODO: #include <kotlinx/coroutines/channels/channels.hpp>
// TODO: #include <org/reactivestreams/Publisher.hpp>
// TODO: #include <kotlin/coroutines/CoroutineContext.hpp>

/** @suppress */
// @Deprecated(message = "Deprecated in the favour of consumeAsFlow()",
//     level = DeprecationLevel.HIDDEN, // Error in 1.4, HIDDEN in 1.6.0
//     replaceWith = ReplaceWith("this.consumeAsFlow().asPublisher(context)", imports = ["kotlinx.coroutines.flow.consumeAsFlow"]))
[[deprecated("Deprecated in the favour of consume_as_flow(). Use this.consume_as_flow().as_publisher(context)")]]
template<typename T>
Publisher<T> as_publisher(ReceiveChannel<T>& channel, const CoroutineContext& context = EmptyCoroutineContext) {
    return publish(context, [&channel](ProducerScope<T>& scope) {
        // TODO: implement coroutine suspension
        for (const auto& t : channel) {
            scope.send(t);
        }
    });
}

} // namespace reactive
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO List for semantic implementation:
 *
 * 1. Implement ReceiveChannel<T> interface
 * 2. Implement publish() function
 * 3. Implement ProducerScope<T>
 * 4. Implement CoroutineContext and EmptyCoroutineContext
 * 5. Implement for-each iteration over channels
 * 6. Implement coroutine suspension in lambda
 */
