#include <string>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ (first-pass, syntax-only)
// Original: kotlinx-coroutines-core/common/src/flow/operators/Delay.kt
//
// TODO: Implement coroutine semantics (suspend functions, delay, select)
// TODO: Map Kotlin Flow types to C++ equivalents
// TODO: Implement Duration type and conversion utilities
// TODO: Implement Channel types and producer/consumer patterns
// TODO: Map selects and onReceiveCatching
// TODO: Implement TimeoutCancellationException
// TODO: Implement scopedFlow helper

#pragma once

// @file:JvmMultifileClass
// @file:JvmName("FlowKt")

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.channels.*
// TODO: import kotlinx.coroutines.flow.internal.*
// TODO: import kotlinx.coroutines.selects.*
// TODO: import kotlin.jvm.*
// TODO: import kotlin.time.*

/* Scaffolding for Knit code examples
<!--- TEST_NAME FlowDelayTest -->
<!--- PREFIX .*-duration-.*
----- INCLUDE .*-duration-.*
----- INCLUDE .*
// import kotlinx.coroutines.*// import kotlinx.coroutines.flow.*// import kotlin::time.Duration.Companion.milliseconds
auto main() = runBlocking {
----- SUFFIX .*
.toList().joinTostd::string().let { println(it) } }
-->
*/

/**
 * Returns a flow that mirrors the original flow, but filters out values
 * that are followed by the newer values within the given [timeout][timeoutMillis].
 * The latest value is always emitted.
 *
 * Example:
 *
 * ```kotlin
 * flow {
 *     emit(1)
 *     delay(90)
 *     emit(2)
 *     delay(90)
 *     emit(3)
 *     delay(1010)
 *     emit(4)
 *     delay(1010)
 *     emit(5)
 * }.debounce(1000)
 * ```
 * <!--- KNIT example-delay-01.kt -->
 *
 * produces the following emissions
 *
 * ```text
 * 3, 4, 5
 * ```
 * <!--- TEST -->
 *
 * Note that the resulting flow does not emit anything as long as the original flow emits
 * items faster than every [timeoutMillis] milliseconds.
 */
// @FlowPreview
template<typename T>
Flow<T> debounce(Flow<T> flow, long timeout_millis) {
    // require(timeoutMillis >= 0L)
    if (!(timeout_millis >= 0L)) {
        throw std::invalid_argument("Debounce timeout should not be negative");
    }
    if (timeout_millis == 0L) return flow;
    return debounce_internal(flow, [timeout_millis](T) { return timeout_millis; });
}

/**
 * Returns a flow that mirrors the original flow, but filters out values
 * that are followed by the newer values within the given [timeout][timeoutMillis].
 * The latest value is always emitted.
 *
 * A variation of [debounce] that allows specifying the timeout value dynamically.
 *
 * Example:
 *
 * ```kotlin
 * flow {
 *     emit(1)
 *     delay(90)
 *     emit(2)
 *     delay(90)
 *     emit(3)
 *     delay(1010)
 *     emit(4)
 *     delay(1010)
 *     emit(5)
 * }.debounce {
 *     if (it == 1) {
 *         0L
 *     } else {
 *         1000L
 *     }
 * }
 * ```
 * <!--- KNIT example-delay-02.kt -->
 *
 * produces the following emissions
 *
 * ```text
 * 1, 3, 4, 5
 * ```
 * <!--- TEST -->
 *
 * Note that the resulting flow does not emit anything as long as the original flow emits
 * items faster than every [timeoutMillis] milliseconds.
 *
 * @param timeoutMillis [T] is the emitted value and the return value is timeout in milliseconds.
 */
// @FlowPreview
// @OverloadResolutionByLambdaReturnType
template<typename T, typename Fn>
Flow<T> debounce(Flow<T> flow, Fn timeout_millis) {
    return debounce_internal(flow, timeout_millis);
}

/**
 * Returns a flow that mirrors the original flow, but filters out values
 * that are followed by the newer values within the given [timeout].
 * The latest value is always emitted.
 *
 * Example:
 *
 * ```kotlin
 * flow {
 *     emit(1)
 *     delay(90.milliseconds)
 *     emit(2)
 *     delay(90.milliseconds)
 *     emit(3)
 *     delay(1010.milliseconds)
 *     emit(4)
 *     delay(1010.milliseconds)
 *     emit(5)
 * }.debounce(1000.milliseconds)
 * ```
 * <!--- KNIT example-delay-duration-01.kt -->
 *
 * produces the following emissions
 *
 * ```text
 * 3, 4, 5
 * ```
 * <!--- TEST -->
 *
 * Note that the resulting flow does not emit anything as long as the original flow emits
 * items faster than every [timeout] milliseconds.
 */
