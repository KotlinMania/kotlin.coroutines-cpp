#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/selects/OnTimeout.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.selects
 */

#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/DisposableHandle.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include "kotlinx/coroutines/Unit.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>

namespace kotlinx::coroutines::selects {

/**
 * We implement [SelectBuilder.onTimeout] as a clause, so each invocation creates
 * an instance of [OnTimeout] that specifies the registration part according to the
 * [timeout][time_millis] parameter.
 *
 * Upstream:
 *   private class OnTimeout(private val timeMillis: Long) {
 *       val selectClause: SelectClause0 get() = SelectClause0Impl(this, OnTimeout::register)
 *       private fun register(select: SelectInstance<*>, ignoredParam: Any?) { ... }
 *   }
 */
class OnTimeout : public std::enable_shared_from_this<OnTimeout> {
public:
    explicit OnTimeout(std::int64_t time_millis) : time_millis_(time_millis) {}

    /**
     * Upstream:
     *   val selectClause: SelectClause0 get() =
     *       SelectClause0Impl(clauseObject = this@OnTimeout,
     *                         regFunc = OnTimeout::register as RegistrationFunction)
     */
    std::unique_ptr<SelectClause0> select_clause() {
        auto self = shared_from_this();
        return std::make_unique<SelectClause0Impl>(
            static_cast<void*>(this),
            [self](void* clause_obj, void* select_ptr, void* param) {
                self->register_clause(clause_obj, select_ptr, param);
            });
    }

private:
    std::int64_t time_millis_;

    /**
     * Upstream:
     *   private fun register(select: SelectInstance<*>, ignoredParam: Any?) {
     *       if (timeMillis <= 0) { select.selectInRegistrationPhase(Unit); return }
     *       val action = Runnable { select.trySelect(this@OnTimeout, Unit) }
     *       select as SelectImplementation<*>
     *       val context = select.context
     *       val disposableHandle = context.delay.invokeOnTimeout(timeMillis, action, context)
     *       select.disposeOnCompletion(disposableHandle)
     *   }
     */
    void register_clause(void* /*clause_obj*/, void* select_ptr, void* /*ignored_param*/) {
        auto* select = static_cast<SelectInstance<void*>*>(select_ptr);

        // Should this clause complete immediately?
        if (time_millis_ <= 0) {
            select->select_in_registration_phase(nullptr); // Unit
            return;
        }

        // Invoke `try_select` after the timeout is reached.
        auto self = shared_from_this();
        auto action = std::make_shared<LambdaRunnable>([select, self]() {
            select->try_select(self.get(), nullptr); // Unit
        });

        auto context = select->get_context();
        auto& delay = get_default_delay();
        auto handle = delay.invoke_on_timeout(time_millis_, action, *context);

        // Do not forget to clean-up when this `select` is completed or cancelled.
        select->dispose_on_completion(handle);
    }

    /** Internal lambda-as-Runnable adapter. */
    class LambdaRunnable : public Runnable {
    public:
        explicit LambdaRunnable(std::function<void()> func) : func_(std::move(func)) {}
        void run() override { func_(); }

    private:
        std::function<void()> func_;
    };
};

/**
 * Clause that selects the given [block] after a specified timeout passes.
 * If timeout is negative or zero, [block] is selected immediately.
 *
 * **Note: This is an experimental api.** It may be replaced with light-weight timer/timeout
 * channels in the future.
 *
 * Upstream:
 *   @ExperimentalCoroutinesApi
 *   @Suppress("EXTENSION_SHADOWED_BY_MEMBER")
 *   public fun <R> SelectBuilder<R>.onTimeout(timeMillis: Long, block: suspend () -> R): Unit =
 *       OnTimeout(timeMillis).selectClause.invoke(block)
 *
 * @param time_millis timeout time in milliseconds.
 */
template <typename R>
inline void on_timeout(
    SelectBuilder<R>& builder,
    std::int64_t time_millis,
    std::function<void*(Continuation<void*>*)> block) {
    auto timeout = std::make_shared<OnTimeout>(time_millis);
    auto clause = timeout->select_clause();
    builder.invoke(*clause, std::move(block));
}

/**
 * Clause that selects the given [block] after the specified [timeout] passes.
 *
 * Upstream:
 *   @ExperimentalCoroutinesApi
 *   public fun <R> SelectBuilder<R>.onTimeout(timeout: Duration, block: suspend () -> R): Unit =
 *       onTimeout(timeout.toDelayMillis(), block)
 */
template <typename R, typename Rep, typename Period>
inline void on_timeout(
    SelectBuilder<R>& builder,
    std::chrono::duration<Rep, Period> timeout,
    std::function<void*(Continuation<void*>*)> block) {
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
    on_timeout<R>(builder, static_cast<std::int64_t>(millis), std::move(block));
}

} // namespace kotlinx::coroutines::selects
