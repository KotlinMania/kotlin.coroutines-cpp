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
#include <string>
#include <stdexcept>
#include <functional>

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

// ============================================================================
// Implementation classes (Lines 142-204)
// ============================================================================

/**
 * Line 142-146: StartedEagerly
 *
 * Sharing is started immediately and never stops.
 * Returns a flow that emits START immediately.
 */
class StartedEagerly : public SharingStarted {
public:
    Flow<SharingCommand>* command(StateFlow<int>* subscription_count) override {
        // Returns flow that emits START immediately
        // TODO: Implement flow_of(SharingCommand::START)
        return nullptr;
    }

    std::string to_string() const {
        return "SharingStarted.Eagerly";
    }
};

/**
 * Line 148-160: StartedLazily
 *
 * Sharing is started when the first subscriber appears and never stops.
 * Returns a flow that emits START when subscription count first becomes > 0.
 */
class StartedLazily : public SharingStarted {
public:
    Flow<SharingCommand>* command(StateFlow<int>* subscription_count) override {
        // Returns flow that emits START when first subscriber appears
        // Implementation: collect subscription_count, emit START when count > 0 and not started
        // TODO: Implement with proper flow builder
        return nullptr;
    }

    std::string to_string() const {
        return "SharingStarted.Lazily";
    }
};

/**
 * Line 162-204: StartedWhileSubscribed
 *
 * Sharing is started when the first subscriber appears, immediately stops when the last
 * subscriber disappears (by default), keeping the replay cache forever (by default).
 *
 * @param stop_timeout_millis Delay before stopping after last subscriber disappears
 * @param replay_expiration_millis Delay before resetting replay cache after stopping
 */
class StartedWhileSubscribed : public SharingStarted {
public:
    explicit StartedWhileSubscribed(
        long long stop_timeout_millis = 0,
        long long replay_expiration_millis = std::numeric_limits<long long>::max()
    ) : stop_timeout_(stop_timeout_millis)
      , replay_expiration_(replay_expiration_millis)
    {
        if (stop_timeout_ < 0) {
            throw std::invalid_argument(
                "stopTimeout cannot be negative");
        }
        if (replay_expiration_ < 0) {
            throw std::invalid_argument(
                "replayExpiration cannot be negative");
        }
    }

    Flow<SharingCommand>* command(StateFlow<int>* subscription_count) override {
        // Implementation using transformLatest:
        // - When count > 0: emit START
        // - When count == 0: delay(stop_timeout), emit STOP,
        //                    delay(replay_expiration), emit STOP_AND_RESET_REPLAY_CACHE
        // Then dropWhile not START, distinctUntilChanged
        // TODO: Implement with proper flow operators
        return nullptr;
    }

    std::string to_string() const {
        std::string result = "SharingStarted.WhileSubscribed(";
        bool has_params = false;
        if (stop_timeout_ > 0) {
            result += "stopTimeout=" + std::to_string(stop_timeout_) + "ms";
            has_params = true;
        }
        if (replay_expiration_ < std::numeric_limits<long long>::max()) {
            if (has_params) result += ", ";
            result += "replayExpiration=" + std::to_string(replay_expiration_) + "ms";
        }
        result += ")";
        return result;
    }

    // Equality operators for testing
    bool operator==(const StartedWhileSubscribed& other) const {
        return stop_timeout_ == other.stop_timeout_ &&
               replay_expiration_ == other.replay_expiration_;
    }

    size_t hash() const {
        return std::hash<long long>{}(stop_timeout_) * 31 +
               std::hash<long long>{}(replay_expiration_);
    }

private:
    long long stop_timeout_;
    long long replay_expiration_;
};

// ============================================================================
// Static factory implementations
// ============================================================================

inline SharingStarted* SharingStarted::eagerly() {
    static StartedEagerly instance;
    return &instance;
}

inline SharingStarted* SharingStarted::lazily() {
    static StartedLazily instance;
    return &instance;
}

inline SharingStarted* SharingStarted::while_subscribed(
    long long stop_timeout_millis,
    long long replay_expiration_millis
) {
    return new StartedWhileSubscribed(stop_timeout_millis, replay_expiration_millis);
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx

