/**
 * @file JavaFxDispatcher.cpp
 * @brief Implementation of JavaFxDispatcher.
 *
 * NOTE: The detailed API documentation, KDocs, and declarations are located
 * in the companion header file: `include/kotlinx/coroutines/javafx/JavaFxDispatcher.hpp`.
 */

#include "kotlinx/coroutines/javafx/JavaFxDispatcher.hpp"

namespace kotlinx {
namespace coroutines {
namespace javafx {

void JavaFxDispatcher::dispatch(CoroutineContext& context, Runnable block) {
    // Stub: simulate platform runLater
    if (block) block();
}

JavaFxDispatcher& JavaFx::get_dispatcher() {
    static JavaFxDispatcher instance;
    return instance;
}

} // namespace javafx
} // namespace coroutines
} // namespace kotlinx
