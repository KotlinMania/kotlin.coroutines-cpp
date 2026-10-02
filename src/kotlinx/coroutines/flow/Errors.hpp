// port-lint: source kotlinx-coroutines-core/common/src/flow/operators/Errors.kt
#pragma once
/**
 * @file Errors.hpp
 * @brief Error handling operators for flows: catch_, retry, retry_when
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Errors.kt
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <cstdint>
#include <exception>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>

namespace kotlinx::coroutines::flow {

namespace internal {

/**
 * Checks whether an exception_ptr holds a CancellationException.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Errors.kt:198
 */
inline bool is_cancellation_exception(const std::exception_ptr& e) {
    if (!e) return false;
    try {
        std::rethrow_exception(e);
    } catch (const CancellationException&) {
        return true;
    } catch (...) {
        return false;
    }
}

/**
 * Compares two exceptions for equivalence.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Errors.kt:216-217
 */
inline bool is_same_exception_as(const std::exception_ptr& a, const std::exception_ptr& b) {
    if (!a || !b) return false;
    if (a == b) return true;
    try {
        std::rethrow_exception(a);
    } catch (const std::exception& ea) {
        try {
            std::rethrow_exception(b);
        } catch (const std::exception& eb) {
            return typeid(ea) == typeid(eb) && std::string(ea.what()) == std::string(eb.what());
        } catch (...) {
            return false;
        }
    } catch (...) {
        return false;
    }
}

/**
 * Checks whether an exception was the cancellation cause of the job in coroutineContext.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Errors.kt:210-214
 */
inline bool is_cancellation_cause(const std::exception_ptr& e, const CoroutineContext* context) {
    if (!context || !e) return false;
    auto element = context->get(Job::type_key);
    if (!element) return false;
    auto* job = dynamic_cast<Job*>(element.get());
    if (!job || !job->is_cancelled()) return false;
    return is_same_exception_as(e, job->get_cancellation_exception()) || is_cancellation_exception(e);
}

/**
 * Implementation helper for catch and retry operators.
 * Returns the exception from upstream or null.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Errors.kt:147-208
 */
template <typename T>
inline void* catch_impl(
    std::shared_ptr<Flow<T>> upstream,
    FlowCollector<T>* collector,
    Continuation<void*>* continuation) {
    std::exception_ptr from_downstream = nullptr;
    try {
        class CatchCollector : public FlowCollector<T> {
            FlowCollector<T>* collector_;
            std::exception_ptr& from_downstream_;
        public:
            CatchCollector(FlowCollector<T>* c, std::exception_ptr& fd)
                : collector_(c), from_downstream_(fd) {
                (void)fd;
            }

            void* emit(T value, Continuation<void*>* cont) override {
                try {
                    return collector_->emit(std::move(value), cont);
                } catch (...) {
                    from_downstream_ = std::current_exception();
                    throw;
                }
            }
        };
        CatchCollector catch_collector(collector, from_downstream);
        void* r = upstream->collect(&catch_collector, continuation);
        if (intrinsics::is_coroutine_suspended(r)) {
            return intrinsics::get_COROUTINE_SUSPENDED();
        }
    } catch (...) {
        auto e = std::current_exception();
        auto* ctx = continuation ? continuation->get_context().get() : nullptr;
        if (is_same_exception_as(e, from_downstream) || is_cancellation_cause(e, ctx)) {
            throw;
        } else {
            if (!from_downstream) {
                return new std::exception_ptr(e);
            }
            if (is_cancellation_exception(e)) {
                std::rethrow_exception(from_downstream);
            } else {
                std::rethrow_exception(e);
            }
        }
    }
    return nullptr;
}

} // namespace internal

/**
 * Catches exceptions in the flow completion and calls a specified [action] with
 * the caught exception. This operator is transparent to exceptions that occur
 * in downstream flow and does not catch exceptions that are thrown to cancel the flow.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Errors.kt:53-57
 */
template <typename T>
inline std::shared_ptr<Flow<T>> catch_(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(FlowCollector<T>*, std::exception_ptr, Continuation<void*>*)> action) {
    return flow<T>(
        [upstream = std::move(upstream), action = std::move(action)](
            FlowCollector<T>* collector,
            Continuation<void*>* completion) -> void* {
            void* exception_handle = internal::catch_impl<T>(upstream, collector, completion);
            if (intrinsics::is_coroutine_suspended(exception_handle)) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
            if (exception_handle) {
                auto* ex_ptr = static_cast<std::exception_ptr*>(exception_handle);
                std::exception_ptr ex = *ex_ptr;
                delete ex_ptr;
                if (ex) {
                    return action(collector, ex, completion);
                }
            }
            return nullptr;
        });
}

