// port-lint: source kotlinx-coroutines-core/common/src/flow/SharingStarted.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/SharingStarted.kt
 * (private classes StartedEagerly, StartedLazily, StartedWhileSubscribed; lines 142-204)
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.flow
 */

#include "kotlinx/coroutines/flow/SharingStarted.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/StateFlow.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <functional>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace kotlinx::coroutines::flow {

/**
 * Upstream:
 *   private class StartedEagerly : SharingStarted {
 *       override fun command(subscriptionCount: StateFlow<Int>): Flow<SharingCommand> =
 *           flowOf(SharingCommand.START)
 *   }
 */
std::shared_ptr<Flow<SharingCommand>> StartedEagerly::command(
    std::shared_ptr<StateFlow<int>> /*subscription_count*/) {
    return flow_of<SharingCommand>({SharingCommand::START});
}

std::string StartedEagerly::to_string() const {
    return "SharingStarted.Eagerly";
}

/**
 * Upstream:
 *   private class StartedLazily : SharingStarted {
 *       override fun command(subscriptionCount: StateFlow<Int>): Flow<SharingCommand> = flow {
 *           var started = false
 *           subscriptionCount.collect { count ->
 *               if (count > 0 && !started) {
 *                   started = true
 *                   emit(SharingCommand.START)
 *               }
 *           }
 *       }
 *   }
 */
std::shared_ptr<Flow<SharingCommand>> StartedLazily::command(
    std::shared_ptr<StateFlow<int>> subscription_count) {
    return flow_builder<SharingCommand>(
        [subscription_count](FlowCollector<SharingCommand>* sink,
                             std::shared_ptr<Continuation<void*>> cont) -> void* {
            auto started = std::make_shared<bool>(false);
            return subscription_count->collect_each(
                [sink, started](int count) {
                    if (count > 0 && !*started) {
                        *started = true;
                        sink->emit(SharingCommand::START, nullptr);
                    }
                },
                cont.get());
        });
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

/**
 * Upstream:
 *   override fun command(subscriptionCount: StateFlow<Int>): Flow<SharingCommand> =
 *       subscriptionCount
 *           .transformLatest { count ->
 *               if (count > 0) emit(SharingCommand.START)
 *               else {
 *                   delay(stopTimeout)
 *                   if (replayExpiration > 0) { emit(STOP); delay(replayExpiration) }
 *                   emit(STOP_AND_RESET_REPLAY_CACHE)
 *               }
 *           }
 *           .dropWhile { it != SharingCommand.START }
 *           .distinctUntilChanged()
 */
std::shared_ptr<Flow<SharingCommand>> StartedWhileSubscribed::command(
    std::shared_ptr<StateFlow<int>> subscription_count) {
    const long long stop_timeout = stop_timeout_;
    const long long replay_expiration = replay_expiration_;
    auto staged = transform_latest<int, SharingCommand>(
        subscription_count,
        [stop_timeout, replay_expiration](
            FlowCollector<SharingCommand>* sink, int count,
            std::shared_ptr<Continuation<void*>> cont) -> void* {
            if (count > 0) {
                return sink->emit(SharingCommand::START, cont.get());
            }
            ::kotlinx::coroutines::delay(stop_timeout, cont.get());
            if (replay_expiration > 0) {
                sink->emit(SharingCommand::STOP, cont.get());
                ::kotlinx::coroutines::delay(replay_expiration, cont.get());
            }
            return sink->emit(SharingCommand::STOP_AND_RESET_REPLAY_CACHE, cont.get());
        });
    auto dropped = drop_while<SharingCommand>(
        staged,
        [](SharingCommand value) { return value != SharingCommand::START; });
    return distinct_until_changed<SharingCommand>(dropped);
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
