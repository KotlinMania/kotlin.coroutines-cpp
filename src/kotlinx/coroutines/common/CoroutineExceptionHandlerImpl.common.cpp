/**
 * @file CoroutineExceptionHandlerImpl.common.cpp
 * @brief Implementation of CoroutineExceptionHandler infrastructure.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/CoroutineExceptionHandler.hpp`.
 */

#include "kotlinx/coroutines/CoroutineExceptionHandler.hpp"

// NOTE:
// `handle_coroutine_exception(...)` is defined inline in the public header.
// This common/internal TU is kept for source parity only and must not emit
// another out-of-line definition.