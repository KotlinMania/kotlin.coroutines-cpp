#pragma once
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

            // TODO(port): Full implementation requires select { onTimeout { } onReceiveCatching { } }
            // which is not yet available in C++ port. See Select.hpp on_timeout().
            //
            // The Kotlin implementation:
            // select<Unit> {
            //     if (lastValue != null) {
            //         onTimeout(timeoutMillis) {
            //             downstream.emit(NULL.unbox(lastValue))
            //             lastValue = null
            //         }
            //     }
            //     values.onReceiveCatching { value ->
            //         value.onSuccess { lastValue = it }
            //              .onFailure { ... lastValue = DONE }
            //     }
            // }

            // Temporary: just receive without timeout (breaks debounce semantics)
            auto result = values->try_receive();
            if (result.is_success()) {
                if (last_value != nullptr) {
                    delete static_cast<T*>(last_value);
                }
                last_value = new T(result.get_or_throw());
            } else if (result.is_closed()) {
                if (last_value != nullptr) {
                    downstream->emit(*static_cast<T*>(last_value), nullptr);
                    delete static_cast<T*>(last_value);
                }
                done = true;
            }
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

        // TODO(port): Full implementation requires:
        // 1. fixedPeriodTicker(period_millis) - a produce channel that emits Unit at intervals
        // 2. select { values.onReceiveCatching { } ticker.onReceive { } }
        //
        // Transliterated from Kotlin:
        // var lastValue: Any? = null
        // val ticker = fixedPeriodTicker(periodMillis)
        // while (lastValue !== DONE) {
        //     select<Unit> {
        //         values.onReceiveCatching { result -> ... }
        //         ticker.onReceive { ... emit lastValue ... }
        //     }
        // }

        // Temporary: pass-through (breaks sample semantics)
        void* last_value = nullptr;
        bool done = false;
        while (!done) {
            auto result = values->try_receive();
            if (result.is_success()) {
                if (last_value != nullptr) {
                    delete static_cast<T*>(last_value);
                }
                last_value = new T(result.get_or_throw());
                // In real impl, only emit on ticker, not on every receive
                downstream->emit(*static_cast<T*>(last_value), nullptr);
            } else if (result.is_closed()) {
                done = true;
            }
        }
        if (last_value != nullptr) {
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

        // TODO(port): Full implementation requires whileSelect with onTimeout:
        // whileSelect {
        //     values.onReceiveCatching { value ->
        //         value.onSuccess { downStream.emit(it) }
        //              .onClosed { it?.let { throw it }; return false }
        //         return true
        //     }
        //     onTimeout(timeout) {
        //         throw TimeoutCancellationException("Timed out waiting for $timeout")
        //     }
        // }

        // Temporary: collect without timeout (breaks timeout semantics)
        bool done = false;
        while (!done) {
            auto result = values->try_receive();
            if (result.is_success()) {
                downstream->emit(result.get_or_throw(), nullptr);
            } else if (result.is_closed()) {
                auto cause = result.exception_or_null();
                if (cause) {
                    std::rethrow_exception(cause);
                }
                done = true;
            }
        }
    });
}

// =============================================================================
// Helper: fixedPeriodTicker
// =============================================================================

/**
 * Creates a channel that emits Unit at fixed intervals.
 *
 * Transliterated from: internal fun CoroutineScope.fixedPeriodTicker(delayMillis: Long)
 *
 * TODO(port): Requires delay() suspend function to work properly.
 */
inline std::shared_ptr<ReceiveChannel<Unit>> fixed_period_ticker(
    CoroutineScope* scope,
    long delay_millis
) {
    (void)delay_millis;  // Used when delay() is implemented
    return produce<Unit>(scope, nullptr, 0, BufferOverflow::SUSPEND, CoroutineStart::DEFAULT,
        [](ProducerScope<Unit>* producer) {
            // TODO(port): Initial delay(delay_millis) here

            while (true) {
                producer->send(Unit{}, nullptr);
                // TODO(port): delay(delay_millis) here
            }
        });
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
