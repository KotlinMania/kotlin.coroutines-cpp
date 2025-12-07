// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt
//
// TODO: Implement atomic operations (atomicfu library) - use std::atomic or platform-specific atomics
// TODO: Map @InternalCoroutinesApi annotation to comment
// TODO: Map @Suppress("LeakingThis") to comment
// TODO: Map @JvmField annotation to comment (JVM-specific)
// TODO: Map @PublishedApi internal to appropriate C++ visibility
// TODO: Implement tailrec optimization manually (tail call optimization)
// TODO: Implement 'actual' keyword semantics (expect/actual pattern for multiplatform)
// TODO: Implement inline functions
// TODO: Implement Kotlin's 'is' type checking (use dynamic_cast or type traits)
// TODO: Implement Kotlin's 'as' and 'as?' casting
// TODO: Implement lambda with receiver syntax for loop
// TODO: Implement lazy property initialization (lazySet)
// TODO: Implement assert { } blocks
// TODO: Implement ::classSimpleName and hexAddress extensions
// TODO: Implement 'Nothing' return type (use [[noreturn]] or similar)
// TODO: kotlin.jvm.* imports are JVM-specific, can be ignored

namespace kotlinx {
namespace coroutines {
namespace internal {

// Forward declaration
class LockFreeLinkedListNode;
class Removed;

using Node = LockFreeLinkedListNode;

/**
 * Doubly-linked concurrent list node with remove support.
 * Based on paper
 * ["Lock-Free and Practical Doubly Linked List-Based Deques Using Single-Word Compare-and-Swap"](https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.140.4693&rep=rep1&type=pdf)
 * by Sundell and Tsigas with considerable changes.
 *
 * The core idea of the algorithm is to maintain a doubly-linked list with an ever-present sentinel node (it is
 * never removed) that serves both as a list head and tail and to linearize all operations (both insert and remove) on
 * the update of the next pointer. Removed nodes have their next pointer marked with [Removed] class.
 *
 * Important notes:
 * - There are no operations to add items to left side of the list, only to the end (right side), because we cannot
 *   efficiently linearize them with atomic multi-step head-removal operations. In short,
 *   support for [describeRemoveFirst] operation precludes ability to add items at the beginning.
 * - Previous pointers are not marked for removal. We don't support linearizable backwards traversal.
 * - Remove-helping logic is simplified and consolidated in [correctPrev] method.
 *
 * @suppress **This is unstable API and it is subject to change.**
 */
// @Suppress("LeakingThis")
// @InternalCoroutinesApi
class LockFreeLinkedListNode {
private:
    std::atomic<void*> _next; // Node* | Removed* | OpDescriptor* - stored as void* for polymorphism
    std::atomic<Node*> _prev; // Node to the left (cannot be marked as removed)
    std::atomic<Removed*> _removed_ref; // lazily cached removed ref to this

    Removed* removed() {
        Removed* cached = _removed_ref.load(std::memory_order_relaxed);
        if (cached != nullptr) return cached;
        Removed* new_removed = new Removed(this);
        _removed_ref.store(new_removed, std::memory_order_release); // TODO: lazySet equivalent
        return new_removed;
    }

public:
    LockFreeLinkedListNode()
        : _next(static_cast<void*>(this))
        , _prev(this)
        , _removed_ref(nullptr) {
    }

    virtual bool is_removed() const {
        void* next_val = _next.load(std::memory_order_acquire);
        return dynamic_cast<Removed*>(static_cast<Node*>(next_val)) != nullptr; // TODO: implement proper type check
    }

    // LINEARIZABLE. Returns Node | Removed
    void* next() const {
        return _next.load(std::memory_order_acquire);
    }

    // LINEARIZABLE. Returns next non-removed Node
    Node* next_node() const {
        void* next_val = next();
        Removed* removed_ptr = dynamic_cast<Removed*>(static_cast<Node*>(next_val));
        if (removed_ptr != nullptr) {
            return removed_ptr->ref;
        }
        return static_cast<Node*>(next_val);
    }

    // LINEARIZABLE WHEN THIS NODE IS NOT REMOVED:
    // Returns prev non-removed Node, makes sure prev is correct (prev.next === this)
    // NOTE: if this node is removed, then returns non-removed previous node without applying
    // prev.next correction, which does not provide linearizable backwards iteration, but can be used to
    // resume forward iteration when current node was removed.
    Node* prev_node() {
        Node* corrected = correct_prev();
        if (corrected != nullptr) return corrected;
        return find_prev_non_removed(_prev.load(std::memory_order_acquire));
    }

private:
    // TODO: tailrec optimization
    Node* find_prev_non_removed(Node* current) {
        if (!current->is_removed()) return current;
        return find_prev_non_removed(current->_prev.load(std::memory_order_acquire));
    }

public:
    // ------ addOneIfEmpty ------

