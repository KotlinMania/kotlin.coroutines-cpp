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

// S = Slot type, F = Flow type (the derived class itself)
template<typename S, typename F>
class AbstractSharedFlow {
protected:
    std::vector<S*>* slots_ = nullptr; // array of slots
    int n_collectors_ = 0; // number of allocated slots
    int next_index_ = 0; // oracle for next free slot
    std::recursive_mutex mutex_; // Lock for state synchronization

    virtual S* create_slot() = 0;
    virtual std::vector<S*>* create_slot_array(int size) = 0;

    // Derived class must implement this to return itself as F*
    virtual F* as_flow() = 0;

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
        } else if (n_collectors_ >= static_cast<int>(current_slots->size())) {
            // Expand
            auto new_slots = create_slot_array(2 * current_slots->size());
            std::copy(current_slots->begin(), current_slots->end(), new_slots->begin());
            delete slots_;
            slots_ = new_slots;
            current_slots = slots_;
        }

        int index = next_index_;
        S* found_slot = nullptr;
        while (true) {
            if (static_cast<size_t>(index) >= current_slots->size()) index = 0;
            
            found_slot = (*current_slots)[index];
            if (!found_slot) {
                found_slot = create_slot();
                (*current_slots)[index] = found_slot;
            }
            
            if (found_slot->allocate_locked(as_flow())) {
                 break;
            }
            
            index++;
            if (index >= static_cast<int>(current_slots->size())) index = 0;
        }
        
        next_index_ = index + 1;
        n_collectors_++;
        return found_slot;
    }

    void free_slot(S* slot) {
        std::vector<ContinuationBase*> resumes;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            n_collectors_--;
            if (n_collectors_ == 0) next_index_ = 0;
            
            resumes = slot->free_locked(as_flow());
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
         return n_collectors_;
    }
    
protected:
    void for_each_slot_locked(std::function<void(S*)> block) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (n_collectors_ == 0 || !slots_) return;
        for (auto slot : *slots_) {
            if (slot) block(slot);
        }
    }
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
