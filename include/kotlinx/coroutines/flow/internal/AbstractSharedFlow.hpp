#pragma once
#include <vector>
#include <functional>
#include <mutex>
#include <algorithm>
#include <memory>
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/Continuation.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

// Interface for slots used in AbstractSharedFlow
template<typename F>
class AbstractSharedFlowSlot {
public:
    virtual ~AbstractSharedFlowSlot() = default;
    
    // Returns true if allocation was successful (slot was free)
    virtual bool allocate_locked(F* flow) = 0;
    
    // Returns list of continuations to resume (e.g. suspended emitters)
    virtual std::vector<ContinuationBase*> free_locked(F* flow) = 0;
};

template<typename S>
class AbstractSharedFlow {
protected:
    std::vector<S*>* slots_ = nullptr; // array of slots
    int nCollectors_ = 0; // number of allocated slots
    int nextIndex_ = 0; // oracle for next free slot
    std::recursive_mutex mutex_; // Lock for state synchronization

    virtual S* create_slot() = 0;
    virtual std::vector<S*>* create_slot_array(int size) = 0;

public:
    virtual ~AbstractSharedFlow() {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (slots_) {
            for (auto slot : *slots_) {
                if (slot) delete slot;
            }
            delete slots_;
        }
    }

    S* allocate_slot() {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        
        std::vector<S*>* current_slots = slots_;
        if (!current_slots) {
            slots_ = create_slot_array(2);
            current_slots = slots_;
        } else if (nCollectors_ >= static_cast<int>(current_slots->size())) {
            // Expand
            auto new_slots = create_slot_array(2 * current_slots->size());
            std::copy(current_slots->begin(), current_slots->end(), new_slots->begin());
            delete slots_;
            slots_ = new_slots;
            current_slots = slots_;
        }

        int index = nextIndex_;
        S* found_slot = nullptr;
        while (true) {
            if (static_cast<size_t>(index) >= current_slots->size()) index = 0;
            
            found_slot = (*current_slots)[index];
            if (!found_slot) {
                found_slot = create_slot();
                (*current_slots)[index] = found_slot;
            }
            
            if (found_slot->allocate_locked(this)) {
                 break;
            }
            
            index++;
            if (index >= static_cast<int>(current_slots->size())) index = 0;
        }
        
        nextIndex_ = index + 1;
        nCollectors_++;
        return found_slot;
    }

    void free_slot(S* slot) {
        std::vector<ContinuationBase*> resumes;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            nCollectors_--;
            if (nCollectors_ == 0) nextIndex_ = 0;
            
            resumes = slot->free_locked(this);
        }
        
        // Resume outside lock
        for (auto cont : resumes) {
            if (cont) {
                // Resume logic to be implemented with proper context
                // cont->resume_with(...)
            }
        }
    }

    int get_subscription_count() const {
         std::lock_guard<std::recursive_mutex> lock(const_cast<std::recursive_mutex&>(mutex_));
         return nCollectors_;
    }
    
protected:
    void for_each_slot_locked(std::function<void(S*)> block) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (nCollectors_ == 0 || !slots_) return;
        for (auto slot : *slots_) {
            if (slot) block(slot);
        }
    }
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
