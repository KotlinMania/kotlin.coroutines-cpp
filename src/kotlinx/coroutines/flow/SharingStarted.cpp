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
#include "kotlinx/coroutines/flow/Merge.hpp"
#include "kotlinx/coroutines/flow/Context.hpp"
#include "kotlinx/coroutines/flow/Limit.hpp"
#include "kotlinx/coroutines/flow/Distinct.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <functional>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace kotlinx::coroutines::flow {

namespace {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/SharingStarted.kt:171-185
class WhileSubscribedCommandFrame final : public ContinuationImpl {
public:
    WhileSubscribedCommandFrame(FlowCollector<SharingCommand>* sink,
                                long long stop_timeout, long long replay_expiration,
                                Continuation<void*>* completion)
        : ContinuationImpl(std::shared_ptr<Continuation<void*>>(
              completion, [](Continuation<void*>*) {})),
          sink_(sink), stop_timeout_(stop_timeout), replay_expiration_(replay_expiration) {}

    void retain() { self_ref_ = shared_from_this(); }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/SharingStarted.kt:148-185
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            coroutine_yield(this, delay(stop_timeout_, shared_from_this()));
            if (replay_expiration_ > 0) {
                coroutine_yield(this, sink_->emit(SharingCommand::STOP, this));
                coroutine_yield(this, delay(replay_expiration_, shared_from_this()));
            }
            coroutine_yield(this, sink_->emit(SharingCommand::STOP_AND_RESET_REPLAY_CACHE, this));
            self_ref_.reset();
            coroutine_end(this)
        } catch (...) {
            self_ref_.reset();
            throw;
        }
    }

private:
    void* _label = nullptr;
    FlowCollector<SharingCommand>* sink_;
    long long stop_timeout_;
    long long replay_expiration_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/SharingStarted.kt:149-158
class LazyCommandFrame final : public ContinuationImpl, public FlowCollector<int> {
public:
    LazyCommandFrame(std::shared_ptr<StateFlow<int>> counts, FlowCollector<SharingCommand>* sink,
                     Continuation<void*>* completion)
        : ContinuationImpl(std::shared_ptr<Continuation<void*>>(completion, [](Continuation<void*>*) {})),
          counts_(std::move(counts)), sink_(sink) {}

    void retain() { self_ref_ = shared_from_this(); }

    void* emit(int count, Continuation<void*>* completion) override {
        if (count > 0 && !started_) {
            started_ = true;
            return sink_->emit(SharingCommand::START, completion);
        }
        return nullptr;
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/SharingStarted.kt:148-185
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            coroutine_yield(this, counts_->collect(this, this));
            self_ref_.reset();
            coroutine_end(this)
        } catch (...) {
            self_ref_.reset();
            throw;
        }
    }

private:
    void* _label = nullptr;
    std::shared_ptr<StateFlow<int>> counts_;
    FlowCollector<SharingCommand>* sink_;
    bool started_ = false;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};

} // namespace

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
// Transliterated from: kotlinx-coroutines-core/common/src/flow/SharingStarted.kt:148-158
std::shared_ptr<Flow<SharingCommand>> StartedLazily::command(
    std::shared_ptr<StateFlow<int>> subscription_count) {
    return flow<SharingCommand>(
        [subscription_count](FlowCollector<SharingCommand>* sink,
                             Continuation<void*>* cont) -> void* {
            auto frame = std::make_shared<LazyCommandFrame>(subscription_count, sink, cont);
            frame->retain();
            return frame->invoke_suspend(Result<void*>::success(nullptr));
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
// Transliterated from: kotlinx-coroutines-core/common/src/flow/SharingStarted.kt:171-185
std::shared_ptr<Flow<SharingCommand>> StartedWhileSubscribed::command(
    std::shared_ptr<StateFlow<int>> subscription_count) {
    const long long stop_timeout = stop_timeout_;
    const long long replay_expiration = replay_expiration_;
    auto staged = transform_latest<int, SharingCommand>(
        subscription_count,
        [stop_timeout, replay_expiration](
            FlowCollector<SharingCommand>* sink, int count,
            Continuation<void*>* cont) -> void* {
            if (count > 0) {
                return sink->emit(SharingCommand::START, cont);
            }
            auto frame = std::make_shared<WhileSubscribedCommandFrame>(
                sink, stop_timeout, replay_expiration, cont);
            frame->retain();
            return frame->invoke_suspend(Result<void*>::success(nullptr));
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

} // namespace kotlinx::coroutines::flow
