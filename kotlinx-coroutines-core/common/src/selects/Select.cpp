/**
 * @file Select.cpp
 * @brief Implementation of Select.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/selects/Select.hpp`.
 */

#include "kotlinx/coroutines/selects/Select.hpp"

namespace kotlinx {
namespace coroutines {
namespace selects {

// Template implementations are in the header.

template<typename R>
void SelectBuilder<R>::on_timeout(long time_millis, std::function<R()> block) {
    // Stub
}

} // namespace selects
} // namespace coroutines
} // namespace kotlinx
