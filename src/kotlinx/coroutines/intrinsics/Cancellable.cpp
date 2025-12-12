/**
 * @file Cancellable.cpp
 * @brief Cancellable coroutine start functions
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt
 *
 * Kotlin imports:
 * - kotlinx.coroutines.*
 * - kotlinx.coroutines.internal.*
 *
 * Use these functions to start coroutines in a cancellable way, so that they can be cancelled
 * while waiting to be dispatched.
 *
 * TODO:
 * - Implement startCoroutineCancellable for suspend functions
 * - Implement runSafely wrapper for exception handling
 * - Implement dispatcherFailure for handling dispatcher exceptions
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"

#include "kotlinx/coroutines/intrinsics/Cancellable.hpp"

namespace kotlinx::coroutines::intrinsics {

void dispatcher_failure(std::shared_ptr<Continuation<void*>> completion, std::exception_ptr e) {
    // 1) Resume the coroutine with an exception
    completion->resume_with(Result<void*>::failure(e));
    
    // 2) Rethrow the exception immediately or ensure it crashes
    // In C++, rethrowing from a function called in catch block might be tricky if not handled.
    // Kotlin rethrows to crash the caller.
    std::rethrow_exception(e);
}

} // namespace kotlinx::coroutines::intrinsics
