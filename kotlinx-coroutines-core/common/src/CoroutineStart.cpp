/**
 * @file CoroutineStart.cpp
 * @brief Implementation of CoroutineStart helper functions.
 *
 * NOTE: The enum definition is in `include/kotlinx/coroutines/CoroutineStart.hpp`.
 */

#include "kotlinx/coroutines/CoroutineStart.hpp"

namespace kotlinx {
namespace coroutines {

bool is_lazy(CoroutineStart start) {
    return start == CoroutineStart::LAZY;
}

} // namespace coroutines
} // namespace kotlinx