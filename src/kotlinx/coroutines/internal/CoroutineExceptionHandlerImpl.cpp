// port-lint: source kotlinx-coroutines-core/native/src/internal/CoroutineExceptionHandlerImpl.kt
/**
 * @file CoroutineExceptionHandlerImpl.cpp
 * @brief Native platform implementation of exception handler
 *
 * Transliterated from: kotlinx-coroutines-core/native/src/internal/CoroutineExceptionHandlerImpl.kt
 *
 * Platform-specific (native) implementation of coroutine exception handling.
 * The common functionality is in CoroutineExceptionHandlerImpl.hpp.
 */

#include "kotlinx/coroutines/internal/CoroutineExceptionHandlerImpl.hpp"

namespace kotlinx {
namespace coroutines {
namespace internal {

// Native platform has minimal additional implementation beyond the common code.
// All functionality is provided by the header's inline functions.

} // namespace internal
} // namespace coroutines
} // namespace kotlinx