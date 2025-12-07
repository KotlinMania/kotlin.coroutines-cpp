#pragma once
#include <atomic>
#include <functional>
#include "kotlinx/coroutines/core_fwd.hpp"

namespace kotlinx {
namespace coroutines {
namespace internal {

/**
 * Doubly-linked list for concurrent access.
 * This is a stub implementation for the C++ port structure.
 */
class LockFreeLinkedListNode {
public:
    LockFreeLinkedListNode() {}
    virtual ~LockFreeLinkedListNode() = default;

    bool is_removed() const;
    LockFreeLinkedListNode* next_node() const;
    LockFreeLinkedListNode* prev_node() const;
    
    bool add_last(LockFreeLinkedListNode* node);
    bool add_one_if_empty(LockFreeLinkedListNode* node);
    virtual bool remove();

    // Permissions bits for close/add
    static const int LIST_ON_COMPLETION_PERMISSION = 1;
    static const int LIST_CANCELLATION_PERMISSION = 2;

    /**
     * Closes the list for anything that requests the permission [forbiddenElementsBit].
     */
    void close(int forbidden_elements_bit);

    // Intrusive list pointers (simulated for now)
    LockFreeLinkedListNode* next = nullptr;
    LockFreeLinkedListNode* prev = nullptr;
};

class LockFreeLinkedListHead : public LockFreeLinkedListNode {
public:
    LockFreeLinkedListHead() : LockFreeLinkedListNode() {}

    template<typename Block>
    void for_each(Block block) {
        LockFreeLinkedListNode* current = next_node();
        while (current != this && current != nullptr) {
            block(current);
            current = current->next_node();
        }
    }

    bool remove() override { return false; } // Cannot be removed
    bool is_empty() const;
};

// Aliases for common core usage if needed, or core components should use internal namespace
// using Node = LockFreeLinkedListNode; 

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
