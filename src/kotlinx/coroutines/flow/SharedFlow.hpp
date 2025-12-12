#pragma once
/**
 * @file SharedFlow.hpp
 * @brief SharedFlow and MutableSharedFlow interfaces
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/SharedFlow.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <deque>

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * A hot flow that shares emitted values among all its collectors in a broadcast fashion,
 * so that all collectors get all emitted values.
 *
 * A shared flow is called _hot_ because its active instance exists independently of the
 * presence of collectors. This is opposed to a regular Flow, such as defined by the flow builder,
 * which is _cold_ and is started separately for each collector.
 *
 * **SharedFlow is a highly-configurable generalization of StateFlow.**
 *
 * @note **CURRENT LIMITATIONS**: 
 *       - emit() and collect() are not suspending functions, breaking backpressure
 *       - collect() does not wait for new values after replay cache is consumed
 *       - Buffer overflow policies are not properly implemented
 *       - subscriptionCount is not reactive (returns snapshot instead of StateFlow)
 *
 * @note **INTENDED BEHAVIOR**: 
 *       - emit() should suspend when buffer is full (depending on overflow policy)
 *       - collect() should suspend indefinitely, emitting new values as they arrive
 *       - Should support configurable BufferOverflow (SUSPEND, DROP_OLDEST, DROP_LATEST)
 *       - subscriptionCount should return StateFlow<int> for reactive observation
 *
 * ### Thread Safety
 * SharedFlow is designed for concurrent access. Multiple collectors can subscribe
 * and multiple emitters can emit values simultaneously. All operations are
 * thread-safe through mutex protection.
 *
 * ### Backpressure
 * In the intended design, SharedFlow provides backpressure through configurable
 * buffer policies. Currently, backpressure is broken due to non-suspending emit().
 *
 * ### Usage Example
 * ```cpp
 * auto sharedFlow = std::make_shared<MutableSharedFlow<int>>(1, 10); // replay=1, buffer=10
 * 
 * // Emit values (should suspend when full, but currently doesn't)
 * sharedFlow->emit(42);
 * sharedFlow->emit(43);
 * 
 * // Collect values (should wait for new values, but currently returns after replay)
 * sharedFlow->collect([](int value) {
 *     std::cout << "Received: " << value << std::endl;
 * });
 * ```
 */
template<typename T>
struct SharedFlow : public Flow<T> {
    virtual ~SharedFlow() = default;

    /**
     * A snapshot of the replay cache.
     */
    virtual std::vector<T> replay_cache() const = 0;
};

/**
 * A mutable SharedFlow that provides functions to emit values to the flow.
 *
 * An instance of MutableSharedFlow with the given configuration parameters can be created using
 * MutableSharedFlow() constructor function.
 *
 * See SharedFlow documentation for details on shared flows.
 *
 * MutableSharedFlow is a SharedFlow that also provides the ability to emit, tryEmit, and
 * resetReplayCache, and exposes the subscriptionCount as a Flow.
 */
template<typename T>
class MutableSharedFlow : public SharedFlow<T> {
public:
    /**
     * Creates a MutableSharedFlow with the given replay cache size.
     *
     * @param replay the number of values replayed to new subscribers (cannot be negative).
     * @param extra_buffer_capacity the number of values buffered in addition to replay.
     *        emit does not suspend while there is a buffer space remaining (optional, cannot be negative, defaults to zero).
     */
    MutableSharedFlow(int replay = 0, int extra_buffer_capacity = 0)
        : replay_(replay), extra_buffer_capacity_(extra_buffer_capacity) {
        if (replay < 0) throw std::invalid_argument("replay cannot be negative");
        if (extra_buffer_capacity < 0) throw std::invalid_argument("extraBufferCapacity cannot be negative");
    }

    ~MutableSharedFlow() override = default;

    /**
     * Emits a value to this shared flow, suspending if the buffer is full.
     */
    void emit(T value) {
        /*
         * TODO: STUB - SharedFlow emit suspension not implemented
         *
         * Kotlin source: MutableSharedFlow.emit() in SharedFlow.kt
         *
         * What's missing:
         * - Should be a suspend function: suspend fun emit(value: T)
         * - When buffer is full and onBufferOverflow == SUSPEND:
         *   - Should suspend using suspendCancellableCoroutine
         *   - Resume when a collector consumes a value, freeing buffer space
         * - Requires: emitters queue to track waiting emitters
         * - Requires: proper Continuation<void*>* parameter (Kotlin-style suspend)
         *
         * Current behavior: Always succeeds immediately by dropping oldest values
         * Correct behavior: Suspend when buffer full (if SUSPEND overflow policy)
         *
         * Dependencies:
         * - BufferOverflow policy support (currently hardcoded to DROP_OLDEST behavior)
         * - suspendCancellableCoroutine integration
         * - Emitters queue for backpressure
         */
        std::lock_guard<std::mutex> lock(mutex_);
        // Add to replay cache
        replay_cache_.push_back(value);
        while (static_cast<int>(replay_cache_.size()) > replay_ + extra_buffer_capacity_) {
            replay_cache_.pop_front();
        }
        // Notify collectors
        cv_.notify_all();
    }

