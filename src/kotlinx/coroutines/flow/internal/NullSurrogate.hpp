#pragma once
// port-lint: source flow/internal/NullSurrogate.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/NullSurrogate.kt
 */

#include "kotlinx/coroutines/internal/Symbol.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

/**
 * This value is used as a surrogate `nullptr` value when needed.
 * It should never leak to the outside world.
 * Its usage typically is paired with Symbol.unbox usages.
 */
inline ::kotlinx::coroutines::internal::Symbol& NULL_VALUE() {
    static ::kotlinx::coroutines::internal::Symbol instance("NULL");
    return instance;
}

/**
 * Symbol used to indicate that the value is not yet initialized.
 * It should never leak to the outside world.
 */
inline ::kotlinx::coroutines::internal::Symbol& UNINITIALIZED() {
    static ::kotlinx::coroutines::internal::Symbol instance("UNINITIALIZED");
    return instance;
}

/**
 * Symbol used to indicate that the flow is complete.
 * It should never leak to the outside world.
 */
inline ::kotlinx::coroutines::internal::Symbol& DONE() {
    static ::kotlinx::coroutines::internal::Symbol instance("DONE");
    return instance;
}

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
