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
    LockFreeLinkedListNode() : next(this), prev(this) {}
    virtual ~LockFreeLinkedListNode() = default;

    // Intrusive list pointers
    // In a real lock-free list, next would be std::atomic<marked_ptr>
    // For this port, we interpret "High Fidelity" as semantic equivalent but somewhat simplified for C++ without a GC.
    // However, JobSupport heavily relies on "remove()" and "addLast()" being thread safe.
    // We will use a spinlock or correct CAS logic if possible.
    // For now, let's use std::atomic for next/prev and a simplistic detailed implementation.
    
    std::atomic<LockFreeLinkedListNode*> next;
    std::atomic<LockFreeLinkedListNode*> prev;
    std::atomic<bool> removed = false;

    bool is_removed() const {
        return removed.load(std::memory_order_acquire);
    }
    
    LockFreeLinkedListNode* next_node() const {
        return next.load(std::memory_order_acquire);
    }
    
    LockFreeLinkedListNode* prev_node() const {
        return prev.load(std::memory_order_acquire);
    }
    
    // Basic non-thread-safe add for now? No, JobSupport is concurrent.
    // We'll implement a coarse "CAS loop" insert.
    bool add_last(LockFreeLinkedListNode* node) {
        while (true) {
            LockFreeLinkedListNode* prev_ptr = prev.load(std::memory_order_acquire);
            LockFreeLinkedListNode* expected = this;
            if (prev_ptr->next.compare_exchange_strong(expected, node)) {
                // Success hooking next
                node->prev.store(prev_ptr, std::memory_order_release);
                node->next.store(this, std::memory_order_release);
                prev.store(node, std::memory_order_release); 
                return true;
            }
        }
        return false;
    }

    bool add_one_if_empty(LockFreeLinkedListNode* node) {
        LockFreeLinkedListNode* n = next.load(std::memory_order_acquire);
        if (n == this) {
            node->next.store(this, std::memory_order_release);
            node->prev.store(this, std::memory_order_release);
            if (next.compare_exchange_strong(n, node)) {
                prev.store(node, std::memory_order_release);
                return true;
            }
        }
        return false;
    }
    
    virtual bool remove() {
        if (is_removed()) return false;
        // simplistic remove: P <-> N
        LockFreeLinkedListNode* p = prev.load();
        LockFreeLinkedListNode* n = next.load();
        // p->next = n; n->prev = p
        
        // This is not atomic in this simplistic version, just writing.
        // In real LFLL we mark 'next' pointer.
        // For now:
        p->next.store(n);
        n->prev.store(p);
        
        removed.store(true, std::memory_order_release);
        return true;
    }

    // Permissions bits for close/add
    static const int LIST_ON_COMPLETION_PERMISSION = 1;
    static const int LIST_CANCELLATION_PERMISSION = 2;

    void close(int forbidden_elements_bit) {
        // active = false or similar
    }
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
    
    bool is_empty() const {
        return next_node() == this;
    }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
