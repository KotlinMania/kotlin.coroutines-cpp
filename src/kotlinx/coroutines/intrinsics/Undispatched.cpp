// port-lint: source intrinsics/Undispatched.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Undispatched.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.intrinsics
 *
 * Upstream provides three suspend-intrinsic entries — `startCoroutineUndispatched`,
 * `startCoroutineUndispatchedOrReturnIgnoreTimeout`, and `startUndispatchedOrReturn` —
 * that run the coroutine immediately in the current thread until the first suspension,
 * bypassing the ContinuationInterceptor while still installing the new context's
 * thread-locals. The C++ port routes these through `intrinsics/Intrinsics.hpp`; this
 * translation unit is the inventory companion for the file pair.
 */

#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"
#include "kotlinx/coroutines/internal/ThreadContext.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
