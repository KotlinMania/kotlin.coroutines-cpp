#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace javafx {

using Runnable = std::function<void()>;

class JavaFxDispatcher : public CoroutineDispatcher {
public:
    void dispatch(CoroutineContext& context, Runnable block) override;
};

class JavaFx {
public:
    static JavaFxDispatcher& get_dispatcher();
};

} // namespace javafx
} // namespace coroutines
} // namespace kotlinx
