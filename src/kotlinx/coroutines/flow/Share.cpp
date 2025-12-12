/**
 * @file Share.cpp
 * @brief Flow sharing operators: shareIn, stateIn, asSharedFlow, asStateFlow, onSubscription
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Share.kt
 *
 * TODO:
 * - Implement SharingConfig for flow fusion
 * - Implement launchSharing coroutine
 * - Implement ReadonlySharedFlow and ReadonlyStateFlow wrappers
 * - Implement onSubscription operator
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/SharedFlow.hpp"
#include "kotlinx/coroutines/flow/StateFlow.hpp"
#include "kotlinx/coroutines/flow/SharingStarted.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            using channels::Channel;
            using channels::BufferOverflow;

            // -------------------------------- shareIn --------------------------------

            /**
 * Configuration for sharing upstream flow.
 *
 * @tparam T The type of values in the flow
 */
            template<typename T>
            struct SharingConfig {
                std::shared_ptr<Flow<T> > upstream;
                int extra_buffer_capacity;
                BufferOverflow on_buffer_overflow;
                std::shared_ptr<CoroutineContext> context;

                SharingConfig(
                    std::shared_ptr<Flow<T> > upstream_,
                    int extra_buffer_capacity_,
                    BufferOverflow on_buffer_overflow_,
                    std::shared_ptr<CoroutineContext> context_
                ) : upstream(upstream_),
                    extra_buffer_capacity(extra_buffer_capacity_),
                    on_buffer_overflow(on_buffer_overflow_),
                    context(context_) {
                }
            };

            /**
 * Converts a _cold_ Flow into a _hot_ SharedFlow that is started in the given coroutine scope,
 * sharing emissions from a single running instance of the upstream flow with multiple downstream subscribers,
 * and replaying a specified number of replay values to new subscribers. See the SharedFlow documentation
 * for the general concepts of shared flows.
 *
 * The starting of the sharing coroutine is controlled by the started parameter. The following options
 * are supported.
 *
 * - Eagerly - the upstream flow is started even before the first subscriber appears. Note
 *   that in this case all values emitted by the upstream beyond the most recent values as specified by
 *   replay parameter **will be immediately discarded**.
 * - Lazily - starts the upstream flow after the first subscriber appears, which guarantees
 *   that this first subscriber gets all the emitted values, while subsequent subscribers are only guaranteed to
 *   get the most recent replay values. The upstream flow continues to be active even when all subscribers
 *   disappear, but only the most recent replay values are cached without subscribers.
 * - WhileSubscribed() - starts the upstream flow when the first subscriber
 *   appears, immediately stops when the last subscriber disappears, keeping the replay cache forever.
 *   It has additional optional configuration parameters as explained in its documentation.
 * - A custom strategy can be supplied by implementing the SharingStarted interface.
 *
 * @param scope the coroutine scope in which sharing is started.
 * @param started the strategy that controls when sharing is started and stopped.
 * @param replay the number of values replayed to new subscribers (cannot be negative, defaults to zero).
 *
 * TODO: Implement proper coroutine-based sharing
 */
            template<typename T>
            std::shared_ptr<SharedFlow<T> > share_in(
                std::shared_ptr<Flow<T> > upstream,
                CoroutineScope *scope,
                SharingStarted *started,
                int replay = 0
            ) {
                // TODO: Implement proper sharing with coroutine scope and started strategy
                // For now, return a simple MutableSharedFlow
                auto shared = std::make_shared<MutableSharedFlow<T> >(replay);
                // TODO: Launch sharing coroutine
                return shared;
            }

            // -------------------------------- stateIn --------------------------------

            /**
 * Converts a _cold_ Flow into a _hot_ StateFlow that is started in the given coroutine scope,
 * sharing the most recently emitted value from a single running instance of the upstream flow with multiple
 * downstream subscribers. See the StateFlow documentation for the general concepts of state flows.
 *
 * The starting of the sharing coroutine is controlled by the started parameter, as explained in the
 * documentation for shareIn operator.
 *
 * @param scope the coroutine scope in which sharing is started.
 * @param started the strategy that controls when sharing is started and stopped.
 * @param initial_value the initial value of the state flow.
 *
 * TODO: Implement proper coroutine-based state sharing
 */
            template<typename T>
            std::shared_ptr<StateFlow<T> > state_in(
                std::shared_ptr<Flow<T> > upstream,
                CoroutineScope *scope,
                SharingStarted *started,
                T initial_value
            ) {
                // TODO: Implement proper state sharing with coroutine scope and started strategy
                auto state = std::make_shared<MutableStateFlow<T> >(initial_value);
                // TODO: Launch sharing coroutine
                return state;
            }

            /**
 * Starts the upstream flow in a given scope, suspends until the first value is emitted, and returns a _hot_
 * StateFlow of future emissions, sharing the most recently emitted value from this running instance of the upstream flow
 * with multiple downstream subscribers. See the StateFlow documentation for the general concepts of state flows.
 *
 * @param scope the coroutine scope in which sharing is started.
 * @throws NoSuchElementException if the upstream flow does not emit any value.
 *
 * TODO: Implement suspending stateIn
 */
            template<typename T>
            std::shared_ptr<StateFlow<T> > state_in(
                std::shared_ptr<Flow<T> > upstream,
                CoroutineScope *scope
            ) {
                // TODO: Implement suspending stateIn that waits for first value
                throw std::runtime_error("Suspending stateIn not yet implemented");
            }

            // -------------------------------- asSharedFlow/asStateFlow --------------------------------

            /**
 * Represents this mutable shared flow as a read-only shared flow.
 */
            template<typename T>
            std::shared_ptr<SharedFlow<T> > as_shared_flow(std::shared_ptr<MutableSharedFlow<T> > mutable_flow) {
                // MutableSharedFlow already extends SharedFlow, so just return as base type
                return mutable_flow;
            }

            /**
 * Represents this mutable state flow as a read-only state flow.
 */
            template<typename T>
            std::shared_ptr<StateFlow<T> > as_state_flow(std::shared_ptr<MutableStateFlow<T> > mutable_flow) {
                // MutableStateFlow already extends StateFlow, so just return as base type
                return mutable_flow;
            }

            // -------------------------------- onSubscription --------------------------------

            /**
 * Returns a flow that invokes the given action **after** this shared flow starts to be collected
 * (after the subscription is registered).
 *
 * The action is called before any value is emitted from the upstream
 * flow to this subscription but after the subscription is established. It is guaranteed that all emissions to
 * the upstream flow that happen inside or immediately after this onSubscription action will be
 * collected by this subscription.
 *
 * The receiver of the action is FlowCollector, so onSubscription can emit additional elements.
 *
 * TODO: Implement onSubscription operator
 */
            template<typename T>
            std::shared_ptr<SharedFlow<T> > on_subscription(
                std::shared_ptr<SharedFlow<T> > shared_flow,
                std::function<void(FlowCollector<T> *)> action
            ) {
                // TODO: Implement SubscribedSharedFlow wrapper
                return shared_flow;
            }

            // Explicit template instantiations for common types
            // (Add more as needed for specific applications)
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx