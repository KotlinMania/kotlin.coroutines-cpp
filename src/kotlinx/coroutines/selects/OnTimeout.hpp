#pragma once
/**
 * @file OnTimeout.hpp
 * @brief Select clause for timeout handling
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/selects/OnTimeout.kt
 */

#include "kotlinx/coroutines/selects/Select.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include <functional>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace selects {

class OnTimeout; // Forward decl

/**
 * We implement SelectBuilder.on_timeout as a clause, so each invocation creates
 * an instance of OnTimeout that specifies the registration part according to
 * the timeout (time_millis) parameter.
 *
 * Transliterated from:
 * private class OnTimeout(private val timeMillis: Long)
 */
    explicit OnTimeout(long long time_millis) : time_millis_(time_millis) {
        (void)time_millis;
    }

    /**
     * Get the SelectClause0 for this timeout.
     *
     * Transliterated from:
     * val selectClause: SelectClause0
     *     get() = SelectClause0Impl(
     *         clauseObject = this@OnTimeout,
     *         regFunc = OnTimeout::register as RegistrationFunction
     *     )
     */
    std::unique_ptr<SelectClause0> select_clause() {
        // Capture shared_ptr to keep this object alive during select registration/handling
        auto self = shared_from_this();
        return std::make_unique<SelectClause0Impl>(
            static_cast<void*>(this),
            [self](void* clause_obj, void* select_ptr, void* param) {
                self->do_register(clause_obj, select_ptr, param);
            }
        );
    }

private:
    long long time_millis_;

    /**
     * Registration function for the timeout clause.
     *
     * Transliterated from:
     * private fun register(select: SelectInstance<*>, ignoredParam: Any?) {
     *     // Should this clause complete immediately?
     *     if (timeMillis <= 0) {
     *         select.selectInRegistrationPhase(Unit)
     *         return
     *     }
     *     // Invoke `trySelect` after the timeout is reached.
     *     val action = Runnable {
     *         select.trySelect(this@OnTimeout, Unit)
     *     }
     *     select as SelectImplementation<*>
     *     val context = select.context
     *     val disposableHandle = context.delay.invokeOnTimeout(timeMillis, action, context)
     *     // Do not forget to clean-up when this `select` is completed or cancelled.
     *     select.disposeOnCompletion(disposableHandle)
     * }
     */
    void do_register(void* clause_obj, void* select_ptr, void* /*ignored_param*/) {
        // Should this clause complete immediately?
        if (time_millis_ <= 0) {
            // Cast to SelectInstance to call selectInRegistrationPhase
            auto* select = static_cast<SelectInstance<void*>*>(select_ptr);
            select->select_in_registration_phase(nullptr);  // Unit
            return;
        }

        // Invoke `trySelect` after the timeout is reached.
        // We need to keep 'this' alive for the action callback too
        auto self = shared_from_this();
        auto action = std::make_shared<LambdaRunnable>([select_ptr, self]() {
            auto* select = static_cast<SelectInstance<void*>*>(select_ptr);
            // Pass 'self.get()' (address of this OnTimeout object) as clauseObject identity
            select->try_select(self.get(), nullptr);  // Unit
        });

        // Get context and schedule timeout
        // We know select is SelectImplementation
        // Cast via void* is safe per design (SelectInstance* -> SelectImplementation*)
        // But SelectInstance has try_select (virtual) but not dispose_on_completion (virtual).
        // Wait, SelectInstance HAS dispose_on_completion virtual.
        // But get_context is on SelectInstance too.
        
        auto* select = static_cast<SelectInstance<void*>*>(select_ptr);
        auto context = select->get_context();
        
        // We need to access Delay. In Kotlin it's context.delay (extension).
        // Here we use global get_default_delay() or similar for now.
        auto& delay = get_default_delay();
        auto handle = delay.invoke_on_timeout(time_millis_, action, *context);

        // Do not forget to clean-up when this `select` is completed or cancelled.
        select->dispose_on_completion(handle);
    }

    // Helper class for lambda-based Runnable
    class LambdaRunnable : public Runnable {
        std::function<void()> func_;
    public:
        explicit LambdaRunnable(std::function<void()> f) : func_(std::move(f)) {}
        void run() override { func_(); }
    };
};

/**
 * Clause that selects the given [block] after a specified timeout passes.
 * If timeout is negative or zero, [block] is selected immediately.
 *
 * **Note: This is an experimental api.** It may be replaced with light-weight timer/timeout channels in the future.
 *
 * @param time_millis timeout time in milliseconds.
 */
template<typename R>
void on_timeout(SelectBuilder<R>& builder, long long time_millis, std::function<void*(Continuation<void*>*)> block) {
    // Create OnTimeout object managed by shared_ptr
    auto timeout = std::make_shared<OnTimeout>(time_millis);
    // Create the clause (which captures the shared_ptr)
    auto clause = timeout->select_clause();
    // Register it
    builder.invoke(*clause, std::move(block));
}

} // namespace selects
} // namespace coroutines
} // namespace kotlinx
