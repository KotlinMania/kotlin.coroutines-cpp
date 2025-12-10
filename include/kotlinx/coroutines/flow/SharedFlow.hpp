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
     *
     * TODO: Implement proper suspension
     */
    void emit(T value) {
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
        std::lock_guard<std::mutex> lock(mutex_);
        replay_cache_.push_back(value);
        while (static_cast<int>(replay_cache_.size()) > replay_ + extra_buffer_capacity_) {
            replay_cache_.pop_front();
        }
        cv_.notify_all();
        return true; // TODO: Implement proper buffer overflow handling
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
     *
     * TODO: Implement proper subscription counting
     */
    int subscription_count() const {
        return subscription_count_.load();
    }

    /**
     * Collects values from this shared flow.
     */
    void collect(FlowCollector<T>* collector) override {
        subscription_count_++;
        // TODO: Implement proper collection with replay and waiting for new values
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
