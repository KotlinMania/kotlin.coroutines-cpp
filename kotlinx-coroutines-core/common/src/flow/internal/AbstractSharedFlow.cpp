// filepath: /Volumes/stuff/Projects/kotlin.coroutines-cpp/kotlinx-coroutines-core/common/src/flow/internal/AbstractSharedFlow.cpp
#include "kotlinx/coroutines/core_fwd.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

namespace kotlinx {namespace coroutines {namespace flow {namespace internal {

// import kotlinx.coroutines.*
// import kotlinx.coroutines.channels.*
// import kotlinx.coroutines.flow.*
// import kotlinx.coroutines.internal.*
// import kotlin.coroutines.*
// import kotlin.jvm.*

// @JvmField
std::vector<Continuation<Unit>*> EMPTY_RESUMES;

// Forward declarations
template<typename F>
class AbstractSharedFlowSlot {
public:
    virtual ~AbstractSharedFlowSlot() = default;
    virtual bool allocate_locked(F* flow) = 0;
    virtual std::vector<Continuation<Unit>*> free_locked(F* flow) = 0; // returns continuations to resume after lock
};

template<typename S>
class AbstractSharedFlow : public SynchronizedObject {
protected:
    std::vector<S*>* slots = nullptr; // allocated when needed
    int nCollectors = 0; // number of allocated (!free) slots
    int nextIndex = 0; // oracle for the next free slot index
    SubscriptionCountStateFlow* _subscriptionCount = nullptr; // init on first need

    virtual S* create_slot() = 0;
    virtual std::vector<S*>* create_slot_array(int size) = 0;

public:
    virtual ~AbstractSharedFlow() {
        if (slots) {
            delete slots;
        }
    }

    StateFlow<int>* subscriptionCount() {
        return synchronized(this, [&]() -> StateFlow<int>* {
            // allocate under lock in sync with nCollectors variable
            if (!_subscriptionCount) {
                _subscriptionCount = new SubscriptionCountStateFlow(nCollectors);
            }
            return _subscriptionCount;
        });
    }

    // @Suppress("UNCHECKED_CAST")
    S* allocate_slot() {
        // Actually create slot under lock
        SubscriptionCountStateFlow* subscriptionCount_local = nullptr;
        S* slot = synchronized(this, [&]() -> S* {
            std::vector<S*>* current_slots = slots;
            if (!current_slots) {
                slots = create_slot_array(2);
                current_slots = slots;
            } else if (nCollectors >= current_slots->size()) {
                auto new_slots = create_slot_array(2 * current_slots->size());
                std::copy(current_slots->begin(), current_slots->end(), new_slots->begin());
                delete slots;
                slots = new_slots;
                current_slots = slots;
            }

            int index = nextIndex;
            S* found_slot = nullptr;
            while (true) {
                found_slot = (*current_slots)[index];
                if (!found_slot) {
                    found_slot = create_slot();
                    (*current_slots)[index] = found_slot;
                }
                index++;
                if (index >= current_slots->size()) index = 0;
                // @Suppress("UNCHECKED_CAST")
                if (reinterpret_cast<AbstractSharedFlowSlot<AbstractSharedFlow>*>(found_slot)->allocate_locked(this)) {
                    break; // break when found and allocated free slot
                }
            }
            nextIndex = index;
            nCollectors++;
            subscriptionCount_local = _subscriptionCount; // retrieve under lock if initialized
            return found_slot;
        });

        // increments subscription count
        if (subscriptionCount_local) {
            subscriptionCount_local->increment(1);
        }
        return slot;
    }

    // @Suppress("UNCHECKED_CAST")
    void free_slot(S* slot) {
        // Release slot under lock
        SubscriptionCountStateFlow* subscriptionCount_local = nullptr;
        auto resumes = synchronized(this, [&]() -> std::vector<Continuation<Unit>*> {
            nCollectors--;
            subscriptionCount_local = _subscriptionCount;
            // retrieve under lock if initialized
            // Reset next index oracle if we have no more active collectors for more predictable behavior next time
            if (nCollectors == 0) nextIndex = 0;
            return reinterpret_cast<AbstractSharedFlowSlot<AbstractSharedFlow>*>(slot)->free_locked(this);
        });

        /*
         * Resume suspended coroutines.
         * This can happen when the subscriber that was freed was a slow one and was holding up buffer.
         * When this subscriber was freed, previously queued emitted can now wake up and are resumed here.
         */
        for (auto cont : resumes) {
            if (cont) {
                cont->resume(Unit);
            }
        }
        // decrement subscription count
        if (subscriptionCount_local) {
            subscriptionCount_local->increment(-1);
        }
    }

protected:
    void for_each_slot_locked(std::function<void(S*)> block) {
        if (nCollectors == 0) return;
        if (slots) {
            for (auto slot : *slots) {
                if (slot != nullptr) {
                    block(slot);
                }
            }
        }
    }
};

/**
 * [StateFlow] that represents the number of subscriptions.
 *
 * It is exposed as a regular [StateFlow] in our API, but it is implemented as [SharedFlow] undercover to
 * avoid conflations of consecutive updates because the subscription count is very sensitive to it.
 *
 * The importance of non-conflating can be demonstrated with the following example:
 * ```
 * auto shared = flowOf(239).stateIn(this, SharingStarted.Lazily, 42) // stateIn for the sake of the initial value
 * println(shared.first())
 * yield()
 * println(shared.first())
 * ```
 * If the flow is shared within the same dispatcher (e.g. Main) or with a slow/throttled one,
 * the `SharingStarted.Lazily` will never be able to start the source: `first` sees the initial value and immediately
 * unsubscribes, leaving the asynchronous `SharingStarted` with conflated zero.
 *
 * To avoid that (especially in a more complex scenarios), we do not conflate subscription updates.
 */
// @OptIn(ExperimentalForInheritanceCoroutinesApi::class)
class SubscriptionCountStateFlow : public StateFlow<int>, public SharedFlowImpl<int> {
public:
    SubscriptionCountStateFlow(int initialValue)
        : SharedFlowImpl<int>(1, INT_MAX, BufferOverflow::DROP_OLDEST) {
        tryEmit(initialValue);
    }

    int value() override {
        return synchronized(this, [&]() -> int {
            return lastReplayedLocked();
        });
    }

    void increment(int delta) {
        synchronized(this, [&]() {
            tryEmit(lastReplayedLocked() + delta);
        });
    }
};

}}}} // namespace kotlinx::coroutines::flow::internal