    /**
     * Tries to emit a value to this shared flow without suspending.
     *
     * @return true if the value was emitted, false if the buffer is full
     */
    bool try_emit(T value) {
        /*
         * TODO: STUB - SharedFlow tryEmit buffer overflow not implemented
         *
         * Kotlin source: MutableSharedFlow.tryEmit() in SharedFlow.kt
         *
         * What's missing:
         * - Should check BufferOverflow policy:
         *   - SUSPEND: return false when buffer is full (caller should use emit() instead)
         *   - DROP_OLDEST: drop oldest and return true (current behavior)
         *   - DROP_LATEST: drop this value and return true
         * - Currently always drops oldest and returns true
         *
         * Current behavior: Always succeeds by dropping oldest values
         * Correct behavior: Respect BufferOverflow policy, return false for SUSPEND when full
         *
         * Dependencies:
         * - BufferOverflow enum (should be a constructor parameter)
         * - Proper buffer size checking before deciding action
         */
        std::lock_guard<std::mutex> lock(mutex_);
        replay_cache_.push_back(value);
        while (static_cast<int>(replay_cache_.size()) > replay_ + extra_buffer_capacity_) {
            replay_cache_.pop_front();
        }
        cv_.notify_all();
        return true;
    }

    /**
     * Resets the replay cache of this shared flow to an empty state.
     * New subscribers will not receive any previously emitted values.
     */
    void reset_replay_cache() {
        std::lock_guard<std::mutex> lock(mutex_);
        replay_cache_.clear();
    }

    /**
     * A snapshot of the replay cache.
     */
    std::vector<T> replay_cache() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return std::vector<T>(replay_cache_.begin(), replay_cache_.end());
    }

    /**
     * The number of active subscribers (collectors).
     */
    int subscription_count() const {
        /*
         * TODO: PARTIAL - SharedFlow subscriptionCount is basic
         *
         * Kotlin source: MutableSharedFlow.subscriptionCount in SharedFlow.kt
         *
         * What's partially missing:
         * - In Kotlin, subscriptionCount is a StateFlow<Int> that can be collected
         * - Should allow observing subscription count changes reactively
         * - Current implementation only provides snapshot value
         *
         * Current behavior: Returns atomic int snapshot
         * Correct behavior: Return StateFlow<int> for reactive observation
         *
         * Impact: Low - snapshot is sufficient for most use cases
         */
        return subscription_count_.load();
    }

    /**
     * Collects values from this shared flow.
     */
    void collect(FlowCollector<T>* collector) override {
        /*
         * TODO: STUB - SharedFlow collect does not wait for new values
         *
         * Kotlin source: SharedFlow.collect() in SharedFlow.kt
         *
         * What's missing:
         * - Should be a suspend function: suspend fun collect(collector: FlowCollector<T>)
         * - After emitting replay cache, should suspend waiting for new values
         * - Should never complete normally (SharedFlow is hot, runs forever)
         * - Requires: condition variable wait or suspension mechanism
         * - Requires: proper coordination with emit() to wake up collectors
         *
         * Current behavior: Emits replay cache then returns immediately
         * Correct behavior: Emit replay cache, then suspend indefinitely waiting for
         *   new values, resuming each time emit() is called
         *
         * Dependencies:
         * - Kotlin-style suspension (Continuation<void*>* parameter)
         * - Collector registration/tracking in emit()
         * - Proper cleanup on collector cancellation
         *
         * Workaround: External polling loop calling collect() repeatedly
         */
        subscription_count_++;
        std::unique_lock<std::mutex> lock(mutex_);
        // First emit replay cache
        for (const auto& value : replay_cache_) {
            lock.unlock();
            collector->emit(value);
            lock.lock();
        }
        subscription_count_--;
    }

    void collect(std::function<void(T)> action) override {
        struct FunctionCollector : FlowCollector<T> {
            std::function<void(T)> action_;
            explicit FunctionCollector(std::function<void(T)> a) : action_(a) {}
            void emit(T value) override { action_(value); }
        };
        FunctionCollector fc(action);
        collect(&fc);
    }

private:
    int replay_;
    int extra_buffer_capacity_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<T> replay_cache_;
    std::atomic<int> subscription_count_{0};
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