// @FlowPreview
template<typename T>
Flow<T> debounce(Flow<T> flow, Duration timeout) {
    return debounce(flow, to_delay_millis(timeout));
}

/**
 * Returns a flow that mirrors the original flow, but filters out values
 * that are followed by the newer values within the given [timeout].
 * The latest value is always emitted.
 *
 * A variation of [debounce] that allows specifying the timeout value dynamically.
 *
 * Example:
 *
 * ```kotlin
 * flow {
 *     emit(1)
 *     delay(90.milliseconds)
 *     emit(2)
 *     delay(90.milliseconds)
 *     emit(3)
 *     delay(1010.milliseconds)
 *     emit(4)
 *     delay(1010.milliseconds)
 *     emit(5)
 * }.debounce {
 *     if (it == 1) {
 *         0.milliseconds
 *     } else {
 *         1000.milliseconds
 *     }
 * }
 * ```
 * <!--- KNIT example-delay-duration-02.kt -->
 *
 * produces the following emissions
 *
 * ```text
 * 1, 3, 4, 5
 * ```
 * <!--- TEST -->
 *
 * Note that the resulting flow does not emit anything as long as the original flow emits
 * items faster than every [timeout] unit.
 *
 * @param timeout [T] is the emitted value and the return value is timeout in [Duration].
 */
// @FlowPreview
// @JvmName("debounceDuration")
// @OverloadResolutionByLambdaReturnType
template<typename T, typename Fn>
Flow<T> debounce_duration(Flow<T> flow, Fn timeout) {
    return debounce_internal(flow, [timeout](T emitted_item) {
        return to_delay_millis(timeout(emitted_item));
    });
}

template<typename T, typename Fn>
Flow<T> debounce_internal(Flow<T> flow, Fn timeout_millis_selector) {
    // TODO: implement scopedFlow
    return scoped_flow(flow, [timeout_millis_selector](auto downstream) {
        // Produce the values using the default (rendezvous) channel
        // TODO: implement produce
        auto values = produce([&flow]() {
            flow.collect([](T value) {
                send(value ? value : NULL);
            });
        });
        // Now consume the values
        void* last_value = nullptr;
        while (last_value != DONE) {
            long timeout_millis = 0L; // will be always computed when lastValue != nullptr
            // Compute timeout for this value
            if (last_value != nullptr) {
                timeout_millis = timeout_millis_selector(NULL.unbox(last_value));
                // require(timeoutMillis >= 0L)
                if (!(timeout_millis >= 0L)) {
                    throw std::invalid_argument("Debounce timeout should not be negative");
                }
                if (timeout_millis == 0L) {
                    downstream.emit(NULL.unbox(last_value));
                    last_value = nullptr; // Consume the value
                }
            }
            // assert invariant: lastValue != nullptr implies timeoutMillis > 0
            // assert { lastValue == nullptr || timeoutMillis > 0 }
            // wait for the next value with timeout
            // TODO: implement select
            select<void>([&]() {
                // Set timeout when lastValue exists and is not consumed yet
                if (last_value != nullptr) {
                    on_timeout(timeout_millis, [&]() {
                        downstream.emit(NULL.unbox(last_value));
                        last_value = nullptr; // Consume the value
                    });
                }
                values.on_receive_catching([&](auto value) {
                    value
                        .on_success([&](auto it) { last_value = it; })
                        .on_failure([&](auto it) {
                            if (it) throw it;
                            // If closed normally, emit the latest value
                            if (last_value != nullptr) downstream.emit(NULL.unbox(last_value));
                            last_value = DONE;
                        });
                });
            });
        }
    });
}

/**
 * Returns a flow that emits only the latest value emitted by the original flow during the given sampling [period][periodMillis].
 *
 * Example:
 *
 * ```kotlin
 * flow {
 *     repeat(10) {
 *         emit(it)
 *         delay(110)
 *     }
 * }.sample(200)
 * ```
 * <!--- KNIT example-delay-03.kt -->
 *
 * produces the following emissions
 *
 * ```text
 * 1, 3, 5, 7, 9
 * ```
 * <!--- TEST -->
 *
 * Note that the latest element is not emitted if it does not fit into the sampling window.
 */
