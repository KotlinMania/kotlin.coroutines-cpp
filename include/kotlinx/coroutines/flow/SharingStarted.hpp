#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include <string>
#include <memory>
#include <chrono>

namespace kotlinx {
namespace coroutines {
namespace flow {

enum class SharingCommand {
    START,
    STOP,
    STOP_AND_RESET_REPLAY_CACHE
};

struct SharingStarted {
    virtual ~SharingStarted() = default;

    // TODO: command method returning Flow<SharingCommand>
    // virtual std::shared_ptr<Flow<SharingCommand>> command(std::shared_ptr<StateFlow<int>> subscriptionCount) = 0;

    static std::shared_ptr<SharingStarted> Eagerly();
    static std::shared_ptr<SharingStarted> Lazily();
    static std::shared_ptr<SharingStarted> WhileSubscribed(int64_t stopTimeoutMillis = 0, int64_t replayExpirationMillis = -1);
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
