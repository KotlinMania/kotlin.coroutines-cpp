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
    template<typename T> struct StateFlow;
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
    // Vector of unique_ptr owns the slots; slots are reused (not deleted) when freed
    std::unique_ptr<std::vector<std::unique_ptr<S>>> slots_;

    // Kotlin: protected var nCollectors = 0 (with private set)
    int n_collectors_ = 0;

    // Kotlin: private var nextIndex = 0
    int next_index_ = 0;

    // Kotlin: private var _subscriptionCount: SubscriptionCountStateFlow? = null
    std::unique_ptr<SubscriptionCountStateFlow> subscription_count_;

    // Mutex for synchronization (replaces Kotlin's SynchronizedObject)
    mutable std::recursive_mutex mutex_;

protected:
    // Protected getters for slots and nCollectors (matching Kotlin's protected var with private set)
    // Returns raw pointer to vector for read access; vector owns the slots
    std::vector<std::unique_ptr<S>>* get_slots() const { return slots_.get(); }
    int get_n_collectors() const { return n_collectors_; }

    // Expose the shared lock to derived classes to mirror Kotlin synchronized(this).
    std::recursive_mutex& mutex() const { return mutex_; }

    /**
     * Creates a new slot instance.
     * Kotlin: protected abstract fun createSlot(): S
     * Returns unique_ptr - ownership transfers to the slots_ vector.
     */
    virtual std::unique_ptr<S> create_slot() = 0;

    /**
     * Creates a new slot array of the given size (all slots initially null).
     * Kotlin: protected abstract fun createSlotArray(size: Int): Array<S?>
     * Returns unique_ptr to vector of unique_ptr - full ownership transfer.
     */
    virtual std::unique_ptr<std::vector<std::unique_ptr<S>>> create_slot_array(int size) = 0;

    // Derived class must implement this to return itself as F* (CRTP pattern)
    virtual F* as_flow() = 0;

public:
    virtual ~AbstractSharedFlow() {
        // All members are unique_ptr - automatic cleanup
        // Lock not needed during destruction (no concurrent access expected)
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
     * Iterates over all allocated slots.
     * ASSUMES: Caller already holds mutex_ lock (matches Kotlin's inline semantics).
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
        // No lock acquisition - caller must hold mutex_ (per Kotlin inline semantics)
        if (n_collectors_ == 0 || !slots_) return;

        for (const auto& slot : *slots_) {
            if (slot) {
                block(slot.get());
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
