/**
 * @file SharedFlow.cpp
 * @brief SharedFlow and MutableSharedFlow implementations
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/SharedFlow.kt
 *
 * TODO:
 * - Full SharedFlow implementation
 * - MutableSharedFlow implementation
 * - StateFlow implementation
 * - Replay cache management
 * - Subscription counting
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            /**
 * A hot Flow that shares emitted values among all its collectors in a broadcast fashion,
 * so that all collectors get all emitted values.
 *
 * A shared flow never completes. A call to Flow::collect() on a shared flow never completes normally.
 *
 * See the SharedFlow documentation in Kotlin for full details.
 */
            template<typename T>
            class SharedFlow : public Flow<T> {
            public:
                virtual ~SharedFlow() = default;

                /**
     * A snapshot of the replay cache.
     */
                virtual std::vector<T> get_replay_cache() const = 0;
            };

            /**
 * A mutable SharedFlow that provides functions to emit values to the flow.
 *
 * MutableSharedFlow is a SharedFlow that also provides the abilities to emit a value,
 * to try_emit without suspension if possible, to track the subscription_count,
 * and to reset_replay_cache.
 */
            template<typename T>
            class MutableSharedFlow : public SharedFlow<T>, public FlowCollector<T> {
            public:
                virtual ~MutableSharedFlow() = default;

                /**
     * Tries to emit a value to this shared flow without suspending.
     * Returns true if the value was emitted successfully.
     */
                virtual bool try_emit(T value) = 0;

                /**
     * The number of active subscribers (collectors) to this shared flow.
     */
                virtual int get_subscription_count() const = 0;

                /**
     * Resets the replay cache of this shared flow to an empty state.
     */
                virtual void reset_replay_cache() = 0;
            };

            /**
 * A SharedFlow that represents a state with a single updatable data value.
 * A state flow is a hot flow because its active instance exists independently of
 * the presence of collectors.
 */
            template<typename T>
            class StateFlow : public SharedFlow<T> {
            public:
                virtual ~StateFlow() = default;

                /**
     * The current value of this state flow.
     */
                virtual T get_value() const = 0;
            };

            /**
 * A mutable StateFlow that provides a setter for value.
 */
            template<typename T>
            class MutableStateFlow : public StateFlow<T>, public MutableSharedFlow<T> {
            public:
                virtual ~MutableStateFlow() = default;

                /**
     * Sets the current value of this state flow.
     */
                virtual void set_value(T value) = 0;

                /**
     * Atomically compares the current value with expect and sets it to update if equal.
     * Returns true if successful.
     */
                virtual bool compare_and_set(T expect, T update) = 0;
            };

            // TODO: Factory functions
            // template<typename T>
            // std::shared_ptr<MutableSharedFlow<T>> MutableSharedFlow(
            //     int replay = 0,
            //     int extraBufferCapacity = 0,
            //     BufferOverflow onBufferOverflow = BufferOverflow::SUSPEND
            // );

            // template<typename T>
            // std::shared_ptr<MutableStateFlow<T>> MutableStateFlow(T value);
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx