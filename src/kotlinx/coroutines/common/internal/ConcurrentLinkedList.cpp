/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/ConcurrentLinkedList.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Lock-free linked-list segments backing the coroutine wait queues. Kotlin's `atomicfu`
 * is translated to `std::atomic` with explicit memory orderings, and `@JvmField` /
 * `@JvmInline` annotations have no C++ analogue — value-class wrappers become plain
 * structs. The CLOSED sentinel is a unique `Symbol*` allocated once; `nextOrIfClosed`
 * dispatches to either the live next node or the closure-supplied fallback.
 */

#include <atomic>
#include <functional>
#include <cassert>

namespace kotlinx {
    namespace coroutines {
        namespace internal {
            // Forward declarations
            template<typename S>
            class Segment;
            template<typename S>
            class SegmentOrClosed;
            class Symbol;

            /**
 * Returns the first segment `s` with `s.id >= id` or `CLOSED`
 * if all the segments in this linked list have lower `id`, and the list is closed for further segment additions.
 */
            template<typename S>
            SegmentOrClosed<S> find_segment_internal(
                S *self,
                long id,
                std::function<S*(long, S *)> create_new_segment
            ) {
                /*
       Go through `next` references and add new segments if needed, similarly to the `push` in the Michael-Scott
       queue algorithm. The only difference is that "CAS failure" means that the required segment has already been
       added, so the algorithm just uses it. This way, only one segment with each id can be added.
     */
                S *cur = self;
                while (cur->id < id || cur->is_removed()) {
                    // next_or_if_closed returns the next live segment or, if this node is
                    // CLOSED, the closure's return value. Here we observe the CLOSED state
                    // by returning the SegmentOrClosed sentinel through SegmentOrClosed<S>.
                    S *next = cur->next_or_if_closed([&]() -> S * {
                        return SegmentOrClosed<S>::closed_sentinel();
                    });
                    if (next != nullptr) {
                        // there is a next node -- move there
                        cur = next;
                        continue;
                    }
                    S *new_tail = create_new_segment(cur->id + 1, cur);
                    if (cur->try_set_next(new_tail)) {
                        // successfully added new node -- move there
                        if (cur->is_removed()) cur->remove();
                        cur = new_tail;
                    }
                }
                return SegmentOrClosed<S>(cur);
            }

            /**
 * Returns `false` if the segment `to` is logically removed, `true` on a successful update.
 */
            // Upstream:
            //   internal inline fun <S : Segment<S>> AtomicRef<S>.moveForward(to: S): Boolean
            // The Kotlin extension on AtomicRef becomes a free function template over
            // std::atomic<S*>. The relaxed load is acceptable because the surrounding
            // compare_exchange_strong synchronizes the visible-state transition.
            template<typename S>
            bool move_forward(std::atomic<S *> &atomic_ref, S *to) {
                while (true) {
                    S *cur = atomic_ref.load();
                    if (cur->id >= to->id) return true;
                    if (!to->try_inc_pointers()) return false;
                    if (atomic_ref.compare_exchange_strong(cur, to)) {
                        // the segment is moved
                        if (cur->dec_pointers()) cur->remove();
                        return true;
                    }
                    if (to->dec_pointers()) to->remove(); // undo tryIncPointers
                }
            }

            /**
 * Tries to find a segment with the specified [id] following by next references from the
 * [startFrom] segment and creating new ones if needed. The typical use-case is reading this `AtomicRef` values,
 * doing some synchronization, and invoking this function to find the required segment and update the pointer.
 * At the same time, [Segment.cleanPrev] should also be invoked if the previous segments are no longer needed
 * (e.g., queues should use it in dequeue operations).
 *
 * Since segments can be removed from the list, or it can be closed for further segment additions.
 * Returns the segment `s` with `s.id >= id` or `CLOSED` if all the segments in this linked list have lower `id`,
 * and the list is closed.
 */
            template<typename S>
            SegmentOrClosed<S> find_segment_and_move_forward(
                std::atomic<S *> &atomic_ref,
                long id,
                S *start_from,
                std::function<S*(long, S *)> create_new_segment
            ) {
                while (true) {
                    SegmentOrClosed<S> s = find_segment_internal(start_from, id, create_new_segment);
                    if (s.is_closed() || move_forward(atomic_ref, s.segment())) return s;
                }
            }

            /**
 * Closes this linked list of nodes by forbidding adding new ones,
 * returns the last node in the list.
 */
            template<typename N>
            N *close(N *self) {
                N *cur = self;
                while (true) {
                    // Upstream: nextOrClosed(action = { return this }) — on a CLOSED node,
                    // the action returns the same node so the loop terminates.
                    N *next = cur->next_or_if_closed([&]() -> N * { return cur; });
                    if (next == nullptr) {
                        if (cur->mark_as_closed()) return cur;
                    } else {
                        cur = next;
                    }
                }
            }

            template<typename N>
            class ConcurrentLinkedListNode {
            protected:
                // Pointer to the next node, updates similarly to the Michael-Scott queue algorithm.
                std::atomic<void *> _next;
                // Pointer to the previous node, updates in [remove] function.
                std::atomic<N *> _prev;

                void *next_or_closed() const { return _next.load(); }

            public:
                explicit ConcurrentLinkedListNode(N *prev) : _next(nullptr), _prev(prev) {
                }

                /**
     * Returns the next segment or `nullptr` of the one does not exist,
     * and invokes [onClosedAction] if this segment is marked as closed.
     */
                template<typename OnClosedAction>
                N *next_or_if_closed(OnClosedAction on_closed_action) {
                    void *it = next_or_closed();
                    // CLOSED is the unique sentinel produced by close() via
                    // mark_as_closed. It's defined at the bottom of this translation
                    // unit; an `extern` declaration here keeps the lookup unambiguous.
                    extern Symbol *CLOSED;
                    if (it == CLOSED) {
                        return on_closed_action();
                    } else {
                        return static_cast<N *>(it);
                    }
                }

                N *next() {
                    return next_or_if_closed([]() -> N * { return nullptr; });
                }

                /**
     * Tries to set the next segment if it is not specified and this segment is not marked as closed.
     */
                bool try_set_next(N *value) {
                    void *expected = nullptr;
                    return _next.compare_exchange_strong(expected, value);
                }

                /**
     * Checks whether this node is the physical tail of the current linked list.
     */
                bool is_tail() const { return next() == nullptr; }

                N *prev() const { return _prev.load(); }

                /**
     * Cleans the pointer to the previous node.
     */
                void clean_prev() { _prev.store(nullptr, std::memory_order_relaxed); }

                /**
     * Tries to mark the linked list as closed by forbidding adding new nodes after this one.
     */
                bool mark_as_closed() {
                    // CLOSED is the unique sentinel defined at the bottom of this
                    // translation unit. CAS-only on a nullptr `_next` slot installs it.
                    extern Symbol *CLOSED;
                    void *expected = nullptr;
                    return _next.compare_exchange_strong(expected, CLOSED);
                }

                /**
     * This property indicates whether the current node is logically removed.
     * The expected use-case is removing the node logically (so that [isRemoved] becomes true),
     * and invoking [remove] after that. Note that this implementation relies on the contract
     * that the physical tail cannot be logically removed. Please, do not break this contract;
     * otherwise, memory leaks and unexpected behavior can occur.
     */
                virtual bool is_removed() const = 0;

                /**
     * Removes this node physically from this linked list. The node should be
     * logically removed (so [isRemoved] returns `true`) at the point of invocation.
     */
                void remove() {
                    assert(is_removed() || is_tail()); // The node should be logically removed at first.
                    // The physical tail cannot be removed. Instead, we remove it when
                    // a new segment is added and this segment is not the tail one anymore.
                    if (is_tail()) return;
                    while (true) {
                        // Read `next` and `prev` pointers ignoring logically removed nodes.
                        N *prev = alive_segment_left();
                        N *next = alive_segment_right();
                        // Link `next` and `prev`.
                        //
                        // Upstream: next._prev.update { if (it == null) null else prev }
                        //
                        // AtomicRef.update is a CAS-loop that observes the current value
                        // and writes the updated value only while the prior observation
                        // still holds.
                        while (true) {
                            N *cur_prev = next->_prev.load();
                            N *new_prev = (cur_prev == nullptr) ? nullptr : prev;
                            if (next->_prev.compare_exchange_strong(cur_prev, new_prev)) break;
                        }
                        if (prev != nullptr) prev->_next.store(next);
                        // Checks that prev and next are still alive.
                        if (next->is_removed() && !next->is_tail()) continue;
                        if (prev != nullptr && prev->is_removed()) continue;
                        // This node is removed.
                        return;
                    }
                }

            private:
                N *alive_segment_left() {
                    N *cur = prev();
                    while (cur != nullptr && cur->is_removed())
                        cur = cur->_prev.load();
                    return cur;
                }

                N *alive_segment_right() {
                    assert(!is_tail()); // Should not be invoked on the tail node
                    N *cur = next();
                    assert(cur != nullptr);
                    while (cur->is_removed()) {
                        N *n = cur->next();
                        if (n == nullptr) return cur;
                        cur = n;
                    }
                    return cur;
                }
            };

            /**
 * Each segment in the list has a unique id and is created by the provided to [findSegmentAndMoveForward] method.
 * Essentially, this is a node in the Michael-Scott queue algorithm,
 * but with maintaining [prev] pointer for efficient [remove] implementation.
 *
 * NB: this class cannot be or leak into user's code as type as [CancellableContinuationImpl]
 * instance-check it and uses a separate code-path for that.
 *
 * The `NotCompleted` marker referenced by upstream is just a tag interface; in the C++
 * port that role is filled by the `Segment` base class itself, since the
 * CancellableContinuationImpl instanceof-check is against `Segment<*>` shape.
 */
            template<typename S>
            class Segment : public ConcurrentLinkedListNode<S> {
            public:
                const long id;

                Segment(long id, S *prev, int pointers)
                    : ConcurrentLinkedListNode<S>(prev),
                      id(id),
                      cleaned_and_pointers_(pointers << POINTERS_SHIFT) {
                }

                /**
     * This property should return the number of slots in this segment,
     * it is used to define whether the segment is logically removed.
     */
                virtual int number_of_slots() const = 0;

                /**
     * Numbers of cleaned slots (the lowest bits) and AtomicRef pointers to this segment (the highest bits)
     */
            private:
                std::atomic<int> cleaned_and_pointers_;
                static constexpr int POINTERS_SHIFT = 16;

            public:
                /**
     * The segment is considered as removed if all the slots are cleaned
     * and there are no pointers to this segment from outside.
     */
                bool is_removed() const override {
                    return cleaned_and_pointers_.load() == number_of_slots() && !this->is_tail();
                }

                // increments the number of pointers if this segment is not logically removed.
                bool try_inc_pointers() {
                    return add_conditionally(cleaned_and_pointers_, 1 << POINTERS_SHIFT,
                                             [this](int it) { return it != number_of_slots() || this->is_tail(); });
                }

                // returns `true` if this segment is logically removed after the decrement.
                bool dec_pointers() {
                    int result = cleaned_and_pointers_.fetch_add(-(1 << POINTERS_SHIFT));
                    result -= (1 << POINTERS_SHIFT);
                    return result == number_of_slots() && !this->is_tail();
                }

                /**
     * This function is invoked on continuation cancellation when this segment
     * with the specified [index] are installed as cancellation handler via
     * `SegmentDisposable.disposeOnCancellation(Segment, Int)`.
     *
     * @param index the index under which the sement registered itself in the continuation.
     *        Indicies are opaque and arithmetics or numeric intepretation is not allowed on them,
     *        as they may encode additional metadata.
     * @param cause the cause of the cancellation, with the same semantics as [CancellableContinuation.invokeOnCancellation]
     * @param context the context of the cancellable continuation the segment was registered in
     *
     * Upstream signature uses `Throwable?` and `CoroutineContext`; in the C++ port the
     * subclasses cast `cause` to `std::exception_ptr*` and `context` to
     * `CoroutineContext*` at their override sites where the concrete types are known.
     */
                virtual void on_cancellation(int index, void *cause, void *context) = 0;

                /**
     * Invoked on each slot clean-up; should not be invoked twice for the same slot.
     */
                void on_slot_cleaned() {
                    if (cleaned_and_pointers_.fetch_add(1) + 1 == number_of_slots()) this->remove();
                }

            private:
                template<typename Condition>
                static bool add_conditionally(std::atomic<int> &atomic_int, int delta, Condition condition) {
                    while (true) {
                        int cur = atomic_int.load();
                        if (!condition(cur)) return false;
                        if (atomic_int.compare_exchange_strong(cur, cur + delta)) return true;
                    }
                }
            };

            /**
             * Upstream:
             *   @JvmInline internal value class SegmentOrClosed<S : Segment<S>>(
             *       private val value: Any?)
             *
             * Kotlin's `@JvmInline value class` has no C++ analogue — the wrapper still
             * owns a single pointer so the cost matches upstream. `closed_sentinel()`
             * returns the unique CLOSED node that callers compare via `is_closed()`.
             */
            template<typename S>
            class SegmentOrClosed {
            public:
                explicit SegmentOrClosed(void *value) : value_(value) {}

                bool is_closed() const { return value_ == closed_sentinel(); }

                S *segment() const {
                    if (is_closed()) {
                        // Upstream: error("Does not contain segment")
                        throw std::runtime_error("Does not contain segment");
                    }
                    return static_cast<S *>(value_);
                }

                // Sentinel used by next_or_if_closed callers to signal CLOSED state.
                static S *closed_sentinel() {
                    return reinterpret_cast<S *>(CLOSED);
                }

            private:
                void *value_;
            };

            /**
             * Upstream:
             *   private val CLOSED = Symbol("CLOSED")
             *
             * The CLOSED sentinel is a single shared Symbol instance — pointer-equality
             * against this value identifies a closed segment without allocating.
             */
            Symbol *CLOSED = new Symbol("CLOSED");
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx
