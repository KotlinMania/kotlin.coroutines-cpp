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
     */
    void collect(FlowCollector<T>* collector) override {
        /*
         * TODO: STUB - StateFlow collect does not wait for updates
         *
         * Kotlin source: StateFlow.collect() in StateFlow.kt
         *
         * What's missing:
         * - Should be a suspend function: suspend fun collect(collector: FlowCollector<T>): Nothing
         * - After emitting current value, should suspend waiting for updates
         * - Should never complete normally (StateFlow is hot)
         * - Should use distinctUntilChanged semantics (don't emit if value unchanged)
         * - Requires: condition variable or suspension mechanism to wait for set_value() calls
         *
         * Current behavior: Emits current value once then returns immediately
         * Correct behavior: Emit current value, then suspend indefinitely,
         *   resuming each time value changes, never completing normally
         *
         * Dependencies:
         * - Kotlin-style suspension (Continuation<void*>* parameter)
         * - Value change notification from set_value()
         * - Proper cancellation support
         *
         * Workaround: External polling loop calling value() and comparing
         */
        std::unique_lock<std::mutex> lock(mutex_);
        T last_emitted = value_;
        lock.unlock();
        collector->emit(last_emitted);
        // Returns immediately - should suspend and wait for updates
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

// SubscriptionCountStateFlow and AbstractSharedFlow implementations moved from SharedFlow.hpp
namespace kotlinx::coroutines::flow::internal {

class SubscriptionCountStateFlow
    : public StateFlow<int>,
      public ::kotlinx::coroutines::flow::SharedFlowImpl<int>
{
public:
    explicit SubscriptionCountStateFlow(int initial_value)
        : ::kotlinx::coroutines::flow::SharedFlowImpl<int>(
            1,
            std::numeric_limits<int>::max(),
            channels::BufferOverflow::DROP_OLDEST)
    {
        this->SharedFlowImpl<int>::try_emit(initial_value);
    }

    int value() const override {
        std::lock_guard<std::recursive_mutex> lock(this->mutex());
        return this->get_last_replayed_locked();
    }

    void increment(int delta) {
        std::lock_guard<std::recursive_mutex> lock(this->mutex());
        int current = this->get_last_replayed_locked();
        this->try_emit(current + delta);
    }

    std::vector<int> get_replay_cache() const override {
        return SharedFlowImpl<int>::get_replay_cache();
    }

    void* collect(FlowCollector<int>* collector, Continuation<void*>* continuation) override {
        return SharedFlowImpl<int>::collect(collector, continuation);
    }
};

template<typename S, typename F>
inline StateFlow<int>* AbstractSharedFlow<S, F>::get_subscription_count() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!subscription_count_) {
        subscription_count_ = new SubscriptionCountStateFlow(n_collectors_);
    }
    return static_cast<StateFlow<int>*>(subscription_count_);
}

template<typename S, typename F>
inline S* AbstractSharedFlow<S, F>::allocate_slot() {
    SubscriptionCountStateFlow* subscription_count_copy = nullptr;
    S* slot = nullptr;

    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        std::vector<S*>* current_slots = slots_;
        if (!current_slots) {
            slots_ = create_slot_array(2);
            current_slots = slots_;
        } else if (n_collectors_ >= static_cast<int>(current_slots->size())) {
            auto* new_slots = create_slot_array(2 * current_slots->size());
            for (size_t i = 0; i < current_slots->size(); ++i) {
                (*new_slots)[i] = (*current_slots)[i];
            }
            delete slots_;
            slots_ = new_slots;
            current_slots = slots_;
        }

        int index = next_index_;
        while (true) {
            slot = (*current_slots)[index];
            if (!slot) {
                slot = create_slot();
                (*current_slots)[index] = slot;
            }
            index++;
            if (index >= static_cast<int>(current_slots->size())) index = 0;

            if (slot->allocate_locked(as_flow())) {
                break;
            }
        }

        next_index_ = index;
        n_collectors_++;
        subscription_count_copy = subscription_count_;
    }

    if (subscription_count_copy) {
        subscription_count_copy->increment(1);
    }

    return slot;
}

template<typename S, typename F>
inline void AbstractSharedFlow<S, F>::free_slot(S* slot) {
    SubscriptionCountStateFlow* subscription_count_copy = nullptr;
    std::vector<Continuation<Unit>*> resumes;

    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        n_collectors_--;
        subscription_count_copy = subscription_count_;

        if (n_collectors_ == 0) {
            next_index_ = 0;
        }

        resumes = slot->free_locked(as_flow());
    }

    for (auto cont : resumes) {
        if (cont) {
            cont->resume_with(Result<Unit>::success(Unit{}));
        }
    }

    if (subscription_count_copy) {
        subscription_count_copy->increment(-1);
    }
}

} // namespace kotlinx::coroutines::flow::internal
