#pragma once
/**
 * @file DebugCoroutineInfo.hpp
 * @brief Internal structure to hold debug information about a coroutine.
 */

#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/internal/CoroutineStackFrame.hpp"
#include <string>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace debug {
namespace internal {

    struct DebugCoroutineInfo {
        std::shared_ptr<CoroutineContext> context;
        kotlinx::coroutines::internal::CoroutineStackFrame* creation_stack_bottom = nullptr;
        kotlinx::coroutines::internal::CoroutineStackFrame* last_observed_frame = nullptr;
        std::string state;
    };

} // namespace internal
} // namespace debug
} // namespace coroutines
} // namespace kotlinx
