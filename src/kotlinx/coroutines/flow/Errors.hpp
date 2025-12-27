/**
 * @file Errors.hpp
 * @brief Error handling operators for flows: catch_, retry, retry_when
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Errors.kt
 *
 * These operators provide error handling capabilities for flows:
 * - catch_: Catches exceptions and handles them
 * - retry: Retries collection up to a specified number of times
 * - retry_when: Retries with a custom predicate that can inspect cause and attempt number
 */
#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include <functional>
#include <exception>

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * Catches exceptions in the flow completion and calls a specified action with
 * the caught exception. This operator is *transparent* to exceptions that occur
 * in downstream flow and does not catch exceptions that are thrown to cancel the flow.
 *
 * For example:
 *
 * ```cpp
 * flow | emit_data()
 *      | map([](auto it) { return compute_one(it); })
 *      | catch_([](auto* collector, auto cause) { ... }) // catches exceptions in emit_data and compute_one
 *      | map([](auto it) { return compute_two(it); })
 *      | collect([](auto it) { process(it); }); // throws exceptions from process and compute_two
 * ```
 *
 * Conceptually, the action of catch_ operator is similar to wrapping the code of upstream flows with
 * `try { ... } catch (const std::exception& e) { action(e); }`.
 *
 * Any exception in the action code itself proceeds downstream where it can be
 * caught by further catch_ operators if needed. If a particular exception does not need to be
 * caught it can be rethrown from the action of catch_ operator.
 *
 * The action code has FlowCollector as a receiver and can emit values downstream.
 * For example, caught exception can be replaced with some wrapper value for errors:
 *
 * ```cpp
 * flow | catch_([](auto* collector, auto cause) { collector->emit(ErrorWrapperValue(cause)); });
 * ```
 *
 * @note Named catch_ in C++ to avoid conflict with C++ keyword 'catch'.
 *
 * Transliterated from:
 * public fun <T> Flow<T>.catch(action: suspend FlowCollector<T>.(cause: Throwable) -> Unit): Flow<T>
 */
template<typename T>
Flow<T>* catch_(Flow<T>* flow, std::function<void(FlowCollector<T>*, std::exception_ptr)> action) {
    // TODO(port): Implement catch operator using catch_impl
    return flow;
}

/**
 * Retries collection of the given flow up to retries times when an exception that matches the
 * given predicate occurs in the upstream flow. This operator is *transparent* to exceptions that occur
 * in downstream flow and does not retry on exceptions that are thrown to cancel the flow.
 *
 * See catch_ for details on how exceptions are caught in flows.
 *
 * The default value of retries parameter is effectively infinite (LONG_MAX).
 * This value effectively means to retry forever.
 * This operator is a shorthand for the following code (see retry_when). Note that `attempt` is checked first
 * and predicate is not called when it reaches the given number of retries:
 *
 * ```cpp
 * retry_when(flow, [retries, predicate](auto*, auto cause, long attempt) {
 *     return attempt < retries && predicate(cause);
 * });
 * ```
 *
 * The predicate parameter is always true by default. The predicate is a suspending function,
 * so it can be also used to introduce delay before retry, for example:
 *
 * ```cpp
 * retry(flow, 3, [](auto cause) {
 *     // retry on any std::system_error but also introduce delay if retrying
 *     try {
 *         std::rethrow_exception(cause);
 *     } catch (const std::system_error& e) {
 *         delay(1000);
 *         return true;
 *     } catch (...) {
 *         return false;
 *     }
 * });
 * ```
 *
 * @throws std::invalid_argument when retries is not positive.
 *
 * Transliterated from:
 * public fun <T> Flow<T>.retry(retries: Long = Long.MAX_VALUE, predicate: suspend (cause: Throwable) -> Boolean): Flow<T>
 */
template<typename T>
Flow<T>* retry(Flow<T>* flow, long retries = -1, std::function<bool(std::exception_ptr)> predicate = [](std::exception_ptr){ return true; }) {
    // TODO(port): Implement retry logic using retry_when
    return flow;
}

/**
 * Retries collection of the given flow when an exception occurs in the upstream flow and the
 * predicate returns true. The predicate also receives an `attempt` number as parameter,
 * starting from zero on the initial call. This operator is *transparent* to exceptions that occur
 * in downstream flow and does not retry on exceptions that are thrown to cancel the flow.
 *
 * For example, the following call retries the flow forever if the error is caused by std::system_error, but
 * stops after 3 retries on any other exception:
 *
 * ```cpp
 * retry_when(flow, [](auto*, auto cause, long attempt) {
 *     try {
 *         std::rethrow_exception(cause);
 *     } catch (const std::system_error&) {
 *         return true;
 *     } catch (...) {
 *         return attempt < 3;
 *     }
 * });
 * ```
 *
 * To implement a simple retry logic with a limit on the number of retries use retry operator.
 *
 * Similarly to catch_ operator, the predicate code has FlowCollector as a receiver and can
 * emit values downstream.
 * The predicate is a suspending function, so it can be used to introduce delay before retry, for example:
 *
 * ```cpp
 * retry_when(flow, [](auto* collector, auto cause, long attempt) {
 *     try {
 *         std::rethrow_exception(cause);
 *     } catch (const std::system_error&) {
 *         collector->emit(RetryWrapperValue(cause));
 *         delay(1000);  // delay for one second before retry
 *         return true;
 *     } catch (...) {
 *         return false;  // do not retry otherwise
 *     }
 * });
 * ```
 *
 * See catch_ for more details.
 *
 * Transliterated from:
 * public fun <T> Flow<T>.retryWhen(predicate: suspend FlowCollector<T>.(cause: Throwable, attempt: Long) -> Boolean): Flow<T>
 */
template<typename T>
Flow<T>* retry_when(Flow<T>* flow, std::function<bool(FlowCollector<T>*, std::exception_ptr, long)> predicate) {
    // TODO(port): Implement retry_when
    return flow;
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
