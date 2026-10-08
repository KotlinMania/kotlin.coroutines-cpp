// port-lint: source selects/OnTimeout.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/selects/OnTimeout.kt
 */
#pragma once

#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

namespace kotlinx::coroutines::selects {
namespace detail {
// NOTE(port): The private concrete OnTimeout lives in the implementation file.
// Registration at this boundary takes the correctly adjusted SelectInstanceBase pointer.
// Transliterated from: kotlinx-coroutines-core/common/src/selects/OnTimeout.kt:34-42
std::unique_ptr<SelectClause0> make_on_timeout_clause(std::int64_t time_millis);
}

/**
 * Selects the given block after the timeout. Negative or zero timeouts select immediately.
 * This experimental API may be replaced with lightweight timer/timeout channels.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/selects/OnTimeout.kt:16-17
template <typename R>
inline void on_timeout(SelectBuilder<R>& builder, std::int64_t time_millis,
                       std::function<void*(Continuation<void*>*)> block) {
    auto clause = detail::make_on_timeout_clause(time_millis);
    auto registration = clause->get_reg_func();
    // NOTE(port): ClauseData erases SelectInstance<R> before registration. Adjust
    // its actual base pointer before erasing it for the source star-projected API.
    SelectClause0Impl typed_clause(clause->get_clause_object(),
        [registration = std::move(registration)](void* object, void* select, void* param) {
            registration(object, static_cast<SelectInstanceBase*>(
                static_cast<SelectInstance<R>*>(select)), param);
        });
    builder.invoke(typed_clause, std::move(block));
}

// Transliterated from: kotlinx-coroutines-core/common/src/selects/OnTimeout.kt:26-27
template <typename R>
inline void on_timeout(SelectBuilder<R>& builder, kotlin::time::Duration timeout,
                       std::function<void*(Continuation<void*>*)> block) {
    on_timeout<R>(builder, to_delay_millis(timeout), std::move(block));
}

// NOTE(port): Ordinary C++ callers may use chrono durations. Keep the source
// rounding-up and saturation contract when adapting that public C++ type.
// Transliterated from: kotlinx-coroutines-core/common/src/selects/OnTimeout.kt:26-27; kotlinx-coroutines-core/common/src/Delay.kt:155-158
template <typename R, typename Rep, typename Period>
inline void on_timeout(SelectBuilder<R>& builder, std::chrono::duration<Rep, Period> timeout,
                       std::function<void*(Continuation<void*>*)> block) {
    const auto millis = std::chrono::duration<long double, std::milli>(timeout).count();
    const auto maximum = std::numeric_limits<std::int64_t>::max();
    const std::int64_t rounded = millis <= 0 ? 0 :
        millis >= static_cast<long double>(maximum) ? maximum :
        static_cast<std::int64_t>(std::ceil(millis));
    on_timeout<R>(builder, rounded, std::move(block));
}

// Transliterated from: kotlinx-coroutines-core/common/src/selects/Select.kt:111-113; kotlinx-coroutines-core/common/src/selects/OnTimeout.kt:16-17
template <typename R>
template <typename Callback>
inline void SelectBuilder<R>::on_timeout(std::int64_t time_millis, Callback&& block) {
    if constexpr (std::is_invocable_v<Callback&>) {
        selects::on_timeout<R>(*this, time_millis,
            [b = std::forward<Callback>(block)](Continuation<void*>*) mutable -> void* {
                if constexpr (std::is_void_v<std::invoke_result_t<Callback&>>) {
                    b();
                    return nullptr;
                } else if constexpr (std::is_same_v<R, void*>) {
                    return b();
                } else {
                    // NOTE(port): The select result consumer owns this ABI box.
                    return new R(b());
                }
            });
    } else {
        selects::on_timeout<R>(*this, time_millis, std::forward<Callback>(block));
    }
}

} // namespace kotlinx::coroutines::selects
