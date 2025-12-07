// Transliterated from: reactive/kotlinx-coroutines-reactor/src/Migration.cpp

// @file:JvmName("FlowKt")

// TODO: #include <kotlinx/coroutines/flow.hpp>
// TODO: #include <reactor/core/publisher.hpp>

namespace kotlinx { namespace coroutines { namespace reactor {

/** @suppress **/
// @Deprecated
//     message = "Replaced in favor of ReactiveFlow extension, please import kotlinx.coroutines.reactor.* instead of kotlinx.coroutines.reactor.FlowKt",
//     level = DeprecationLevel.HIDDEN
// Compatibility with Spring 5.2-RC
// @JvmName("asFlux")
template<typename T>
Flux<T> as_flux_deprecated(Flow<T>* flow) {
    return as_flux(flow);
}

} } } // namespace kotlinx::coroutines::reactor

// TODO: Semantic implementation tasks:
// 1. Implement Flow<T> type
// 2. Implement Flux<T> type
// 3. Implement as_flux() function
// 4. Handle JvmName annotation for name mangling
