#include <functional>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ (first-pass, mechanical syntax mapping)
// Original: kotlinx-coroutines-core/common/src/Supervisor.kt
//
// TODO:
// - Suspend functions and coroutine infrastructure
// - Contract/InvocationKind needs compile-time contracts or documentation
// - suspendCoroutineUninterceptedOrReturn needs coroutine support
// - Private nested classes need proper encapsulation

#include <exception>

namespace kotlinx {
namespace coroutines {

class Job;
class CompletableJob;
class CoroutineScope;
class CoroutineContext;
template<typename T> class Continuation;

/**
 * Creates a _supervisor_ job class in an active state.
 * Children of a supervisor job can fail independently of each other.
 *
 * A failure or cancellation of a child does not cause the supervisor job to fail and does not affect its other children,
 * so a supervisor can implement a custom policy for handling failures of its children:
 *
 * - A failure of a child job that was created using [launch][CoroutineScope.launch] can be handled via [CoroutineExceptionHandler] in the context.
 * - A failure of a child job that was created using [async][CoroutineScope.async] can be handled via [Deferred.await] on the resulting deferred value.
 *
 * If a [parent] job is specified, then this supervisor job becomes a child job of the [parent] and is cancelled when the
 * parent fails or is cancelled. All this supervisor's children are cancelled in this case, too.
 */
// @Suppress("FunctionName")
CompletableJob* SupervisorJob(Job* parent = nullptr);

/** @suppress Binary compatibility only */
// @Suppress("FunctionName")
// @Deprecated(level = DeprecationLevel.HIDDEN, message = "Since 1.2.0, binary compatibility with versions <= 1.1.x")
// @JvmName("SupervisorJob")
Job* SupervisorJob0(Job* parent = nullptr);

/**
 * Creates a [CoroutineScope] with [SupervisorJob] and calls the specified suspend [block] with this scope.
 * The provided scope inherits its [coroutineContext][CoroutineScope.coroutineContext] from the outer scope, using the
 * [Job] from that context as the parent for the new [SupervisorJob].
 * This function returns as soon as the given block and all its child coroutines are completed.
 *
 * Unlike [coroutineScope], a failure of a child does not cause this scope to fail and does not affect its other children,
 * so a custom policy for handling failures of its children can be implemented. See [SupervisorJob] for additional details.
 *
 * If an exception happened in [block], then the supervisor job is failed and all its children are cancelled.
 * If the current coroutine was cancelled, then both the supervisor job itself and all its children are cancelled.
 *
 * The method may throw a [CancellationException] if the current job was cancelled externally,
 * or rethrow an exception thrown by the given [block].
 */
// TODO: suspend function - coroutine semantics not implemented
template<typename R>
R supervisor_scope(std::function<R(CoroutineScope&)> block);

// Private implementation classes would go here with linkage
// namespace { // anonymous namespace for classes
//     class SupervisorJobImpl : JobImpl { ... }
//     template<typename T> class SupervisorCoroutine : ScopeCoroutine<T> { ... }
// }

} // namespace coroutines
} // namespace kotlinx
