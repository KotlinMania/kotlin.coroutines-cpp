#pragma once
/**
 * @file SharingStarted.hpp
 * @brief Sharing strategy for shareIn and stateIn operators
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/SharingStarted.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/StateFlow.hpp"
#include <limits>
#include <chrono>

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * A command emitted by SharingStarted implementations to control the sharing coroutine in
 * the share_in and state_in operators.
 *
 * Transliterated from:
 * public enum class SharingCommand
 */
enum class SharingCommand {
    /**
     * Starts sharing, launching collection of the upstream flow.
     *
     * Emitting this command again does not do anything. Emit STOP and then START to restart an
     * upstream flow.
     */
    START,

    /**
     * Stops sharing, cancelling collection of the upstream flow.
     */
    STOP,

    /**
     * Stops sharing, cancelling collection of the upstream flow, and resets the SharedFlow::replay_cache
     * to its initial state.
     * The share_in operator calls MutableSharedFlow::reset_replay_cache;
     * the state_in operator resets the value to its original `initial_value`.
     */
    STOP_AND_RESET_REPLAY_CACHE
};

/**
 * A strategy for starting and stopping the sharing coroutine in share_in and state_in operators.
 *
 * This interface provides a set of built-in strategies: Eagerly, Lazily, WhileSubscribed, and
 * supports custom strategies by implementing this interface's command function.
 *
 * For example, it is possible to define a custom strategy that starts the upstream only when the number
 * of subscribers exceeds the given threshold and make it an extension on SharingStarted so
 * that it looks like a built-in strategy on the use-site.
 *
 * ## Commands
 *
 * The SharingStarted strategy works by emitting commands that control upstream flow from its
 * command flow implementation function. Back-to-back emissions of the same command have no effect.
 * Only emission of a different command has effect:
 *
 * - START - the upstream flow is started.
 * - STOP - the upstream flow is stopped.
 * - STOP_AND_RESET_REPLAY_CACHE - the upstream flow is stopped and the SharedFlow::replay_cache
 *   is reset to its initial state.
 *
 * Initially, the upstream flow is stopped and is in the initial state, so the emission of additional
 * STOP and STOP_AND_RESET_REPLAY_CACHE commands will have no effect.
 *
 * The completion of the command flow normally has no effect (the upstream flow keeps running if it was running).
 * The failure of the command flow cancels the sharing coroutine and the upstream flow.
 *
 * Transliterated from:
 * public fun interface SharingStarted
 */
struct SharingStarted {
    virtual ~SharingStarted() = default;

    /**
     * Transforms the subscription_count state of the shared flow into the
     * flow of commands that control the sharing coroutine.
     *
     * Transliterated from:
     * public fun command(subscriptionCount: StateFlow<Int>): Flow<SharingCommand>
     */
    virtual Flow<SharingCommand>* command(StateFlow<int>* subscription_count) = 0;

    /**
     * Sharing is started immediately and never stops.
     *
     * Transliterated from:
     * public val Eagerly: SharingStarted = StartedEagerly()
     */
    static SharingStarted* eagerly();

    /**
     * Sharing is started when the first subscriber appears and never stops.
     *
     * Transliterated from:
     * public val Lazily: SharingStarted = StartedLazily()
     */
    static SharingStarted* lazily();

    /**
     * Sharing is started when the first subscriber appears, immediately stops when the last
     * subscriber disappears (by default), keeping the replay cache forever (by default).
     *
     * It has the following optional parameters:
     *
     * - stop_timeout_millis - configures a delay (in milliseconds) between the disappearance of the last
     *   subscriber and the stopping of the sharing coroutine. It defaults to zero (stop immediately).
     * - replay_expiration_millis - configures a delay (in milliseconds) between the stopping of
     *   the sharing coroutine and the resetting of the replay cache (which makes the cache empty for the share_in
     *   operator and resets the cached value to the original initial_value for the state_in operator).
     *   It defaults to LLONG_MAX (keep replay cache forever, never reset buffer).
     *   Use zero value to expire the cache immediately.
     *
     * This function throws std::invalid_argument when either stop_timeout_millis or replay_expiration_millis
     * are negative.
     *
     * Transliterated from:
     * public fun WhileSubscribed(
     *     stopTimeoutMillis: Long = 0,
     *     replayExpirationMillis: Long = Long.MAX_VALUE
     * ): SharingStarted
     */
    static SharingStarted* while_subscribed(
        long long stop_timeout_millis = 0,
        long long replay_expiration_millis = std::numeric_limits<long long>::max()
    );
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
