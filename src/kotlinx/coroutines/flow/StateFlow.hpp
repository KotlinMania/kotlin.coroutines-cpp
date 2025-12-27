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
 * A [SharedFlow] that represents a read-only state with a single updatable data [value] that emits updates
 * to the value to its collectors. A state flow is a _hot_ flow because its active instance exists independently
 * of the presence of collectors. Its current value can be retrieved via the [value] property.
 *
 * **State flow never completes**. A call to [Flow::collect] on a state flow never completes normally, and
 * neither does a coroutine started by the [Flow::launch_in] function. An active collector of a state flow is called a _subscriber_.
 *
 * A [mutable state flow][MutableStateFlow] is created using `MutableStateFlow(value)` constructor function with
 * the initial value. The value of mutable state flow can be updated by setting its [value] property.
 * Updates to the [value] are always [conflated][Flow::conflate]. So a slow collector skips fast updates,
 * but always collects the most recently emitted value.
 *
 * [StateFlow] is useful as a data-model class to represent any kind of state.
 * Derived values can be defined using various operators on the flows, with [combine] operator being especially
 * useful to combine values from multiple state flows using arbitrary functions.
 *
 * For example, the following class encapsulates an integer state and increments its value on each call to `inc`:
 *
 * ```cpp
 * class CounterModel {
 *     MutableStateFlow<int> counter_{0}; // private mutable state flow
 * public:
 *     StateFlow<int>& counter() { return counter_; } // exposed as read-only state flow
 *
 *     void inc() {
 *         counter_.update([](int count) { return count + 1; }); // atomic, safe for concurrent use
 *     }
 * };
 * ```
 *
 * ### Strong equality-based conflation
 *
 * Values in state flow are conflated using `operator==` comparison in a similar way to
 * [distinct_until_changed] operator. It is used to conflate incoming updates
 * to [value][MutableStateFlow::value] in [MutableStateFlow] and to suppress emission of the values to collectors
 * when new value is equal to the previously emitted one.
 *
 * ### State flow is a shared flow
 *
 * State flow is a special-purpose, high-performance, and efficient implementation of [SharedFlow] for the narrow,
 * but widely used case of sharing a state. See the [SharedFlow] documentation for the basic rules,
 * constraints, and operators that are applicable to all shared flows.
 *
 * State flow always has an initial value, replays one most recent value to new subscribers, does not buffer any
 * more values, but keeps the last emitted one, and does not support [reset_replay_cache][MutableSharedFlow::reset_replay_cache].
 *
 * ### Concurrency
 *
 * All methods of state flow are **thread-safe** and can be safely invoked from concurrent coroutines without
 * external synchronization.
 *
 * ### Implementation notes
 *
 * State flow implementation is optimized for memory consumption and allocation-freedom. It uses a lock to ensure
 * thread-safety, but suspending collector coroutines are resumed outside of this lock to avoid dead-locks when
 * using unconfined coroutines. Adding new subscribers has `O(1)` amortized cost, but updating a [value] has `O(N)`
 * cost, where `N` is the number of active subscribers.
 *
 * Transliterated from:
 * public interface StateFlow<out T> : SharedFlow<T>
 */
template<typename T>
struct StateFlow : public SharedFlow<T> {
    virtual ~StateFlow() = default;

    /**
     * The current value of this state flow.
     *
     * Transliterated from:
     * public val value: T
     */
    virtual T value() const = 0;
};

/**
 * A mutable [StateFlow] that provides a setter for [value].
 * An instance of `MutableStateFlow` with the given initial `value` can be created using
 * `MutableStateFlow(value)` constructor function.
 *
 * See the [StateFlow] documentation for details on state flows.
 * Note that all emission-related operators, such as [value]'s setter, [emit], and [try_emit], are conflated
 * using `operator==`.
 *
 * ### Comparison with SharedFlow
 *
 * `MutableStateFlow(initial_value)` is a shared flow with the following parameters:
 *
 * ```cpp
 * auto shared = MutableSharedFlow<T>(
 *     /*replay=*/1,
 *     /*extra_buffer_capacity=*/0,
 *     /*on_buffer_overflow=*/BufferOverflow::DROP_OLDEST
 * );
 * shared.try_emit(initial_value); // emit the initial value
 * auto state = distinct_until_changed(shared); // get StateFlow-like behavior
 * ```
 *
 * Transliterated from:
 * public interface MutableStateFlow<T> : StateFlow<T>, MutableSharedFlow<T>
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
        subscription_count_ = std::make_unique<SubscriptionCountStateFlow>(n_collectors_);
    }
    return static_cast<StateFlow<int>*>(subscription_count_.get());
}

template<typename S, typename F>
inline S* AbstractSharedFlow<S, F>::allocate_slot() {
    SubscriptionCountStateFlow* subscription_count_copy = nullptr;
    S* slot = nullptr;

    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        // Kotlin: val slots = when (val curSlots = slots) { null -> createSlotArray(2).also { slots = it } ... }
        if (!slots_) {
            slots_ = create_slot_array(2);
        } else if (n_collectors_ >= static_cast<int>(slots_->size())) {
            // Kotlin: curSlots.copyOf(2 * curSlots.size).also { slots = it }
            // Transfer ownership of existing slots to new, larger vector
            auto new_slots = create_slot_array(2 * slots_->size());
            for (size_t i = 0; i < slots_->size(); ++i) {
                (*new_slots)[i] = std::move((*slots_)[i]);
            }
            slots_ = std::move(new_slots);
        }

        int index = next_index_;
        while (true) {
            // Kotlin: slot = slots[index] ?: createSlot().also { slots[index] = it }
            if (!(*slots_)[index]) {
                (*slots_)[index] = create_slot();
            }
            slot = (*slots_)[index].get();
            index++;
            if (index >= static_cast<int>(slots_->size())) index = 0;

            if (slot->allocate_locked(as_flow())) {
                break;
            }
        }

        next_index_ = index;
        n_collectors_++;
        subscription_count_copy = subscription_count_.get();
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
        subscription_count_copy = subscription_count_.get();

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
