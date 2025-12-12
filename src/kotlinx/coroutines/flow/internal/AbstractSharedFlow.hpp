#pragma once
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/AbstractSharedFlow.kt
//
// Kotlin imports:
// - kotlinx.coroutines.*
// - kotlinx.coroutines.channels.*
// - kotlinx.coroutines.flow.*
// - kotlinx.coroutines.internal.*

#include <vector>
#include <functional>
#include <mutex>
#include <algorithm>
#include <memory>
#include <limits>
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Unit.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"

// Forward declarations (full definitions come from SharedFlow.hpp include below)
namespace kotlinx::coroutines::flow {
    template<typename T> class StateFlow;
}

namespace kotlinx::coroutines::flow::internal {

// Kotlin: @JvmField internal val EMPTY_RESUMES = arrayOfNulls<Continuation<Unit>?>(0)
inline const std::vector<Continuation<Unit>*> EMPTY_RESUMES = {};

// Forward declaration for SubscriptionCountStateFlow (defined at end of file after SharedFlow.hpp include)
class SubscriptionCountStateFlow;

/**
 * Base class for shared flow slot implementations.
 *
 * Kotlin equivalent:
 * internal abstract class AbstractSharedFlowSlot<F> {
 *     abstract fun allocateLocked(flow: F): Boolean
 *     abstract fun freeLocked(flow: F): Array<Continuation<Unit>?>
 * }
 */
template<typename F>
class AbstractSharedFlowSlot {
public:
    virtual ~AbstractSharedFlowSlot() = default;

    /**
     * Attempts to allocate this slot for the given flow.
     * @return true if allocation was successful (slot was free), false otherwise
     */
    virtual bool allocate_locked(F* flow) = 0;

    /**
     * Frees this slot and returns continuations to resume.
     * @return Array of continuations that were waiting and should be resumed
     */
    virtual std::vector<Continuation<Unit>*> free_locked(F* flow) = 0;
};

/**
 * Abstract base class for shared flow implementations.
 *
 * Kotlin equivalent:
 * internal abstract class AbstractSharedFlow<S : AbstractSharedFlowSlot<*>> : SynchronizedObject() {
 *     protected var slots: Array<S?>? = null
 *         private set
 *     protected var nCollectors = 0
 *         private set
 *     private var nextIndex = 0
 *     private var _subscriptionCount: SubscriptionCountStateFlow? = null
 *
 *     val subscriptionCount: StateFlow<Int>
 *     protected abstract fun createSlot(): S
 *     protected abstract fun createSlotArray(size: Int): Array<S?>
 *     protected fun allocateSlot(): S
 *     protected fun freeSlot(slot: S)
 *     protected inline fun forEachSlotLocked(block: (S) -> Unit)
 * }
 *
 * @tparam S Slot type (must inherit from AbstractSharedFlowSlot<F>)
 * @tparam F Flow type (the derived class itself - CRTP pattern)
 */
template<typename S, typename F>
class AbstractSharedFlow {
private:
    // Kotlin: protected var slots: Array<S?>? = null (with private set)
    std::vector<S*>* slots_ = nullptr;

    // Kotlin: protected var nCollectors = 0 (with private set)
    int n_collectors_ = 0;

    // Kotlin: private var nextIndex = 0
    int next_index_ = 0;

    // Kotlin: private var _subscriptionCount: SubscriptionCountStateFlow? = null
    SubscriptionCountStateFlow* subscription_count_ = nullptr;

    // Mutex for synchronization (replaces Kotlin's SynchronizedObject)
    mutable std::recursive_mutex mutex_;

protected:
    // Protected getters for slots and nCollectors (matching Kotlin's protected var with private set)
    std::vector<S*>* get_slots() const { return slots_; }
    int get_n_collectors() const { return n_collectors_; }

    // Expose the shared lock to derived classes to mirror Kotlin synchronized(this).
    std::recursive_mutex& mutex() const { return mutex_; }

    /**
     * Creates a new slot instance.
     * Kotlin: protected abstract fun createSlot(): S
     */
    virtual S* create_slot() = 0;

    /**
     * Creates a new slot array of the given size.
     * Kotlin: protected abstract fun createSlotArray(size: Int): Array<S?>
     */
    virtual std::vector<S*>* create_slot_array(int size) = 0;

    // Derived class must implement this to return itself as F* (CRTP pattern)
    virtual F* as_flow() = 0;

public:
    virtual ~AbstractSharedFlow() {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (slots_) {
            for (auto slot : *slots_) {
                delete slot;
            }
            delete slots_;
        }
        delete subscription_count_;
    }

    /**
     * Returns a StateFlow that represents the number of active collectors.
     *
     * Kotlin:
     * val subscriptionCount: StateFlow<Int>
     *     get() = synchronized(this) {
     *         _subscriptionCount ?: SubscriptionCountStateFlow(nCollectors).also {
     *             _subscriptionCount = it
     *         }
     *     }
     *
     * Note: Implementation moved to end of file after SubscriptionCountStateFlow is defined
     */
    virtual StateFlow<int>* get_subscription_count();

protected:
    /**
     * Allocates a slot for a new collector.
     *
     * Implementation moved to end of file after SubscriptionCountStateFlow is defined.
     */
    S* allocate_slot();

    /**
     * Frees a slot and resumes any waiting continuations.
     *
     * Implementation moved to end of file after SubscriptionCountStateFlow is defined.
     */
    void free_slot(S* slot);

    /**
     * Iterates over all allocated slots while holding the lock.
     *
     * Kotlin:
     * protected inline fun forEachSlotLocked(block: (S) -> Unit) {
     *     if (nCollectors == 0) return
     *     slots?.forEach { slot ->
     *         if (slot != null) block(slot)
     *     }
     * }
     */
    void for_each_slot_locked(std::function<void(S*)> block) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (n_collectors_ == 0 || !slots_) return;

        for (auto slot : *slots_) {
            if (slot) {
                block(slot);
            }
        }
    }
};

} // namespace kotlinx::coroutines::flow::internal

// Include SharedFlow.hpp AFTER AbstractSharedFlow is defined, since SharedFlowImpl inherits from it
#include "kotlinx/coroutines/flow/SharedFlow.hpp"

namespace kotlinx::coroutines::flow::internal {

// NOTE: SubscriptionCountStateFlow is defined at the end of SharedFlow.hpp, not here,
// even though it's in AbstractSharedFlow.kt in Kotlin. This is because in C++, it needs
// the complete definition of SharedFlowImpl<int> to inherit from it, which isn't available
// until after AbstractSharedFlow.hpp is fully processed. This is a necessary structural
// difference to handle C++'s stricter forward-declaration and inheritance requirements.

// Forward declaration - actual definition is in SharedFlow.hpp after SharedFlowImpl is complete
class SubscriptionCountStateFlow;

// Template method implementations


} // namespace kotlinx::coroutines::flow::internal
