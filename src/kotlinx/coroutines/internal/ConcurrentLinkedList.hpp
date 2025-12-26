#pragma once
/**
 * @file ConcurrentLinkedList.hpp
 * @brief Lock-free concurrent linked list infrastructure.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/ConcurrentLinkedList.kt
 *
 * Line-by-line port of Kotlin's segment-based concurrent linked list used by
 * Semaphore, Mutex, and Channel implementations.
 */

#include <atomic>
#include <functional>
#include <cassert>
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/ContinuationState.hpp"

namespace kotlinx {
namespace coroutines {

// Forward declaration for CoroutineContext
class CoroutineContext;

namespace internal {

inline Symbol& CLOSED_SYMBOL() {
    static Symbol instance("CLOSED");
    return instance;
}

// Forward declarations
template <typename S> class Segment;
template <typename S> struct SegmentOrClosed;

/**
 * Non-templated base class for Segment<S>.
 * Used by Waiter interface to avoid circular template dependencies.
 * Corresponds to Kotlin's Segment<*> (star projection).
 *
 * Inherits from NotCompleted because in Kotlin, Segment<*> can be stored
 * directly as state in CancellableContinuationImpl (alongside CancelHandler).
 * See CancellableContinuationImpl.kt line 205, 210, 407.
 */
class SegmentBase : public NotCompleted {
public:
    virtual ~SegmentBase() = default;

    /**
     * Called when a slot is logically removed/cancelled.
     * Kotlin line 237: fun onCancellation(index: Int, cause: Throwable?, context: CoroutineContext)
     * Subclasses implement the actual cleanup logic.
     */
    virtual void on_cancellation(int index, std::exception_ptr cause,
        std::shared_ptr<CoroutineContext> context) = 0;

    std::string to_string() const override { return "Segment"; }
};

/**
 * Line 90-182: ConcurrentLinkedListNode
 *
 * Base class for nodes in a concurrent linked list.
 * Pointer to the next node updates similarly to the Michael-Scott queue algorithm.
 */
template <typename N>
class ConcurrentLinkedListNode {
public:
    //             private val _prev = atomic(prev)
    std::atomic<void*> _next{nullptr};
    std::atomic<N*> _prev;

    explicit ConcurrentLinkedListNode(N* prev) : _prev(prev) {}

    virtual ~ConcurrentLinkedListNode() = default;

    void* next_or_closed() const {
        return _next.load(std::memory_order_acquire);
    }

    /**
     * Line 102-109: Returns the next segment or null if one does not exist,
     * and invokes onClosedAction if this segment is marked as closed.
     */
    template <typename OnClosedAction>
    N* next_or_if_closed(OnClosedAction&& on_closed_action) const {
        void* val = next_or_closed();
        if (val == static_cast<void*>(&CLOSED_SYMBOL())) {
            on_closed_action();
            return nullptr; // Should not reach here if on_closed_action throws/returns
        }
        return static_cast<N*>(val);
    }

    N* next() const {
        return next_or_if_closed([]() { /* return null via outer return */ });
    }

    /**
     * Line 116: Tries to set the next segment if it is not specified
     * and this segment is not marked as closed.
     */
    bool try_set_next(N* value) {
        void* expected = nullptr;
        return _next.compare_exchange_strong(expected, static_cast<void*>(value),
            std::memory_order_release, std::memory_order_relaxed);
    }

    bool is_tail() const {
        return next() == nullptr;
    }

    N* prev() const {
        return _prev.load(std::memory_order_acquire);
    }

    void clean_prev() {
        _prev.store(nullptr, std::memory_order_release);
    }

    bool mark_as_closed() {
        void* expected = nullptr;
        return _next.compare_exchange_strong(expected, static_cast<void*>(&CLOSED_SYMBOL()),
            std::memory_order_release, std::memory_order_relaxed);
    }

    /**
     * Line 142: This property indicates whether the current node is logically removed.
     */
    virtual bool is_removed() const = 0;

