#pragma once
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/internal/FlowCoroutine.hpp"
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace flow {

// flowScope builder
template <typename R>
R flow_scope(std::function<R(CoroutineScope&)> block) {
    // Implementation stub - requires suspend support
    // suspendCoroutineUninterceptedOrReturn structure
    return R();
}

// scopedFlow builder
template <typename R>
std::shared_ptr<Flow<R>> scoped_flow(std::function<void(CoroutineScope&, std::shared_ptr<FlowCollector<R>>)> block) {
    // Implementation stub
    return nullptr;
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
