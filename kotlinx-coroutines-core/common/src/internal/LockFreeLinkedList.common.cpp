#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/LockFreeLinkedList.common.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: expect open class needs platform-specific implementation
// TODO: inline functions need proper C++ implementation
// TODO: Lambda parameters need std::function or templates

#include <functional>

namespace kotlinx {
namespace coroutines {
namespace {

/**
 * This is unstable API and it is subject to change.
 */
// TODO: expect open class - needs platform-specific implementation
class LockFreeLinkedListNode {
public:
    LockFreeLinkedListNode() {}

    bool is_removed() const;
    LockFreeLinkedListNode* next_node() const;
    LockFreeLinkedListNode* prev_node() const;
    bool add_last(LockFreeLinkedListNode* node, int permissions_bitmask);
    bool add_one_if_empty(LockFreeLinkedListNode* node);
    virtual bool remove();

    /**
     * Closes the list for anything that requests the permission [forbiddenElementsBit].
     * Only a single permission can be forbidden at a time, but this isn't checked.
     */
    void close(int forbidden_elements_bit);
};

/**
 * This is unstable API and it is subject to change.
 */
// TODO: expect open class - needs platform-specific implementation
class LockFreeLinkedListHead : LockFreeLinkedListNode {
public:
    LockFreeLinkedListHead() : LockFreeLinkedListNode() {}

    template<typename Block>
    void for_each(Block block) {
        // TODO: inline implementation
        LockFreeLinkedListNode* current = next_node();
        while (current != this) {
            block(current);
            current = current->next_node();
        }
    }

    // final override
    bool remove() = delete; // Cannot be removed (Nothing in Kotlin)
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
