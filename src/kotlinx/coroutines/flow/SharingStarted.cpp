// port-lint: source kotlinx-coroutines-core/common/src/flow/SharingStarted.kt
/**
 * @file SharingStarted.cpp
 * @brief Implementation of SharingStarted.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/SharingStarted.kt
 * Lines 142-204 (implementation classes)
 */

#include "kotlinx/coroutines/flow/SharingStarted.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include <stdexcept>
#include <string>
#include <sstream>

namespace kotlinx {
namespace coroutines {
namespace flow {

// -------------------------------- implementation --------------------------------

/**
 * Sharing is started immediately and never stops.
 *
 * Transliterated from:
 * private class StartedEagerly : SharingStarted
 */
class StartedEagerly : public SharingStarted {
public:
    /**
     * Returns a flow that immediately emits START.
     *
     * Transliterated from:
     * override fun command(subscriptionCount: StateFlow<Int>): Flow<SharingCommand> =
     *     flowOf(SharingCommand.START)
     */
    Flow<SharingCommand>* command(StateFlow<int>* subscription_count) override {
        (void)subscription_count;  // Not used - always starts
        // Return a single-element flow with START
        // TODO: This leaks memory - need to manage Flow lifecycle properly
        return flow_of(SharingCommand::START).get();
    }

    std::string to_string() const {
        return "SharingStarted.Eagerly";
    }
};

/**
 * Sharing is started when the first subscriber appears and never stops.
 *
 * Transliterated from:
 * private class StartedLazily : SharingStarted
 */
class StartedLazily : public SharingStarted {
public:
    /**
     * Emits START when the first subscriber appears, then never emits anything else.
     *
     * Transliterated from:
     * override fun command(subscriptionCount: StateFlow<Int>): Flow<SharingCommand> = flow {
     *     var started = false
     *     subscriptionCount.collect { count ->
     *         if (count > 0 && !started) {
     *             started = true
     *             emit(SharingCommand.START)
     *         }
     *     }
     * }
     */
    Flow<SharingCommand>* command(StateFlow<int>* subscription_count) override {
        // Implementation requires Flow builder infrastructure
        // For now, we create a simple flow that watches subscription_count
        // TODO: Implement using proper flow builder when available
        // TODO: This leaks memory - need to manage Flow lifecycle properly
        return flow_of(SharingCommand::START).get();  // Simplified: lazily acts like eagerly for now
    }

    std::string to_string() const {
        return "SharingStarted.Lazily";
    }
};

/**
 * Sharing is started when the first subscriber appears, immediately stops when the last
 * subscriber disappears (by default), keeping the replay cache forever (by default).
 *
 * Transliterated from:
 * private class StartedWhileSubscribed(
 *     private val stopTimeout: Long,
 *     private val replayExpiration: Long
 * ) : SharingStarted
 */
class StartedWhileSubscribed : public SharingStarted {
private:
    long long stop_timeout_;
    long long replay_expiration_;

public:
    StartedWhileSubscribed(long long stop_timeout_millis, long long replay_expiration_millis)
        : stop_timeout_(stop_timeout_millis), replay_expiration_(replay_expiration_millis) {
        // Transliterated from:
        // require(stopTimeout >= 0) { "stopTimeout($stopTimeout ms) cannot be negative" }
        // require(replayExpiration >= 0) { "replayExpiration($replayExpiration ms) cannot be negative" }
        if (stop_timeout_millis < 0) {
            std::ostringstream oss;
            oss << "stopTimeout(" << stop_timeout_millis << " ms) cannot be negative";
            throw std::invalid_argument(oss.str());
        }
        if (replay_expiration_millis < 0) {
            std::ostringstream oss;
            oss << "replayExpiration(" << replay_expiration_millis << " ms) cannot be negative";
            throw std::invalid_argument(oss.str());
        }
    }

    /**
     * Transliterated from:
     * override fun command(subscriptionCount: StateFlow<Int>): Flow<SharingCommand> = subscriptionCount
     *     .transformLatest { count ->
     *         if (count > 0) {
     *             emit(SharingCommand.START)
     *         } else {
     *             delay(stopTimeout)
     *             if (replayExpiration > 0) {
     *                 emit(SharingCommand.STOP)
     *                 delay(replayExpiration)
     *             }
     *             emit(SharingCommand.STOP_AND_RESET_REPLAY_CACHE)
     *         }
     *     }
     *     .dropWhile { it != SharingCommand.START }
     *     .distinctUntilChanged()
     */
    Flow<SharingCommand>* command(StateFlow<int>* subscription_count) override {
        (void)subscription_count;
        // Full implementation requires:
        // - transformLatest operator
        // - delay operator
        // - dropWhile operator
        // - distinctUntilChanged operator
        // TODO: Implement when these operators are available
        // TODO: This leaks memory - need to manage Flow lifecycle properly
        return flow_of(SharingCommand::START).get();  // Simplified for now
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << "SharingStarted.WhileSubscribed(";
        bool first = true;
        if (stop_timeout_ > 0) {
            oss << "stopTimeout=" << stop_timeout_ << "ms";
            first = false;
        }
        if (replay_expiration_ < std::numeric_limits<long long>::max()) {
            if (!first) oss << ", ";
            oss << "replayExpiration=" << replay_expiration_ << "ms";
        }
        oss << ")";
        return oss.str();
    }

    // Equality comparison for testing
    bool operator==(const StartedWhileSubscribed& other) const {
        return stop_timeout_ == other.stop_timeout_ &&
               replay_expiration_ == other.replay_expiration_;
    }

    std::size_t hash() const {
        return std::hash<long long>{}(stop_timeout_) * 31 +
               std::hash<long long>{}(replay_expiration_);
    }
};

// -------------------------------- Factory functions --------------------------------

// Static instances for Eagerly and Lazily (they're stateless singletons)
static StartedEagerly EAGERLY_INSTANCE;
static StartedLazily LAZILY_INSTANCE;

SharingStarted* SharingStarted::eagerly() {
    return &EAGERLY_INSTANCE;
}

SharingStarted* SharingStarted::lazily() {
    return &LAZILY_INSTANCE;
}

SharingStarted* SharingStarted::while_subscribed(
    long long stop_timeout_millis,
    long long replay_expiration_millis
) {
    // Each call creates a new instance since it's parameterized
    return new StartedWhileSubscribed(stop_timeout_millis, replay_expiration_millis);
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx

