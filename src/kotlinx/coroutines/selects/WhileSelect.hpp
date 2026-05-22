#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/selects/WhileSelect.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.selects
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"

#include <functional>
#include <memory>

namespace kotlinx::coroutines::selects {

/**
 * Loops while [select] expression returns `true`.
 *
 * The statement of the form:
 *
 *     while_select<bool>([&](SelectBuilder<bool>& builder) { ... body ... }, completion);
 *
 * is a shortcut for:
 *
 *     while (select<bool>([&](SelectBuilder<bool>& builder) { ... body ... }, completion)) {
 *         // do nothing
 *     }
 *
 * **Note: This is an experimental api.** It may be replaced with a higher-performance DSL for
 * selection from loops.
 *
 * Upstream:
 *   @ExperimentalCoroutinesApi
 *   public suspend inline fun whileSelect(crossinline builder: SelectBuilder<Boolean>.() -> Unit) {
 *       while (select(builder)) { // do nothing
 *       }
 *   }
 */
[[suspend]]
inline void* while_select(
    std::function<void(SelectBuilder<bool>&)> builder,
    std::shared_ptr<Continuation<void*>> completion) {
    while (true) {
        void* select_result =
            dsl::suspend(select<bool>(builder, completion.get()));
        if (intrinsics::is_coroutine_suspended(select_result)) {
            return intrinsics::get_COROUTINE_SUSPENDED();
        }
        // `select_result` carries a boxed boolean; treat null as false.
        bool keep_going =
            select_result && *static_cast<bool*>(select_result);
        if (!keep_going) break;
    }
    return nullptr;
}

} // namespace kotlinx::coroutines::selects
