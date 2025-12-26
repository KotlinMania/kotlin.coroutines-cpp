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
    builder.on_timeout(time_millis, std::move(block));
}

/**
 * We implement SelectBuilder.on_timeout as a clause, so each invocation creates
 * an instance of OnTimeout that specifies the registration part according to
 * the timeout (time_millis) parameter.
 *
 * Transliterated from:
 * private class OnTimeout(private val timeMillis: Long)
 */
class OnTimeout {
public:
    explicit OnTimeout(long long time_millis) : time_millis_(time_millis) {}

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
        return std::make_unique<SelectClause0Impl>(
            static_cast<void*>(this),
            [this](void* clause_obj, void* select_ptr, void* param) {
                this->do_register(clause_obj, select_ptr, param);
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
        auto* timeout_obj = this;
        auto action = std::make_shared<LambdaRunnable>([select_ptr, timeout_obj]() {
            auto* select = static_cast<SelectInstance<void*>*>(select_ptr);
            select->try_select(timeout_obj, nullptr);  // Unit
        });

        // Get context and schedule timeout
        auto* select_impl = static_cast<SelectImplementation<void*>*>(select_ptr);
        auto context = select_impl->get_context();
        auto& delay = get_default_delay();
        auto handle = delay.invoke_on_timeout(time_millis_, action, *context);

        // Do not forget to clean-up when this `select` is completed or cancelled.
        select_impl->dispose_on_completion(handle);
    }

    // Helper class for lambda-based Runnable
    class LambdaRunnable : public Runnable {
        std::function<void()> func_;
    public:
        explicit LambdaRunnable(std::function<void()> f) : func_(std::move(f)) {}
        void run() override { func_(); }
    };
};

} // namespace selects
} // namespace coroutines
} // namespace kotlinx
