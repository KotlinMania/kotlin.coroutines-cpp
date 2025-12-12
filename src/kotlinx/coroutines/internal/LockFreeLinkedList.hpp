#pragma once
#include <atomic>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace internal {

/**
 * Doubly-linked list for concurrent access.
 * This is a stub implementation for the C++ port structure.
 */
class LockFreeLinkedListNode {
public:
    LockFreeLinkedListNode();
    virtual ~LockFreeLinkedListNode() = default;

    std::atomic<LockFreeLinkedListNode*> next;
    std::atomic<LockFreeLinkedListNode*> prev;
    std::atomic<bool> removed;

    bool is_removed() const;
    LockFreeLinkedListNode* next_node() const;
    LockFreeLinkedListNode* prev_node() const;
    
    // Core operations
    bool add_last(LockFreeLinkedListNode* node);
    bool add_one_if_empty(LockFreeLinkedListNode* node);
    virtual bool remove();
    
    // Permissions bits for close/add
    static const int LIST_ON_COMPLETION_PERMISSION = 1;
    static const int LIST_CANCELLATION_PERMISSION = 2;

    void close(int forbidden_elements_bit);
    
    // Helper accessors
    void help_remove(); // helping logic
    void remove_help_needed(LockFreeLinkedListNode* node);
};

class LockFreeLinkedListHead : public LockFreeLinkedListNode {
public:
    LockFreeLinkedListHead();

    template<typename Block>
    void for_each(Block block) {
        LockFreeLinkedListNode* current = next_node();
        while (current != this && current != nullptr) {
            block(current);
            current = current->next_node();
        }
    }
    
    bool remove() override;
    bool is_empty() const;
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
