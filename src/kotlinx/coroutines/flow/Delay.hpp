#pragma once
// port-lint: source flow/operators/Delay.kt
/**
 * @file Delay.hpp
 * @brief Time-based flow operators: debounce, sample, timeout
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Delay.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/internal/NullSurrogate.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/channels/Produce.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"
#include "kotlinx/coroutines/Timeout.hpp"
#include <chrono>
#include <stdexcept>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace flow {

using namespace kotlinx::coroutines::channels;

// Helper: FlowCollector that forwards to a lambda
template<typename T>
class FunctionalCollector : public FlowCollector<T> {
    std::function<void(T)> action_;
public:
    explicit FunctionalCollector(std::function<void(T)> action) : action_(std::move(action)) {}
    void* emit(T value, Continuation<void*>*) override {
        action_(std::move(value));
        return nullptr;
    }
};

// =============================================================================
// debounce - filter values followed by newer values within timeout
// =============================================================================

/**
 * Returns a flow that mirrors the original flow, but filters out values
 * that are followed by the newer values within the given timeout.
 * The latest value is always emitted.
 *
 * Note that the resulting flow does not emit anything as long as the original flow emits
 * items faster than every timeout_millis milliseconds.
 *
 * Example:
 * ```cpp
 * // Given emissions: 1 (wait 90ms) 2 (wait 90ms) 3 (wait 1010ms) 4 (wait 1010ms) 5
 * // With debounce(1000), produces: 3, 4, 5
 * ```
 */
template<typename T>
std::shared_ptr<Flow<T>> debounce(std::shared_ptr<Flow<T>> upstream, long timeout_millis) {
    if (timeout_millis < 0) {
        throw std::invalid_argument("Debounce timeout should not be negative");
    }
    if (timeout_millis == 0) {
        return upstream;
    }
    return debounce_internal<T>(upstream, [timeout_millis](const T&) { return timeout_millis; });
}

/**
 * Returns a flow that mirrors the original flow, but filters out values
 * that are followed by the newer values within the given timeout.
 *
 * A variation of debounce that allows specifying the timeout value dynamically.
 *
 * @param timeout_millis_selector Function that returns timeout in milliseconds for each value.
 */
template<typename T, typename Fn>
std::shared_ptr<Flow<T>> debounce(std::shared_ptr<Flow<T>> upstream, Fn timeout_millis_selector) {
    return debounce_internal<T>(upstream, timeout_millis_selector);
}

/**
 * Internal debounce implementation.
 *
 * Transliterated from: private fun <T> Flow<T>.debounceInternal(timeoutMillisSelector: (T) -> Long)
 *
 * Implementation uses scopedFlow with produce channel and select with onTimeout.
 * Currently blocked on: select.on_timeout() not implemented in C++ port.
 */
