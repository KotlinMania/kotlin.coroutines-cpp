#include <optional>
#include <functional>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/selects/OnTimeout.kt
//
// TODO: This is a mechanical syntax transliteration. The following Kotlin constructs need proper C++ implementation:
// - suspend functions (marked but not implemented as C++20 coroutines)
// - Kotlin coroutines infrastructure (SelectBuilder, SelectInstance, etc.)
// - Extension functions (converted to free functions)
// - Smart casts and type inference
// - Nullable types (T* -> T* or std::optional<T>)
// - Lambda types and closures
// - Kotlin Duration type
// - @ExperimentalCoroutinesApi, @Suppress annotations (kept as comments)

namespace kotlinx {
namespace coroutines {
namespace selects {

// import kotlinx.coroutines.*
// import kotlin.time.*

/**
 * Clause that selects the given [block] after a specified timeout passes.
 * If timeout is negative or zero, [block] is selected immediately.
 *
 * **Note: This is an experimental api.** It may be replaced with light-weight timer/timeout channels in the future.
 *
 * @param timeMillis timeout time in milliseconds.
 */
// @ExperimentalCoroutinesApi
// @Suppress("EXTENSION_SHADOWED_BY_MEMBER")
template<typename R>
void on_timeout(SelectBuilder<R>& select_builder, long time_millis, std::function<R()> block) {
    // TODO: suspend function semantics not implemented
    OnTimeout timeout_obj(time_millis);
    timeout_obj.select_clause.invoke(block);
}

/**
 * Clause that selects the given [block] after the specified [timeout] passes.
 * If timeout is negative or zero, [block] is selected immediately.
 *
 * **Note: This is an experimental api.** It may be replaced with light-weight timer/timeout channels in the future.
 */
// @ExperimentalCoroutinesApi
template<typename R>
void on_timeout(SelectBuilder<R>& select_builder, Duration timeout, std::function<R()> block) {
    // TODO: suspend function semantics not implemented
    on_timeout(select_builder, timeout.to_delay_millis(), block);
}

/**
 * We implement [SelectBuilder.onTimeout] as a clause, so each invocation creates
 * an instance of [OnTimeout] that specifies the registration part according to
 * the [timeout][timeMillis] parameter.
 */
class OnTimeout {
private:
    long time_millis;

public:
    explicit OnTimeout(long time_millis_) : time_millis(time_millis_) {}

    // @Suppress("UNCHECKED_CAST")
    SelectClause0 get_select_clause() const {
        return SelectClause0Impl(
            /* clauseObject = */ this,
            /* regFunc = */ static_cast<RegistrationFunction>(&OnTimeout::register_func)
        );
    }

private:
    // @Suppress("UNUSED_PARAMETER")
    void register_func(SelectInstance<void*>* select, void* ignored_param) {
        // Should this clause complete immediately*
        if (time_millis <= 0) {
            select->select_in_registration_phase(/* Unit */ nullptr);
            return;
        }
        // Invoke `trySelect` after the timeout is reached.
        auto action = [this, select]() {
            select->try_select(this, /* Unit */ nullptr);
        };
        auto* select_impl = static_cast<SelectImplementation<void*>*>(select);
        auto context = select_impl->context;
        auto disposable_handle = context.delay.invoke_on_timeout(time_millis, action, context);
        // Do not forget to clean-up when this `select` is completed or cancelled.
        select->dispose_on_completion(disposable_handle);
    }
};

} // namespace selects
} // namespace coroutines
} // namespace kotlinx
