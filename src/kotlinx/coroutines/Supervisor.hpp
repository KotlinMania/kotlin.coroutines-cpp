#pragma once
// port-lint: source Supervisor.kt
/**
 * @file Supervisor.hpp
 * @brief Supervisor job and scope declarations
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Supervisor.kt
 *
 * Provides SupervisorJob and supervisor_scope for handling child failures independently.
 */

#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include <memory>
#include <functional>

namespace kotlinx {
namespace coroutines {

/**
 * Creates a _supervisor_ job in an active state.
 * Children of a supervisor job can fail independently of each other.
 *
 * A failure or cancellation of a child does not cause the supervisor job to fail and does not affect its other children,
 * so a supervisor can implement a custom policy for handling failures of its children:
 *
 * - A failure of a child job that was created using launch can be handled via CoroutineExceptionHandler in the context.
 * - A failure of a child job that was created using async can be handled via Deferred::await() on the resulting deferred value.
 *
 * If a parent job is specified, then this supervisor job becomes a child job of the parent and is cancelled when the
 * parent fails or is cancelled. All this supervisor's children are cancelled in this case, too.
 *
 * @param parent an optional parent job
 * @return a new supervisor CompletableJob
 */
std::shared_ptr<CompletableJob> make_supervisor_job(std::shared_ptr<struct Job> parent = nullptr);

/**
 * Kotlin-style alias for make_supervisor_job.
 * @param parent an optional parent job
 * @return a new supervisor CompletableJob
 */
inline std::shared_ptr<CompletableJob> SupervisorJob(std::shared_ptr<struct Job> parent = nullptr) {
    return make_supervisor_job(parent);
}

namespace internal {

/**
 * Simple concrete implementation of CoroutineScope for supervisor_scope.
 */
class SimpleCoroutineScope : public CoroutineScope {
    std::shared_ptr<CoroutineContext> context_;
public:
    explicit SimpleCoroutineScope(std::shared_ptr<CoroutineContext> context)
        : context_(std::move(context)) {}

    std::shared_ptr<CoroutineContext> get_coroutine_context() const override {
        return context_;
    }
};

} // namespace internal

/**
 * Creates a CoroutineScope with SupervisorJob and calls the specified block with this scope.
 * The provided scope inherits its coroutineContext from the outer scope, using the
 * Job from that context as the parent for the new SupervisorJob.
 * This function returns as soon as the given block and all its child coroutines are completed.
 *
 * Unlike coroutine_scope, a failure of a child does not cause this scope to fail and does not affect its other children,
 * so a custom policy for handling failures of its children can be implemented. See SupervisorJob for additional details.
 *
 * If an exception happened in block, then the supervisor job is failed and all its children are cancelled.
 * If the current coroutine was cancelled, then both the supervisor job itself and all its children are cancelled.
 *
 * NOTE: In this C++ transliteration, this executes synchronously (blocking).
 *
 * @tparam R the return type of the block
 * @param block the block to execute in the supervisor scope
 * @return the result of the block
 */
template<typename R>
R supervisor_scope(std::function<R(CoroutineScope&)> block) {
    // Create a supervisor job with no parent (standalone)
    auto supervisor_job = make_supervisor_job(nullptr);

    // Create a context containing the supervisor job
    auto context = std::dynamic_pointer_cast<CoroutineContext>(supervisor_job);

    // Create a scope with the supervisor job context
    internal::SimpleCoroutineScope scope(context);

    // Execute the block
    R result = block(scope);

    // Complete the supervisor job
    supervisor_job->complete();

    return result;
}

/**
 * Void specialization of supervisor_scope.
 *
 * @param block the block to execute in the supervisor scope
 */
inline void supervisor_scope(std::function<void(CoroutineScope&)> block) {
    auto supervisor_job = make_supervisor_job(nullptr);
    auto context = std::dynamic_pointer_cast<CoroutineContext>(supervisor_job);
    internal::SimpleCoroutineScope scope(context);
    block(scope);
    supervisor_job->complete();
}

} // namespace coroutines
} // namespace kotlinx
