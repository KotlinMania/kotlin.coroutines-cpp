#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/MainCoroutineDispatcher.hpp"
#include <string>
#include <optional>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace android {

// Forward declarations
class Handler;

class HandlerDispatcher : public MainCoroutineDispatcher, public Delay {
public:
    virtual HandlerDispatcher& get_immediate() = 0;
};

HandlerDispatcher* as_coroutine_dispatcher(Handler& handler, const std::optional<std::string>& name = std::nullopt);

} // namespace android
} // namespace coroutines
} // namespace kotlinx