    /**
     * Line 148-166: Removes this node physically from this linked list.
     */
    void remove() {
        assert(is_removed() || is_tail()); // The node should be logically removed first
        // The physical tail cannot be removed
        if (is_tail()) return;

        while (true) {
            // Read `next` and `prev` pointers ignoring logically removed nodes
            N* prev_node = alive_segment_left();
            N* next_node = alive_segment_right();

            // Link `next` and `prev`
            N* old_prev = next_node->_prev.load(std::memory_order_acquire);
            if (old_prev != nullptr) {
                next_node->_prev.compare_exchange_strong(old_prev, prev_node,
                    std::memory_order_release, std::memory_order_relaxed);
            }
            if (prev_node != nullptr) {
                prev_node->_next.store(static_cast<void*>(next_node), std::memory_order_release);
            }

            // Checks that prev and next are still alive
            if (next_node->is_removed() && !next_node->is_tail()) continue;
            if (prev_node != nullptr && prev_node->is_removed()) continue;

            // This node is removed
            return;
        }
    }

private:
    N* alive_segment_left() const {
        N* cur = _prev.load(std::memory_order_acquire);
        while (cur != nullptr && cur->is_removed()) {
            cur = cur->_prev.load(std::memory_order_acquire);
        }
        return cur;
    }

    N* alive_segment_right() const {
        assert(!is_tail()); // Should not be invoked on the tail node
        N* cur = next();
        while (cur != nullptr && cur->is_removed()) {
            N* next_node = cur->next();
            if (next_node == nullptr) return cur;
            cur = next_node;
        }
        return cur;
    }

public:
    /**
     * Line 78-88 of ConcurrentLinkedList.kt:
     * Closes this linked list of nodes by forbidding adding new ones,
     * returns the last node in the list.
     */
    N* close() {
        N* cur = static_cast<N*>(this);
        while (true) {
            bool was_closed = false;
            N* next_node = cur->next_or_if_closed([&was_closed]() { was_closed = true; });
            if (was_closed) return cur;
            if (next_node == nullptr) {
                if (cur->mark_as_closed()) return cur;
            } else {
                cur = next_node;
            }
        }
    }
};

static constexpr int POINTERS_SHIFT = 16;

/**
 * Line 192-245: Segment<S>
 *
 * Each segment in the list has a unique id and is created by the provided
 * createNewSegment function. Essentially, this is a node in the Michael-Scott
 * queue algorithm, but with maintaining prev pointer for efficient remove.
 */
template <typename S>
class Segment : public ConcurrentLinkedListNode<S>, public SegmentBase {
public:
    const long id;

    std::atomic<int> cleaned_and_pointers;

    Segment(long id, S* prev, int pointers)
        : ConcurrentLinkedListNode<S>(prev)
        , id(id)
        , cleaned_and_pointers(pointers << POINTERS_SHIFT)
    {}

    /**
     * Line 207: This property should return the number of slots in this segment.
     */
    virtual int number_of_slots() const = 0;

    /**
     * Line 218: The segment is considered as removed if all the slots are cleaned
     * and there are no pointers to this segment from outside.
     */
    bool is_removed() const override {
        return cleaned_and_pointers.load(std::memory_order_acquire) == number_of_slots()
            && !this->is_tail();
    }

    /**
     * Line 221: Increments the number of pointers if this segment is not logically removed.
     */
    bool try_inc_pointers() {
        while (true) {
            int cur = cleaned_and_pointers.load(std::memory_order_acquire);
            if (cur == number_of_slots() && !this->is_tail()) return false;
            if (cleaned_and_pointers.compare_exchange_weak(cur, cur + (1 << POINTERS_SHIFT),
                    std::memory_order_release, std::memory_order_relaxed)) {
                return true;
            }
        }
    }

    /**
     * Line 224: Returns true if this segment is logically removed after the decrement.
     */
    bool dec_pointers() {
        int result = cleaned_and_pointers.fetch_sub(1 << POINTERS_SHIFT, std::memory_order_acq_rel);
        result -= (1 << POINTERS_SHIFT);
        return result == number_of_slots() && !this->is_tail();
    }

    /**
     * Line 237: This function is invoked on continuation cancellation.
     */
    void on_cancellation(int index, std::exception_ptr cause,
        std::shared_ptr<CoroutineContext> context) override = 0;

