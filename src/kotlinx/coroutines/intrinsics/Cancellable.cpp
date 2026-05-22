/**
 * Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.intrinsics
 *   imports: kotlinx.coroutines.*, kotlinx.coroutines.internal.*
 *
 * Cancellable coroutine start entries. The `startCoroutineCancellable` overloads and
 * `runSafely` exception wrapper live in the matching header
 * (intrinsics/Cancellable.hpp); this translation unit owns `dispatcher_failure`, the
 * Kotlin `dispatcherFailure` actual.
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
