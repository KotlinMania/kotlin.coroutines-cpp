#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/LockFreeLinkedList.common.kt
 * and kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt
 */
#include <memory>
#include <string>

namespace kotlinx::coroutines::internal {

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:29-258
class LockFreeLinkedListNode {
public:
    LockFreeLinkedListNode();
    virtual ~LockFreeLinkedListNode();

    virtual bool is_removed() const;
    void* get_next() const;
    LockFreeLinkedListNode* next_node() const;
    LockFreeLinkedListNode* prev_node() const;

    bool add_last(LockFreeLinkedListNode* node, int permissions_bitmask);
    // NOTE(port): Keep the existing completion-list call form for source compatibility.
    bool add_last(LockFreeLinkedListNode* node) { return add_last(node, LIST_ON_COMPLETION_PERMISSION); }
    bool add_one_if_empty(LockFreeLinkedListNode* node);
    bool add_next(LockFreeLinkedListNode* node, LockFreeLinkedListNode* next);
    virtual bool remove();
    LockFreeLinkedListNode* remove_or_next();
    void close(int forbidden_elements_bit);
    void validate_node(LockFreeLinkedListNode* previous, LockFreeLinkedListNode* next) const;
    virtual std::string to_string() const;

    static constexpr int LIST_ON_COMPLETION_PERMISSION = 1;
    static constexpr int LIST_CHILD_PERMISSION = 2;
    static constexpr int LIST_CANCELLATION_PERMISSION = 4;

    // Compatibility entry points for the existing C++ list surface.
    void help_remove();
    void remove_help_needed(LockFreeLinkedListNode* node);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    LockFreeLinkedListNode* removed_ref();
    LockFreeLinkedListNode* correct_prev() const;
    void finish_add(LockFreeLinkedListNode* next);
};

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:270-285
class LockFreeLinkedListHead : public LockFreeLinkedListNode {
public:
    LockFreeLinkedListHead();

    // Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:274-280
    template<typename Block>
    void for_each(Block block) {
        LockFreeLinkedListNode* current = next_node();
        while (current != this) {
            block(current);
            current = current->next_node();
        }
    }

    bool remove() override;
    bool is_removed() const override;
    bool is_empty() const;
};

} // namespace kotlinx::coroutines::internal