template<typename T, typename Fn>
std::shared_ptr<Flow<T>> debounce_internal(std::shared_ptr<Flow<T>> upstream, Fn timeout_millis_selector) {
    return scoped_flow<T>([upstream, timeout_millis_selector](CoroutineScope& scope, FlowCollector<T>* downstream) {
        // Produce the values using rendezvous channel
        auto values = produce<T>(&scope, nullptr, 0, BufferOverflow::SUSPEND, CoroutineStart::DEFAULT,
            [upstream](ProducerScope<T>* producer) {
                // Collect upstream and send to channel
                // The nullptr handling with NULL_VALUE symbol would go here
                auto collector = new FunctionalCollector<T>([producer](T value) {
                    producer->send(std::move(value), nullptr);
                });
                upstream->collect(collector, nullptr);
                delete collector;
            });

        void* last_value = nullptr;
        bool done = false;

        while (!done) {
            long timeout_millis = 0;

            // Compute timeout for this value
            if (last_value != nullptr) {
                T* typed_value = static_cast<T*>(last_value);
                timeout_millis = timeout_millis_selector(*typed_value);
                if (timeout_millis < 0) {
                    throw std::invalid_argument("Debounce timeout should not be negative");
                }
                if (timeout_millis == 0) {
                    downstream->emit(*typed_value, nullptr);
                    delete typed_value;
                    last_value = nullptr;
                }
            }

            /**
             * Upstream:
             *   select<Unit> {
             *       if (lastValue != null) {
             *           onTimeout(timeoutMillis) {
             *               downstream.emit(NULL.unbox(lastValue))
             *               lastValue = null
             *           }
             *       }
             *       values.onReceiveCatching { value ->
             *           value.onSuccess { lastValue = it }
             *                .onFailure {
             *                    it?.let { throw it }
             *                    if (lastValue != null) downstream.emit(NULL.unbox(lastValue))
             *                    lastValue = DONE
             *                }
             *       }
             *   }
             */
            selects::select<void>([&](selects::SelectBuilder<void>& sel) {
                if (last_value != nullptr) {
                    sel.on_timeout(timeout_millis, [&]() {
                        downstream->emit(*static_cast<T*>(last_value), nullptr);
                        delete static_cast<T*>(last_value);
                        last_value = nullptr;
                    });
                }
                values->on_receive_catching(sel, [&](const channels::ChannelResult<T>& value) {
                    if (value.is_success()) {
                        if (last_value != nullptr) delete static_cast<T*>(last_value);
                        last_value = new T(value.get_or_throw());
                    } else {
                        if (auto cause = value.exception_or_null()) {
                            std::rethrow_exception(cause);
                        }
                        if (last_value != nullptr) {
                            downstream->emit(*static_cast<T*>(last_value), nullptr);
                            delete static_cast<T*>(last_value);
                        }
                        last_value = DONE_SENTINEL();
                        done = true;
                    }
                });
            });
        }
    });
}

// =============================================================================
// sample - emit latest value at fixed intervals
// =============================================================================

/**
 * Returns a flow that emits only the latest value emitted by the original flow
 * during the given sampling period.
 *
 * Example:
 * ```cpp
 * // With emissions every 110ms and sample(200), every other value is emitted
 * ```
 *
 * Note that the latest element is not emitted if it does not fit into the sampling window.
 *
 * @param period_millis the sampling period in milliseconds
 */
template<typename T>
std::shared_ptr<Flow<T>> sample(std::shared_ptr<Flow<T>> upstream, long period_millis) {
    if (period_millis <= 0) {
        throw std::invalid_argument("Sample period should be positive");
    }

    return scoped_flow<T>([upstream, period_millis](CoroutineScope& scope, FlowCollector<T>* downstream) {
        // Use conflated channel to keep only latest value
        auto values = produce<T>(&scope, nullptr, Channel<T>::CONFLATED, BufferOverflow::DROP_OLDEST,
            CoroutineStart::DEFAULT,
            [upstream](ProducerScope<T>* producer) {
                auto collector = new FunctionalCollector<T>([producer](T value) {
                    producer->send(std::move(value), nullptr);
                });
                upstream->collect(collector, nullptr);
                delete collector;
            });

        /**
         * Upstream:
         *   var lastValue: Any? = null
         *   val ticker = fixedPeriodTicker(periodMillis)
         *   while (lastValue !== DONE) {
         *       select<Unit> {
         *           values.onReceiveCatching { result ->
         *               result.onSuccess { lastValue = it }
         *                     .onFailure {
         *                         it?.let { throw it }
         *                         ticker.cancel(ChildCancelledException())
         *                         lastValue = DONE
         *                     }
         *           }
         *           ticker.onReceive {
         *               val value = lastValue ?: return@onReceive
         *               lastValue = null
         *               downstream.emit(NULL.unbox(value))
         *           }
         *       }
         *   }
         */
        void* last_value = nullptr;
        bool done = false;
        auto ticker = fixed_period_ticker(&scope, period_millis);
        while (!done) {
            selects::select<void>([&](selects::SelectBuilder<void>& sel) {
                values->on_receive_catching(sel, [&](const channels::ChannelResult<T>& result) {
                    if (result.is_success()) {
                        if (last_value != nullptr) delete static_cast<T*>(last_value);
                        last_value = new T(result.get_or_throw());
                    } else {
                        if (auto cause = result.exception_or_null()) {
                            std::rethrow_exception(cause);
                        }
                        ticker->cancel(std::make_exception_ptr(
                            ChildCancelledException()));
                        last_value = DONE_SENTINEL();
                        done = true;
                    }
                });
                ticker->on_receive(sel, [&](Unit) {
                    if (last_value == nullptr || last_value == DONE_SENTINEL()) return;
                    T value = *static_cast<T*>(last_value);
                    delete static_cast<T*>(last_value);
                    last_value = nullptr;
                    downstream->emit(value, nullptr);
                });
            });
        }
        if (last_value != nullptr && last_value != DONE_SENTINEL()) {
            delete static_cast<T*>(last_value);
        }
    });
}