/**
 * Non-suspending overload of [catch_].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Errors.kt:53-57
 */
template <typename T>
inline std::shared_ptr<Flow<T>> catch_(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void(FlowCollector<T>*, std::exception_ptr)> action) {
    return catch_<T>(
        std::move(upstream),
        [action = std::move(action)](
            FlowCollector<T>* collector, std::exception_ptr cause, Continuation<void*>*) -> void* {
            action(collector, cause);
            return nullptr;
        });
}

/**
 * Retries collection of the given flow when an exception occurs in the upstream flow and the
 * [predicate] returns true.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Errors.kt:127-143
 */
template <typename T>
inline std::shared_ptr<Flow<T>> retry_when(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(FlowCollector<T>*, std::exception_ptr, std::int64_t,
                        Continuation<void*>*)>
        predicate) {
    return flow<T>(
        [upstream = std::move(upstream), predicate = std::move(predicate)](
            FlowCollector<T>* collector,
            Continuation<void*>* completion) -> void* {
            std::int64_t attempt = 0;
            bool shall_retry = false;
            do {
                shall_retry = false;
                void* exception_handle =
                    internal::catch_impl<T>(upstream, collector, completion);
                if (intrinsics::is_coroutine_suspended(exception_handle)) {
                    return intrinsics::get_COROUTINE_SUSPENDED();
                }
                if (exception_handle) {
                    auto* ex_ptr = static_cast<std::exception_ptr*>(exception_handle);
                    std::exception_ptr cause = *ex_ptr;
                    delete ex_ptr;
                    if (cause) {
                        void* outcome = predicate(collector, cause, attempt, completion);
                        if (intrinsics::is_coroutine_suspended(outcome)) {
                            return intrinsics::get_COROUTINE_SUSPENDED();
                        }
                        bool retry_now = false;
                        if (outcome) {
                            auto* b = static_cast<bool*>(outcome);
                            retry_now = *b;
                            delete b;
                        }
                        if (retry_now) {
                            shall_retry = true;
                            ++attempt;
                        } else {
                            std::rethrow_exception(cause);
                        }
                    }
                }
            } while (shall_retry);
            return nullptr;
        });
}

/**
 * Non-suspending predicate overload of [retry_when].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Errors.kt:127-143
 */
template <typename T>
inline std::shared_ptr<Flow<T>> retry_when(
    std::shared_ptr<Flow<T>> upstream,
    std::function<bool(FlowCollector<T>*, std::exception_ptr, std::int64_t)> predicate) {
    return retry_when<T>(
        std::move(upstream),
        [predicate = std::move(predicate)](
            FlowCollector<T>* collector, std::exception_ptr cause, std::int64_t attempt,
            Continuation<void*>*) -> void* {
            return new bool(predicate(collector, cause, attempt));
        });
}

/**
 * Retries collection of the given flow up to [retries] times when an exception occurs in the upstream flow.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Errors.kt:86-92
 */
template <typename T>
inline std::shared_ptr<Flow<T>> retry(
    std::shared_ptr<Flow<T>> upstream,
    std::int64_t retries = std::numeric_limits<std::int64_t>::max(),
    std::function<void*(std::exception_ptr, Continuation<void*>*)> predicate =
        [](std::exception_ptr, Continuation<void*>*) -> void* {
            return new bool(true);
        }) {
    if (retries <= 0) {
        throw std::invalid_argument(
            "Expected positive amount of retries, but had " + std::to_string(retries));
    }
    return retry_when<T>(
        std::move(upstream),
        [retries, predicate = std::move(predicate)](
            FlowCollector<T>*, std::exception_ptr cause, std::int64_t attempt,
            Continuation<void*>* cont) -> void* {
            if (attempt >= retries) return new bool(false);
            return predicate(cause, cont);
        });
}

/**
 * Non-suspending predicate overload of [retry].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Errors.kt:86-92
 */
template <typename T>
inline std::shared_ptr<Flow<T>> retry(
    std::shared_ptr<Flow<T>> upstream,
    std::int64_t retries,
    std::function<bool(std::exception_ptr)> predicate) {
    if (retries <= 0) {
        throw std::invalid_argument(
            "Expected positive amount of retries, but had " + std::to_string(retries));
    }
    return retry_when<T>(
        std::move(upstream),
        [retries, predicate = std::move(predicate)](
            FlowCollector<T>*, std::exception_ptr cause, std::int64_t attempt,
            Continuation<void*>*) -> void* {
            if (attempt >= retries) return new bool(false);
            return new bool(predicate(cause));
        });
}

} // namespace kotlinx::coroutines::flow
