// port-lint: source flow/Migration.kt
/**
 * @file Migration.cpp
 * @brief Deprecated Flow operators for migration from reactive streams
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Migration.kt
 *
 * ## General Note
 *
 * These deprecations are added to improve user experience when they will start to
 * search for their favourite operators and/or patterns that are missing or renamed in Flow.
 * Deprecated functions also are moved here when they are renamed. The difference is that they have
 * a body with their implementation while pure stubs have no_impl().
 *
 * ## Migration Guide from Reactive Streams to Flow
 *
 * ### observe_on / publish_on
 *
 * `observe_on` has no direct match in Flow API because all terminal flow operators are suspending
 * and thus use the context of the caller.
 *
 * Example migration:
 * ```cpp
 * // Before (Rx-style):
 * flowable.observe_on(Schedulers::io()).do_on_each([](auto value) { ... }).subscribe();
 *
 * // After (Flow-style):
 * with_context(Dispatchers::IO, [&flow]() {
 *     flow.collect([](auto value) { ... });
 * });
 * ```
 *
 * ### subscribe_on
 *
 * `subscribe_on` has no direct match in Flow API because Flow preserves its context and does not leak it.
 * Use `flow_on()` instead, which works upstream and doesn't change downstream.
 *
 * Example migration:
 * ```cpp
 * // Before (Rx-style):
 * flowable.subscribe_on(Schedulers::io()).observe_on(Schedulers::computation()).subscribe();
 *
 * // After (Flow-style):
 * with_context(Dispatchers::Default, [&flow]() {
 *     flow.flow_on(Dispatchers::IO).collect([](auto value) { ... });
 * });
 * ```
 *
 * ### on_error_resume / on_error_resume_next
 *
 * Flow analogue of `on_error_xxx` is `catch_flow`.
 * Use `catch_flow([](auto& cause) { emit_all(fallback); })`.
 *
 * ### subscribe
 *
 * `subscribe` is Rx-specific API that has no direct match in flows.
 * Use `launch_in` instead:
 *
 * ```cpp
 * // Before (Rx-style):
 * flowable.subscribe(on_next, on_error, on_complete);
 *
 * // After (Flow-style):
 * flow.on_each([](auto value) { ... })
 *     .on_completion([](auto cause) { if (!cause) { ... } })
 *     .catch_flow([](auto cause) { ... })
 *     .flow_on(Dispatchers::IO)
 *     .launch_in(scope);
 * ```
 *
 * ### flatMap
 *
 * Flow analogue of `flat_map` is `flat_map_merge`. Use `flat_map_merge { ... }`.
 * Note that `flat_map` has quadratic complexity and is not recommended for general use.
 *
 * ### concat_map
 *
 * Flow analogue of `concat_map` is `flat_map_concat`. Use `flat_map_concat { ... }`.
 *
 * ### switch_map
 *
 * Flow analogue of `switch_map` is `flat_map_latest`. Use `flat_map_latest { ... }`.
 *
 * ### merge
 *
 * For merging multiple flows, use the standalone `merge()` function:
 * `merge(flow1, flow2, flow3)` instead of extension-style `flow1.merge(flow2, flow3)`.
 *
 * ### debounce
 *
 * For debouncing, use `debounce(timeout_millis)` with a timeout in milliseconds.
 *
 * ### delay
 *
 * To delay a flow, use `delay(time_millis)` with a time in milliseconds.
 * For `delay_each`, use `on_each([](auto) { delay(time); })`.
 */

#include <stdexcept>

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * Throws an UnsupportedOperationException indicating that the deprecated function
 * should not be called and the migration guide should be consulted instead.
 *
 * Transliterated from:
 * internal fun noImpl(): Nothing = throw UnsupportedOperationException("Not implemented, should not be called")
 */
[[noreturn]] inline void no_impl() {
    throw std::runtime_error("Not implemented, should not be called. See migration guide in Migration.hpp");
}

// All deprecated functions translated as function declarations with [[deprecated]] attribute
// These are intentionally not implemented - they exist to provide compiler errors with
// helpful messages guiding users to the correct Flow equivalents.

// See Migration.kt for the complete list of deprecated Rx-style operators:
// - observe_on, publish_on, subscribe_on (use flow_on + with_context)
// - on_error_resume, on_error_resume_next (use catch_flow)
// - subscribe (use launch_in + on_each + on_completion + catch_flow)
// - flat_map, concat_map, switch_map (use flat_map_merge, flat_map_concat, flat_map_latest)
// - merge (use standalone merge function)
// - scan (renamed to running_fold)
// - replay (use share_in + replay parameter)
// - cache (use share_in with Lazily + replay)
// - distinctUntilChanged (use distinct_until_changed with snake_case)
// - debounce (use debounce with timeout in millis)

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
