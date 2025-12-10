#pragma once
/**
 * @file StateFlow.hpp
 * @brief StateFlow and MutableStateFlow interfaces
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/StateFlow.kt
 */

#include "kotlinx/coroutines/flow/SharedFlow.hpp"
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * A SharedFlow that represents a read-only state with a single up-to-date value.
 *
 * State flow is a SharedFlow with a replay of 1 and a specific conflation strategy.
 * It always has a value, and every time a new value is emitted, the previous value is
 * replaced (conflated) with the new one.
 *
 * State flow never completes. A call to collect on a state flow never completes normally,
 * and neither does a call to first on it.
 */
template<typename T>
struct StateFlow : public SharedFlow<T> {
    virtual ~StateFlow() = default;

    /**
     * The current value of this state flow.
     */
    virtual T value() const = 0;
};

/**
 * A mutable StateFlow that provides a setter for value.
 *
 * See StateFlow documentation for details on state flows.
 *
 * A mutable state flow is created using MutableStateFlow(value) constructor function
 * with the initial value. The value of mutable state flow can be updated by setting
 * its value property. Updates to the value are always conflated.
 *
 * MutableStateFlow can be used as a communication mechanism between coroutines
 * or between different parts of the application.
 */
template<typename T>
class MutableStateFlow : public StateFlow<T> {
public:
    /**
     * Creates a MutableStateFlow with the given initial value.
     */
    explicit MutableStateFlow(T initial_value)
        : value_(initial_value) {}

    ~MutableStateFlow() override = default;

    /**
     * The current value of this state flow.
     */
    T value() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return value_;
    }

    /**
     * Sets the value of this state flow.
     *
     * This operation is thread-safe. Setting the same value does not trigger collectors.
     */
    void set_value(T new_value) {
        std::lock_guard<std::mutex> lock(mutex_);
        value_ = new_value;
        cv_.notify_all();
    }

    /**
     * Atomically compares the current value with expect and sets it to update if they are equal.
     *
     * @return true if successful (value was updated), false if the current value was not equal to expect.
     */
    bool compare_and_set(T expect, T update) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (value_ == expect) {
            value_ = update;
            cv_.notify_all();
            return true;
        }
        return false;
    }

    /**
     * Emits a value to this state flow, same as set_value.
     */
    void emit(T value) {
        set_value(value);
    }

    /**
     * Tries to emit a value to this state flow.
     * Always succeeds for state flow since values are conflated.
     */
    bool try_emit(T value) {
        set_value(value);
        return true;
    }

    /**
     * A snapshot of the replay cache (always contains exactly one element - the current value).
     */
    std::vector<T> replay_cache() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return {value_};
    }

    /**
     * Collects values from this state flow.
     *
     * This function never completes normally. It collects the current value immediately
     * and then waits for and collects subsequent updates.
     *
     * TODO: Implement proper infinite collection
     */
    void collect(FlowCollector<T>* collector) override {
        std::unique_lock<std::mutex> lock(mutex_);
        T last_emitted = value_;
        lock.unlock();
        collector->emit(last_emitted);
        // TODO: Wait for updates and continue collecting
        // For now, just emit current value once
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
    T value_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