    bool add_one_if_empty(Node* node) {
        node->_prev.store(this, std::memory_order_release); // TODO: lazySet equivalent
        node->_next.store(static_cast<void*>(this), std::memory_order_release); // TODO: lazySet equivalent
        while (true) {
            void* next_val = next();
            if (next_val != static_cast<void*>(this)) return false; // this is not an empty list!
            if (_next.compare_exchange_strong(next_val, static_cast<void*>(node),
                                               std::memory_order_release,
                                               std::memory_order_acquire)) {
                // added successfully (linearized add) -- fixup the list
                node->finish_add(this);
                return true;
            }
        }
    }

    // ------ addLastXXX ------

    /**
     * Adds last item to this list. Returns `false` if the list is closed.
     */
    bool add_last(Node* node, int permissions_bitmask) {
        while (true) { // lock-free loop on prev.next
            Node* current_prev = prev_node();
            ListClosed* closed = dynamic_cast<ListClosed*>(current_prev);
            if (closed != nullptr) {
                return (closed->forbidden_elements_bitmask & permissions_bitmask) == 0 &&
                       closed->add_last(node, permissions_bitmask);
            }
            if (current_prev->add_next(node, this)) {
                return true;
            }
            continue;
        }
    }

    /**
     * Forbids adding new items to this list.
     */
    void close(int forbidden_elements_bit) {
        add_last(new ListClosed(forbidden_elements_bit), forbidden_elements_bit);
    }

    /**
     * Given:
     * ```
     *                +-----------------------+
     *          this  |         node          V  next
     *          +---+---+     +---+---+     +---+---+
     *  ... <-- | P | N |     | P | N |     | P | N | --> ....
     *          +---+---+     +---+---+     +---+---+
     *                ^                       |
     *                +-----------------------+
     * ```
     * Produces:
     * ```
     *          this            node             next
     *          +---+---+     +---+---+     +---+---+
     *  ... <-- | P | N | ==> | P | N | --> | P | N | --> ....
     *          +---+---+     +---+---+     +---+---+
     *                ^         |   ^         |
     *                +---------+   +---------+
     * ```
     *  Where `==>` denotes linearization point.
     *  Returns `false` if `next` was not following `this` node.
     */
    // @PublishedApi
    bool add_next(Node* node, Node* next) {
        node->_prev.store(this, std::memory_order_release); // TODO: lazySet equivalent
        node->_next.store(static_cast<void*>(next), std::memory_order_release); // TODO: lazySet equivalent
        void* expected_next = static_cast<void*>(next);
        if (!_next.compare_exchange_strong(expected_next, static_cast<void*>(node),
                                            std::memory_order_release,
                                            std::memory_order_acquire)) {
            return false;
        }
        // added successfully (linearized add) -- fixup the list
        node->finish_add(next);
        return true;
    }

    // ------ removeXXX ------

    /**
     * Removes this node from the list. Returns `true` when removed successfully, or `false` if the node was already
     * removed or if it was not added to any list in the first place.
     *
     * **Note**: Invocation of this operation does not guarantee that remove was actually complete if result was `false`.
     * In particular, invoking [nextNode].[prevNode] might still return this node even though it is "already removed".
     */
    virtual bool remove() {
        return remove_or_next() == nullptr;
    }

    // returns null if removed successfully or next node if this node is already removed
    // @PublishedApi
    Node* remove_or_next() {
        while (true) { // lock-free loop on next
            void* next_val = this->next();
            Removed* removed_ptr = dynamic_cast<Removed*>(static_cast<Node*>(next_val));
            if (removed_ptr != nullptr) {
                return removed_ptr->ref; // was already removed -- don't try to help (original thread will take care)
            }
            if (next_val == static_cast<void*>(this)) {
                return static_cast<Node*>(next_val); // was not even added
            }
            Node* next_node = static_cast<Node*>(next_val);
            Removed* removed = next_node->removed();
            void* expected = next_val;
            if (_next.compare_exchange_strong(expected, static_cast<void*>(removed),
                                               std::memory_order_release,
                                               std::memory_order_acquire)) {
                // was removed successfully (linearized remove) -- fixup the list
                next_node->correct_prev();
                return nullptr;
            }
        }
    }

    // This is Harris's RDCSS (Restricted Double-Compare Single Swap) operation
    // It inserts "op" descriptor of when "op" status is still undecided (rolls back otherwise)

    // ------ other helpers ------

