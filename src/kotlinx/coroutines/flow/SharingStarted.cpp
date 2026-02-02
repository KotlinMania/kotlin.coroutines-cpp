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

Flow<SharingCommand>* StartedEagerly::command(StateFlow<int>* subscription_count) {
    (void)subscription_count;  // Not used - always starts
    // Return a single-element flow with START
    // TODO: This leaks memory - need to manage Flow lifecycle properly
    return flow_of(SharingCommand::START).get();
}

std::string StartedEagerly::to_string() const {
    return "SharingStarted.Eagerly";
}

Flow<SharingCommand>* StartedLazily::command(StateFlow<int>* subscription_count) {
    // Implementation requires Flow builder infrastructure
    // For now, we create a simple flow that watches subscription_count
    // TODO: Implement using proper flow builder when available
    // TODO: This leaks memory - need to manage Flow lifecycle properly
    return flow_of(SharingCommand::START).get();  // Simplified: lazily acts like eagerly for now
}

std::string StartedLazily::to_string() const {
    return "SharingStarted.Lazily";
}

StartedWhileSubscribed::StartedWhileSubscribed(long long stop_timeout_millis,
                                               long long replay_expiration_millis)
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

Flow<SharingCommand>* StartedWhileSubscribed::command(StateFlow<int>* subscription_count) {
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

std::string StartedWhileSubscribed::to_string() const {
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

bool StartedWhileSubscribed::operator==(const StartedWhileSubscribed& other) const {
    return stop_timeout_ == other.stop_timeout_ &&
           replay_expiration_ == other.replay_expiration_;
}

std::size_t StartedWhileSubscribed::hash() const {
    return std::hash<long long>{}(stop_timeout_) * 31 +
           std::hash<long long>{}(replay_expiration_);
}

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
