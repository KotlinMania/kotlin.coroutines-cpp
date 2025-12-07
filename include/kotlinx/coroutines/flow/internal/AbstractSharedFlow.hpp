#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include <vector>
#include <functional>
#include <mutex>

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

template<typename F>
class AbstractSharedFlowSlot {
public:
    virtual ~AbstractSharedFlowSlot() = default;
    virtual bool allocate_locked(F* flow) = 0;
    virtual std::vector<Continuation<void>*> free_locked(F* flow) = 0;
};

template<typename S>
class AbstractSharedFlow {
protected:
    std::vector<S*>* slots = nullptr;
    int nCollectors = 0;
    int nextIndex = 0;
    // SubscriptionCountStateFlow* _subscriptionCount = nullptr; // Stub

    virtual S* create_slot() = 0;
    virtual std::vector<S*>* create_slot_array(int size) = 0;
    std::mutex mutex_; // TODO: inherit from SynchronizedObject

public:
    virtual ~AbstractSharedFlow() {
        delete slots;
    }

    // StateFlow<int>* subscriptionCount(); // Stub

    // S* allocate_slot(); // Stub implementation in cpp or hpp

    // void free_slot(S* slot); // Stub
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
