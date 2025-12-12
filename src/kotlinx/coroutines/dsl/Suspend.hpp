#pragma once
/**
 * Kotlin-aligned suspend DSL helpers.
 *
 * The compiler plugin recognizes `suspend(expr)` as a suspension point inside
 * functions marked `[[suspend]]` / `[[kotlinx::suspend]]`.
 *
 * At runtime this wrapper is a zero-cost identity function. Its sole purpose is
 * to provide Kotlin-like spelling at the call site.
 *
 * Example:
 * ```cpp
 * using namespace kotlinx::coroutines::dsl;
 *
 * [[suspend]]
 * void* foo(std::shared_ptr<Continuation<void*>> completion) {
 *     suspend(bar(completion));
 *     return nullptr;
 * }
 * ```
 */

#include <utility>

namespace kotlinx {
namespace coroutines {
namespace dsl {

template <typename T>
inline T suspend(T&& value) {
    return std::forward<T>(value);
}

} // namespace dsl
} // namespace coroutines
} // namespace kotlinx

