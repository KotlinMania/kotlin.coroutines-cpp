/**
 * @file LockFreeLinkedList.common.cpp
 * @brief Implementation of LockFreeLinkedList.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/internal/LockFreeLinkedList.hpp`.
 */

#include "kotlinx/coroutines/internal/LockFreeLinkedList.hpp"

namespace kotlinx {
    namespace coroutines {
        namespace internal {
            LockFreeLinkedListNode::LockFreeLinkedListNode() : next(this), prev(this), removed(false) {
            }

            bool LockFreeLinkedListNode::is_removed() const {
                return removed.load(std::memory_order_acquire);
            }

            LockFreeLinkedListNode *LockFreeLinkedListNode::next_node() const {
                return next.load(std::memory_order_acquire);
            }

            LockFreeLinkedListNode *LockFreeLinkedListNode::prev_node() const {
                return prev.load(std::memory_order_acquire);
            }

            bool LockFreeLinkedListNode::add_last(LockFreeLinkedListNode *node) {
                while (true) {
                    LockFreeLinkedListNode * prev_ptr = prev.load(std::memory_order_acquire);
                    // Naive CAS for illustration of intent:
                    // In real impl, we'd check prev->next == this etc.
                    // For this port stage:
                    if (auto expected = this; prev_ptr->next.compare_exchange_strong(expected, node)) {
                        node->prev.store(prev_ptr, std::memory_order_release);
                        node->next.store(this, std::memory_order_release);
                        prev.store(node, std::memory_order_release);
                        return true;
                    }
                    // In highly concurrent env, backing off or strictly adhering to LF/WaitFree algo (M. Michael) is required
                    // yield();
                }
            }

            bool LockFreeLinkedListNode::add_one_if_empty(LockFreeLinkedListNode *node) {
                LockFreeLinkedListNode * n = next.load(std::memory_order_acquire);
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

            bool LockFreeLinkedListNode::remove() {
                if (is_removed()) return false;
                LockFreeLinkedListNode * p = prev.load();
                LockFreeLinkedListNode * n = next.load();
                // Simplified removal logic
                p->next.store(n);
                n->prev.store(p);
                removed.store(true, std::memory_order_release);
                return true;
            }

            void LockFreeLinkedListNode::close(int forbidden_elements_bit) {
                // Implementation deferred
            }

            void LockFreeLinkedListNode::help_remove() {
            }

            void LockFreeLinkedListNode::remove_help_needed(LockFreeLinkedListNode *node) {
            }

            LockFreeLinkedListHead::LockFreeLinkedListHead() : LockFreeLinkedListNode() {
            }

            bool LockFreeLinkedListHead::is_empty() const {
                return next_node() == this;
            }

            bool LockFreeLinkedListHead::remove() { return false; }
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx