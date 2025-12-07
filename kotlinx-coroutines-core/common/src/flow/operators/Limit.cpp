// Transliterated from Kotlin to C++ (first-pass, syntax-only)
// Original: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt
//
// TODO: Implement coroutine semantics (suspend functions)
// TODO: Map Kotlin Flow types to C++ equivalents
// TODO: Implement AbortFlowException and ownership marker pattern
// TODO: Implement collectWhile utility
// TODO: Implement emitAbort pattern for early termination
// TODO: Handle safe vs unsafe flow builders

#pragma once

// @file:JvmMultifileClass
// @file:JvmName("FlowKt")

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.flow.internal.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.jvm.*
// TODO: import kotlinx.coroutines.flow.flow as safeFlow
// TODO: import kotlinx.coroutines.flow.internal.unsafeFlow as flow

/**
 * Returns a flow that ignores first [count] elements.
 * Throws [IllegalArgumentException] if [count] is negative.
 */
template<typename T>
Flow<T> drop(Flow<T> flow, int count) {
    // require(count >= 0)
    if (!(count >= 0)) {
        throw std::invalid_argument("Drop count should be non-negative, but had " + std::to_string(count));
    }
    return unsafe_flow<T>([flow, count](FlowCollector<T> collector) {
        int skipped = 0;
        flow.collect([&](T value) {
            if (skipped >= count) {
                collector.emit(value);
            } else {
                ++skipped;
            }
        });
    });
}

/**
 * Returns a flow containing all elements except first elements that satisfy the given predicate.
 */
template<typename T, typename Predicate>
Flow<T> drop_while(Flow<T> flow, Predicate predicate) {
    return unsafe_flow<T>([flow, predicate](FlowCollector<T> collector) {
        bool matched = false;
        flow.collect([&](T value) {
            if (matched) {
                collector.emit(value);
            } else if (!predicate(value)) {
                matched = true;
                collector.emit(value);
            }
        });
    });
}

/**
 * Returns a flow that contains first [count] elements.
 * When [count] elements are consumed, the original flow is cancelled.
 * Throws [IllegalArgumentException] if [count] is not positive.
 */
template<typename T>
Flow<T> take(Flow<T> flow, int count) {
    // require(count > 0)
    if (!(count > 0)) {
        throw std::invalid_argument("Requested element count " + std::to_string(count) + " should be positive");
    }
    return unsafe_flow<T>([flow, count](FlowCollector<T> collector) {
        void* ownership_marker = new int{}; // Any() equivalent
        int consumed = 0;
        try {
            flow.collect([&](T value) {
                // Note: this for take is not written via collectWhile on purpose.
                // It checks condition first and then makes a tail-call to either emit or emitAbort.
                // This way normal execution does not require a state machine, only a termination (emitAbort).
                // See "TakeBenchmark" for comparision of different approaches.
                if (++consumed < count) {
                    return collector.emit(value);
                } else {
                    return emit_abort(collector, value, ownership_marker);
                }
            });
        } catch (AbortFlowException& e) {
            e.check_ownership(ownership_marker);
        }
        delete (int*)ownership_marker;
    });
}

template<typename T>
void emit_abort(FlowCollector<T>& collector, T value, void* ownership_marker) {
    collector.emit(value);
    throw AbortFlowException(ownership_marker);
}

/**
 * Returns a flow that contains first elements satisfying the given [predicate].
 *
 * Note, that the resulting flow does not contain the element on which the [predicate] returned `false`.
 * See [transformWhile] for a more flexible operator.
 */
template<typename T, typename Predicate>
Flow<T> take_while(Flow<T> flow, Predicate predicate) {
    return unsafe_flow<T>([flow, predicate](FlowCollector<T> collector) {
        // This return is needed to work around a bug in JS BE: KT-39227
        return collect_while(flow, [&](T value) {
            if (predicate(value)) {
                collector.emit(value);
                return true;
            } else {
                return false;
            }
        });
    });
}

/**
 * Applies [transform] function to each value of the given flow while this
 * function returns `true`.
 *
 * The receiver of the `transformWhile` is [FlowCollector] and thus `transformWhile` is a
 * flexible function that may transform emitted element, skip it or emit it multiple times.
 *
 * This operator generalizes [takeWhile] and can be used as a building block for other operators.
 * For example, a flow of download progress messages can be completed when the
 * download is done but emit this last message (unlike `takeWhile`):
 *
 * ```
 * fun Flow<DownloadProgress>.completeWhenDone(): Flow<DownloadProgress> =
 *     transformWhile { progress ->
 *         emit(progress) // always emit progress
 *         !progress.isDone() // continue while download is not done
 *     }
 * ```
 */
template<typename T, typename R, typename Transform>
Flow<R> transform_while(Flow<T> flow, Transform transform) {
    return safe_flow<R>([flow, transform](FlowCollector<R> collector) {
        // This return is needed to work around a bug in JS BE: KT-39227
        return collect_while(flow, [&](T value) {
            return transform(collector, value);
        });
    });
}

// Internal building block for non-tailcalling flow-truncating operators
template<typename T, typename Predicate>
void collect_while(Flow<T> flow, Predicate predicate) {
    struct PredicateCollector : FlowCollector<T> {
        Predicate pred;

        PredicateCollector(Predicate p) : pred(p) {}

        void emit(T value) /* override */ {
            // Note: we are checking predicate first, then throw. If the predicate does suspend (calls emit, for example)
            // the resulting code is never tail-suspending and produces a state-machine
            if (!pred(value)) {
                throw AbortFlowException(this);
            }
        }
    };

    PredicateCollector collector{predicate};
    try {
        flow.collect(collector);
    } catch (AbortFlowException& e) {
        e.check_ownership(&collector);
        // The task might have been cancelled before AbortFlowException was thrown.
        current_coroutine_context().ensure_active();
    }
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
