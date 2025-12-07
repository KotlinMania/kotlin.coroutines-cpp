#pragma once
#include <functional>
#include "kotlinx/coroutines/core_fwd.hpp"

namespace kotlinx {
namespace coroutines {

enum class CoroutineStart {
    DEFAULT,
    LAZY,
    ATOMIC,
    UNDISPATCHED
};

// Start helpers, usually extension functions in Kotlin, here free functions or static methods
// We can template them or just provide stubs

} // namespace coroutines
} // namespace kotlinx