    /**
     * Line 242-244: Invoked on each slot clean-up.
     */
    void on_slot_cleaned() {
        if (cleaned_and_pointers.fetch_add(1, std::memory_order_acq_rel) + 1 == number_of_slots()) {
            this->remove();
        }
    }
};

/**
 * Line 256-260: SegmentOrClosed<S>
 *
 * Wrapper that holds either a segment or indicates the list is closed.
 */
template <typename S>
struct SegmentOrClosed {
    void* value;

    explicit SegmentOrClosed(void* v) : value(v) {}
    explicit SegmentOrClosed(S* segment) : value(static_cast<void*>(segment)) {}

    bool is_closed() const {
        return value == static_cast<void*>(&CLOSED_SYMBOL());
    }

    S* segment() const {
        assert(!is_closed());
        return static_cast<S*>(value);
    }
};

/**
 * Line 12-35: findSegmentInternal
 *
 * Returns the first segment s with s.id >= id or CLOSED if all segments
 * in this linked list have lower id and the list is closed.
 */
template <typename S, typename CreateNewSegment>
SegmentOrClosed<S> find_segment_internal(
    S* start,
    long id,
    CreateNewSegment&& create_new_segment
) {
    S* cur = start;
    while (cur->id < id || cur->is_removed()) {
        S* next_node = cur->template next_or_if_closed<std::function<void()>>(
            [&]() -> void { /* Will be handled below */ });

        void* next_or_closed = cur->next_or_closed();
        if (next_or_closed == static_cast<void*>(&CLOSED_SYMBOL())) {
            return SegmentOrClosed<S>(&CLOSED_SYMBOL());
        }

        next_node = static_cast<S*>(next_or_closed);
        if (next_node != nullptr) {
            cur = next_node;
            continue;
        }

        S* new_tail = create_new_segment(cur->id + 1, cur);
        if (cur->try_set_next(new_tail)) {
            if (cur->is_removed()) cur->remove();
            cur = new_tail;
        } else {
            delete new_tail; // Failed to add, someone else added
            // Re-read next
            next_node = cur->next();
            if (next_node != nullptr) {
                cur = next_node;
            }
        }
    }
    return SegmentOrClosed<S>(cur);
}

/**
 * Line 41-49: moveForward
 *
 * Returns false if the segment `to` is logically removed, true on successful update.
 */
template <typename S>
bool move_forward(std::atomic<S*>& ref, S* to) {
    while (true) {
        S* cur = ref.load(std::memory_order_acquire);
        if (cur->id >= to->id) return true;
        if (!to->try_inc_pointers()) return false;
        if (ref.compare_exchange_strong(cur, to,
                std::memory_order_release, std::memory_order_relaxed)) {
            if (cur->dec_pointers()) cur->remove();
            return true;
        }
        if (to->dec_pointers()) to->remove(); // undo try_inc_pointers
    }
}

/**
 * Line 63-72: findSegmentAndMoveForward
 *
 * Tries to find a segment with the specified id following next references
 * from startFrom segment and creating new ones if needed.
 */
template <typename S, typename CreateNewSegment>
SegmentOrClosed<S> find_segment_and_move_forward(
    std::atomic<S*>& ref,
    long id,
    S* start_from,
    CreateNewSegment&& create_new_segment
) {
    while (true) {
        SegmentOrClosed<S> s = find_segment_internal(start_from, id, create_new_segment);
        if (s.is_closed() || move_forward(ref, s.segment())) {
            return s;
        }
    }
}

/**
 * Line 78-88: close
 *
 * Closes this linked list of nodes by forbidding adding new ones,
 * returns the last node in the list.
 */
template <typename N>
N* close_list(N* start) {
    N* cur = start;
    while (true) {
        N* next_node = cur->template next_or_if_closed<std::function<void()>>(
            [&]() -> void { /* return cur */ });
        void* next_or_closed = cur->next_or_closed();
        if (next_or_closed == static_cast<void*>(&CLOSED_SYMBOL())) {
            return cur;
        }
        next_node = static_cast<N*>(next_or_closed);
        if (next_node == nullptr) {
            if (cur->mark_as_closed()) return cur;
        } else {
            cur = next_node;
        }
    }
}

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
