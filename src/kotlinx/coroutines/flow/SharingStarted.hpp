#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/StateFlow.hpp"
#include <limits>
#include <chrono>

namespace kotlinx {
namespace coroutines {
namespace flow {

enum class SharingCommand {
    START,
    STOP,
    STOP_AND_RESET_REPLAY_CACHE
};

// Interface for sharing strategies
struct SharingStarted {
    virtual ~SharingStarted() = default;
    
    // Returns a flow of commands to control the upstream execution
    virtual Flow<SharingCommand>* command(StateFlow<int>* subscription_count) = 0;
    
    // Static factories
    static SharingStarted* eagerly();
    static SharingStarted* lazily();
    static SharingStarted* while_subscribed(
        long long stop_timeout_millis = 0,
        long long replay_expiration_millis = std::numeric_limits<long long>::max()
    );
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