// =============================================================================
// timeout - throw if no emission within timeout
// =============================================================================

/**
 * Returns a flow that will emit a TimeoutCancellationException if the upstream
 * doesn't emit an item within the given time.
 *
 * Note that delaying on the downstream doesn't trigger the timeout.
 *
 * @param timeout_millis Timeout in milliseconds. If non-positive, times out immediately.
 */
template<typename T>
std::shared_ptr<Flow<T>> timeout(std::shared_ptr<Flow<T>> upstream, long timeout_millis) {
    return scoped_flow<T>([upstream, timeout_millis](CoroutineScope& scope, FlowCollector<T>* downstream) {
        if (timeout_millis <= 0) {
            throw TimeoutCancellationException("Timed out immediately");
        }

        // Buffer with rendezvous and produce
        auto values = produce<T>(&scope, nullptr, Channel<T>::RENDEZVOUS, BufferOverflow::SUSPEND,
            CoroutineStart::DEFAULT,
            [upstream](ProducerScope<T>* producer) {
                auto collector = new FunctionalCollector<T>([producer](T value) {
                    producer->send(std::move(value), nullptr);
                });
                upstream->collect(collector, nullptr);
                delete collector;
            });

        /**
         * Upstream:
         *   whileSelect {
         *       values.onReceiveCatching { value ->
         *           value.onSuccess { downStream.emit(it) }
         *                .onClosed { it?.let { throw it }; return@onReceiveCatching false }
         *           return@onReceiveCatching true
         *       }
         *       onTimeout(timeout) {
         *           throw TimeoutCancellationException("Timed out waiting for $timeout")
         *       }
         *   }
         */
        selects::while_select([&]() -> bool {
            bool keep_going = false;
            selects::select<void>([&](selects::SelectBuilder<void>& sel) {
                values->on_receive_catching(sel, [&](const channels::ChannelResult<T>& value) {
                    if (value.is_success()) {
                        downstream->emit(value.get_or_throw(), nullptr);
                        keep_going = true;
                    } else {
                        if (auto cause = value.exception_or_null()) {
                            std::rethrow_exception(cause);
                        }
                        keep_going = false;
                    }
                });
                sel.on_timeout(timeout_millis, [&]() {
                    throw TimeoutCancellationException(
                        "Timed out waiting for " + std::to_string(timeout_millis) + "ms");
                });
            });
            return keep_going;
        });
    });
}

// =============================================================================
// Helper: fixedPeriodTicker
// =============================================================================

/**
 * Upstream:
 *   internal fun CoroutineScope.fixedPeriodTicker(delayMillis: Long): ReceiveChannel<Unit> {
 *       return produce(capacity = 0) {
 *           delay(delayMillis)
 *           while (true) {
 *               channel.send(Unit)
 *               delay(delayMillis)
 *           }
 *       }
 *   }
 */
inline std::shared_ptr<ReceiveChannel<Unit>> fixed_period_ticker(
    CoroutineScope* scope,
    long delay_millis) {
    return produce<Unit>(scope, nullptr, 0, BufferOverflow::SUSPEND, CoroutineStart::DEFAULT,
        [delay_millis](ProducerScope<Unit>* producer) {
            ::kotlinx::coroutines::delay(delay_millis, nullptr);
            while (true) {
                producer->send(Unit{}, nullptr);
                ::kotlinx::coroutines::delay(delay_millis, nullptr);
            }
        });
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