// @FlowPreview
template<typename T>
Flow<T> sample(Flow<T> flow, long period_millis) {
    // require(periodMillis > 0)
    if (!(period_millis > 0)) {
        throw std::invalid_argument("Sample period should be positive");
    }
    return scoped_flow(flow, [period_millis](auto downstream) {
        auto values = produce(:CONFLATED Channel, [&flow]() {
            flow.collect([](T value) { send(value ? value : NULL); });
        });
        void* last_value = nullptr;
        auto ticker = fixed_period_ticker(period_millis);
        while (last_value != DONE) {
            select<void>([&]() {
                values.on_receive_catching([&](auto result) {
                    result
                        .on_success([&](auto it) { last_value = it; })
                        .on_failure([&](auto it) {
                            if (it) throw it;
                            ticker.cancel(ChildCancelledException());
                            last_value = DONE;
                        });
                });

                // todo: shall be start sampling only when an element arrives or sample aways as here*
                ticker.on_receive([&]() {
                    auto value = last_value;
                    if (!value) return;
                    last_value = nullptr; // Consume the value
                    downstream.emit(NULL.unbox(value));
                });
            });
        }
    });
}

/*
 * TODO this design (and design of the corresponding operator) depends on #540
 */
// TODO: implement
template<typename Scope>
ReceiveChannel<void> fixed_period_ticker(Scope scope, long delay_millis) {
    return produce(scope, 0, [delay_millis]() {
        delay(delay_millis);
        while (true) {
            channel.send({});
            delay(delay_millis);
        }
    });
}

/**
 * Returns a flow that emits only the latest value emitted by the original flow during the given sampling [period].
 *
 * Example:
 *
 * ```kotlin
 * flow {
 *     repeat(10) {
 *         emit(it)
 *         delay(110.milliseconds)
 *     }
 * }.sample(200.milliseconds)
 * ```
 * <!--- KNIT example-delay-duration-03.kt -->
 *
 * produces the following emissions
 *
 * ```text
 * 1, 3, 5, 7, 9
 * ```
 * <!--- TEST -->
 *
 * Note that the latest element is not emitted if it does not fit into the sampling window.
 */
// @FlowPreview
template<typename T>
Flow<T> sample(Flow<T> flow, Duration period) {
    return sample(flow, to_delay_millis(period));
}

/**
 * Returns a flow that will emit a [TimeoutCancellationException] if the upstream doesn't emit an item within the given time.
 *
 * Example:
 *
 * ```kotlin
 * flow {
 *     emit(1)
 *     delay(100)
 *     emit(2)
 *     delay(100)
 *     emit(3)
 *     delay(1000)
 *     emit(4)
 * }.timeout(100.milliseconds).catch { exception ->
 *     if (exception is TimeoutCancellationException) {
 *         // Catch the TimeoutCancellationException emitted above.
 *         // Emit desired item on timeout.
 *         emit(-1)
 *     } else {
 *         // Throw other exceptions.
 *         throw exception
 *     }
 * }.onEach {
 *     delay(300) // This will not cause a timeout
 * }
 * ```
 * <!--- KNIT example-timeout-duration-01.kt -->
 *
 * produces the following emissions
 *
 * ```text
 * 1, 2, 3, -1
 * ```
 * <!--- TEST -->
 *
 * Note that delaying on the downstream doesn't trigger the timeout.
 *
 * @param timeout Timeout duration. If non-positive, the flow is timed out immediately
 */
// @FlowPreview
template<typename T>
Flow<T> timeout(Flow<T> flow, Duration timeout) {
    return timeout_internal(flow, timeout);
}

template<typename T>
Flow<T> timeout_internal(Flow<T> flow, Duration timeout) {
    return scoped_flow(flow, [timeout](auto down_stream) {
        if (timeout <= Duration::ZERO) {
            throw TimeoutCancellationException("Timed out immediately");
        }
        auto values = buffer(flow, :RENDEZVOUS Channel).produce_in(/* this */);
        while_select([&]() {
            values.on_receive_catching([&](auto value) {
                value.on_success([&](auto it) {
                    down_stream.emit(it);
                }).on_closed([&](auto it) {
                    if (it) throw it;
                    return false;
                });
                return true;
            });
            on_timeout(timeout, [&]() {
                throw TimeoutCancellationException("Timed out waiting for " + to_string(timeout));
            });
        });
    });
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