    /**
     * Given:
     * ```
     *
     *          prev            this             next
     *          +---+---+     +---+---+     +---+---+
     *  ... <-- | P | N | --> | P | N | --> | P | N | --> ....
     *          +---+---+     +---+---+     +---+---+
     *              ^ ^         |             |
     *              | +---------+             |
     *              +-------------------------+
     * ```
     * Produces:
     * ```
     *          prev            this             next
     *          +---+---+     +---+---+     +---+---+
     *  ... <-- | P | N | --> | P | N | --> | P | N | --> ....
     *          +---+---+     +---+---+     +---+---+
     *                ^         |   ^         |
     *                +---------+   +---------+
     * ```
     */
private:
    void finish_add(Node* next) {
        // TODO: Implement loop with lambda
        while (true) {
            Node* next_prev = next->_prev.load(std::memory_order_acquire);
            if (this->next() != static_cast<void*>(next)) return; // this or next was removed or another node added, remover/adder fixes up links
            if (next->_prev.compare_exchange_strong(next_prev, this,
                                                     std::memory_order_release,
                                                     std::memory_order_acquire)) {
                // This newly added node could have been removed, and the above CAS would have added it physically again.
                // Let us double-check for this situation and correct if needed
                if (is_removed()) next->correct_prev();
                return;
            }
        }
    }

    /**
     * Returns the corrected value of the previous node while also correcting the `prev` pointer
     * (so that `this.prev.next === this`) and helps complete node removals to the left ot this node.
     *
     * It returns `null` in two special cases:
     *
     * - When this node is removed. In this case there is no need to waste time on corrections, because
     *   remover of this node will ultimately call [correctPrev] on the next node and that will fix all
     *   the links from this node, too.
     */
    // TODO: tailrec optimization
    Node* correct_prev() {
        Node* old_prev = _prev.load(std::memory_order_acquire);
        Node* prev = old_prev;
        Node* last = nullptr; // will be set so that last.next === prev
        while (true) { // move the left until first non-removed node
            void* prev_next_val = prev->_next.load(std::memory_order_acquire);

            // fast path to find quickly find prev node when everything is properly linked
            if (prev_next_val == static_cast<void*>(this)) {
                if (old_prev == prev) return prev; // nothing to update -- all is fine, prev found
                // otherwise need to update prev
                Node* expected = old_prev;
                if (!this->_prev.compare_exchange_strong(expected, prev,
                                                          std::memory_order_release,
                                                          std::memory_order_acquire)) {
                    // Note: retry from scratch on failure to update prev
                    return correct_prev();
                }
                return prev; // return the correct prev
            }

            // slow path when we need to help remove operations
            if (this->is_removed()) return nullptr; // nothing to do, this node was removed, bail out asap to save time

            Removed* removed_ptr = dynamic_cast<Removed*>(static_cast<Node*>(prev_next_val));
            if (removed_ptr != nullptr) {
                if (last != nullptr) {
                    // newly added (prev) node is already removed, correct last.next around it
                    void* expected_last_next = static_cast<void*>(prev);
                    if (!last->_next.compare_exchange_strong(expected_last_next, static_cast<void*>(removed_ptr->ref),
                                                              std::memory_order_release,
                                                              std::memory_order_acquire)) {
                        return correct_prev(); // retry from scratch on failure to update next
                    }
                    prev = last;
                    last = nullptr;
                } else {
                    prev = prev->_prev.load(std::memory_order_acquire);
                }
            } else { // prevNext is a regular node, but not this -- help delete
                last = prev;
                prev = static_cast<Node*>(prev_next_val);
            }
        }
    }

    void validate_node(Node* prev, Node* next) {
        // TODO: implement assert
        // assert { prev === this._prev.value }
        // assert { next === this._next.value }
    }

public:
    virtual std::string to_string() const {
        // TODO: implement ::classSimpleName and hexAddress
        return "LockFreeLinkedListNode@" + std::to_string(reinterpret_cast<uintptr_t>(this));
    }
};

class Removed {
public:
    // @JvmField
    Node* ref;

    Removed(Node* ref_node) : ref(ref_node) {}

    std::string to_string() const {
        return "Removed[" + ref->to_string() + "]";
    }
};

/**
 * Head (sentinel) item of the linked list that is never removed.
 *
 * @suppress **This is unstable API and it is subject to change.**
 */
class LockFreeLinkedListHead : public LockFreeLinkedListNode {
public:
    /**
     * Iterates over all elements in this list of a specified type.
     */
    // TODO: inline function with lambda
    template<typename F>
    void for_each(F block) {
        Node* cur = static_cast<Node*>(next());
        while (cur != this) {
            block(cur);
            cur = cur->next_node();
        }
    }

    // just a defensive programming -- makes sure that list head sentinel is never removed
    // TODO: 'Nothing' return type - use [[noreturn]]
    [[noreturn]] bool remove() override {
        // TODO: error() function
        throw std::logic_error("head cannot be removed");
    }

    // optimization: because head is never removed, we don't have to read _next.value to check these:
    bool is_removed() const override {
        return false;
    }
};

class ListClosed : public LockFreeLinkedListNode {
public:
    // @JvmField
    int forbidden_elements_bitmask;

    ListClosed(int bitmask) : forbidden_elements_bitmask(bitmask) {}
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
