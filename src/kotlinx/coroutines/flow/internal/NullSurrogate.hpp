#pragma once
// port-lint: source flow/internal/NullSurrogate.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/NullSurrogate.kt */
#include "kotlinx/coroutines/internal/Symbol.hpp"

namespace kotlinx::coroutines::flow::internal {
/**
 * This value is used as a surrogate null value when needed.
 * It should never leak to the outside world.
 * Its usages are typically paired with Symbol.unbox.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/NullSurrogate.kt:12-12
// NOTE(port): NULL is a C++ macro. This accessor exposes the actual singleton
// through the existing C++ binding; its construction lives in .cpp.
kotlinx::coroutines::internal::Symbol& NULL_VALUE();

/** Symbol indicating that a value is not yet initialized. It must not leak outside. */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/NullSurrogate.kt:19-19
kotlinx::coroutines::internal::Symbol& UNINITIALIZED();

/** Symbol indicating that a flow is complete. It must not leak outside. */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/NullSurrogate.kt:26-26
kotlinx::coroutines::internal::Symbol& DONE();
} // namespace kotlinx::coroutines::flow::internal
