// Transliterated from: reactive/kotlinx-coroutines-reactive/src/Migration.kt
// TODO: Implement semantic correctness for migration compatibility

// @file:JvmMultifileClass
// @file:JvmName("FlowKt")

namespace kotlinx {
namespace coroutines {
namespace reactive {

// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <kotlinx/coroutines/flow/flow.hpp>
// TODO: #include <org/reactivestreams/Publisher.hpp>

// Binary compatibility with Spring 5.2 RC
/** @suppress */
// @Deprecated(
//     message = "Replaced in favor of ReactiveFlow extension, please import kotlinx.coroutines.reactive.* instead of kotlinx.coroutines.reactive.FlowKt",
//     level = DeprecationLevel.HIDDEN
// )
// @JvmName("asFlow")
[[deprecated("Replaced in favor of ReactiveFlow extension, please import kotlinx::coroutines::reactive::* instead")]]
template<typename T>
Flow<T> as_flow_deprecated(Publisher<T>& publisher) {
    return as_flow(publisher);
}

// Binary compatibility with Spring 5.2 RC
/** @suppress */
// @Deprecated(
//     message = "Replaced in favor of ReactiveFlow extension, please import kotlinx.coroutines.reactive.* instead of kotlinx.coroutines.reactive.FlowKt",
//     level = DeprecationLevel.HIDDEN
// )
// @JvmName("asPublisher")
[[deprecated("Replaced in favor of ReactiveFlow extension, please import kotlinx::coroutines::reactive::* instead")]]
template<typename T>
Publisher<T> as_publisher_deprecated(Flow<T>& flow) {
    return as_publisher(flow);
}

/** @suppress */
// @Deprecated(
//     message = "batchSize parameter is deprecated, use .buffer() instead to control the backpressure",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("asFlow().buffer(batchSize)", imports = ["kotlinx.coroutines.flow.*"])
// )
[[deprecated("batch_size parameter is deprecated, use .buffer() instead to control the backpressure")]]
template<typename T>
Flow<T> as_flow(Publisher<T>& publisher, int batch_size) {
    return as_flow(publisher).buffer(batch_size);
}

} // namespace reactive
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO List for semantic implementation:
 *
 * 1. Implement Flow<T> interface
 * 2. Implement Publisher<T> interface
 * 3. Implement as_flow() main function
 * 4. Implement as_publisher() main function
 * 5. Implement .buffer() operator on Flow
 */
