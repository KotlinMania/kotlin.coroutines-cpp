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

bool LockFreeLinkedListNode::is_removed() const {
    return false; // Stub
}

LockFreeLinkedListNode* LockFreeLinkedListNode::next_node() const {
    return next;
}

LockFreeLinkedListNode* LockFreeLinkedListNode::prev_node() const {
    return prev;
}

bool LockFreeLinkedListNode::add_last(LockFreeLinkedListNode* node) {
    // Stub simple linking
    if (this->next == nullptr) {
        this->next = node;
        node->prev = this;
        return true;
    }
    return false;
}

bool LockFreeLinkedListNode::add_one_if_empty(LockFreeLinkedListNode* node) {
    if (this->next == nullptr) {
        add_last(node);
        return true;
    }
    return false;
}

bool LockFreeLinkedListNode::remove() {
    // Stub unlinking
    return true;
}

void LockFreeLinkedListNode::close(int forbidden_elements_bit) {
    // Stub
}

bool LockFreeLinkedListHead::is_empty() const {
    return next == this || next == nullptr;
}

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
