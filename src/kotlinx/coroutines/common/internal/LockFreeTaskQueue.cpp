#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/LockFreeTaskQueue.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: atomicfu library needs C++ atomic equivalent
// TODO: @JvmField, @JvmInline annotations - JVM-specific
// TODO: typealias needs using declaration
// TODO: Lock-free algorithm correctness needs careful review
// TODO: atomicArrayOfNulls needs custom implementation
// TODO: Inline functions and lambdas need proper C++ implementation
// TODO: Extension functions (withState, loop, update) need implementation

#include <atomic>
#include <vector>
#include <functional>

namespace kotlinx {
    namespace coroutines {
        namespace {
            // Forward declarations
            template<typename E>
            class LockFreeTaskQueueCore;
            class Symbol;

            // typealias Core<E> = LockFreeTaskQueueCore<E>
            template<typename E>
            using Core = LockFreeTaskQueueCore<E>;

            /**
 * Lock-free Multiply-Producer xxx-Consumer Queue for task scheduling purposes.
 *
 * **Note 1: This queue is NOT linearizable. It provides only quiescent consistency for its operations.**
 * However, this guarantee is strong enough for task-scheduling purposes.
 * In particular, the following execution is permitted for this queue, but is not permitted for a linearizable queue:
 *
 * ```
 * Thread 1: addLast(1) = true, removeFirstOrNull() = nullptr
 * Thread 2: addLast(2) = 2 // this operation is concurrent with both operations in the first thread
 * ```
 *
 * **Note 2: When this queue is used with multiple consumers (`singleConsumer == false`) this it is NOT lock-free.**
 * In particular, consumer spins until producer finishes its operation in the case of near-empty queue.
 * It is a very short window that could manifest itself rarely and only under specific load conditions,
 * but it still deprives this algorithm of its lock-freedom.
 */
            template<typename E>
            class LockFreeTaskQueue {
            private:
                std::atomic<Core<E> *> _cur;

            public:
                explicit LockFreeTaskQueue(bool single_consumer)
                    : _cur(new Core<E>(Core<E>::kInitialCapacity, single_consumer)) {
                }

                // Note: it is not atomic w.r.t. remove operation (remove can transiently fail when isEmpty is false)
                bool is_empty() const { return _cur.load()->is_empty(); }
                int size() const { return _cur.load()->size(); }

                void close() {
                    while (true) {
                        Core<E> *cur = _cur.load();
                        if (cur->close()) return; // closed this copy
                        _cur.compare_exchange_weak(cur, cur->next()); // move to next
                    }
                }

                bool add_last(E *element) {
                    while (true) {
                        Core<E> *cur = _cur.load();
                        int result = cur->add_last(element);
                        if (result == Core<E>::kAddSuccess) return true;
                        if (result == Core<E>::kAddClosed) return false;
                        if (result == Core<E>::kAddFrozen) {
                            _cur.compare_exchange_weak(cur, cur->next()); // move to next
                        }
                    }
                }

                // TODO: @Suppress("UNCHECKED_CAST")
                E *remove_first_or_null() {
                    while (true) {
                        Core<E> *cur = _cur.load();
                        void *result = cur->remove_first_or_null();
                        if (result != Core<E>::kRemoveFrozen) return static_cast<E *>(result);
                        _cur.compare_exchange_weak(cur, cur->next());
                    }
                }

                // Used for validation in tests only
                template<typename R>
                std::vector<R> map(std::function<R(E *)> transform) {
                    return _cur.load()->map(transform);
                }

                // Used for validation in tests only
                bool is_closed() { return _cur.load()->is_closed(); }
            };

            /**
 * Lock-free Multiply-Producer xxx-Consumer Queue core.
 * @see LockFreeTaskQueue
 */
            template<typename E>
            class LockFreeTaskQueueCore {
            private:
                const int capacity_;
                const bool single_consumer_;
                const int mask_;

                std::atomic<Core<E> *> _next;
                std::atomic<long> _state;
                std::vector<std::atomic<void *> > array_; // TODO: atomicArrayOfNulls equivalent

            public:
                LockFreeTaskQueueCore(int capacity, bool single_consumer)
                    : capacity_(capacity),
                      single_consumer_(single_consumer),
                      mask_(capacity - 1),
                      _next(nullptr),
                      _state(0L),
                      array_(capacity) {
                    // TODO: check(mask <= MAX_CAPACITY_MASK)
                    // TODO: check(capacity and mask == 0)
                }

                // Note: it is not atomic w.r.t. remove operation (remove can transiently fail when isEmpty is false)
                bool is_empty() const {
                    long state = _state.load();
                    int head = static_cast<int>((state & kHeadMask) >> kHeadShift);
                    int tail = static_cast<int>((state & kTailMask) >> kTailShift);
                    return head == tail;
                }

                int size() const {
                    long state = _state.load();
                    int head = static_cast<int>((state & kHeadMask) >> kHeadShift);
                    int tail = static_cast<int>((state & kTailMask) >> kTailShift);
                    return (tail - head) & kMaxCapacityMask;
                }

                bool close() {
                    while (true) {
                        long state = _state.load();
                        if ((state & kClosedMask) != 0L) return true; // ok - already closed
                        if ((state & kFrozenMask) != 0L) return false; // frozen -- try next
                        long new_state = state | kClosedMask;
                        if (_state.compare_exchange_weak(state, new_state)) return true;
                    }
                }

                // ADD_CLOSED | ADD_FROZEN | ADD_SUCCESS
                int add_last(E *element) {
                    while (true) {
                        long state = _state.load();
                        if ((state & (kFrozenMask | kClosedMask)) != 0L) {
                            return add_fail_reason(state); // cannot add
                        }

                        int head = static_cast<int>((state & kHeadMask) >> kHeadShift);
                        int tail = static_cast<int>((state & kTailMask) >> kTailShift);
                        int mask = this->mask_; // manually move instance field to local for performance

                        // If queue is Single-Consumer then there could be one element beyond head that we cannot overwrite,
                        // so we check for full queue with an extra margin of one element
                        if (((tail + 2) & mask) == (head & mask)) return kAddFrozen; // overfull, so do freeze & copy

                        // If queue is Multi-Consumer then the consumer could still have not cleared element
                        // despite the above check for one free slot.
                        if (!single_consumer_ && array_[tail & mask].load() != nullptr) {
                            // There are two options in this situation
                            // 1. Spin-wait until consumer clears the slot
                            // 2. Freeze & resize to avoid spinning
                            // We use heuristic here to avoid memory-overallocation
                            // Freeze & reallocate when queue is small or more than half of the queue is used
                            if (capacity_ < kMinAddSpinCapacity || ((tail - head) & kMaxCapacityMask) > (
                                    capacity_ >> 1)) {
                                return kAddFrozen;
                            }
                            // otherwise spin
                            continue;
                        }

                        int new_tail = (tail + 1) & kMaxCapacityMask;
                        long new_state = update_tail(state, new_tail);
                        if (_state.compare_exchange_weak(state, new_state)) {
                            // successfully added
                            array_[tail & mask].store(element);
                            // could have been frozen & copied before this item was set -- correct it by filling placeholder
                            Core<E> *cur = this;
                            while (true) {
                                if ((cur->_state.load() & kFrozenMask) == 0L) break; // all fine -- not frozen yet
                                cur = cur->next()->fill_placeholder(tail, element);
                                if (cur == nullptr) break;
                            }
                            return kAddSuccess; // added successfully
                        }
                    }
                }

                Core<E> *fill_placeholder(int index, E *element) {
                    void *old = array_[index & mask_].load();
                    /*
         * addLast actions:
         * 1) Commit tail slot
         * 2) Write element to array slot
         * 3) Check for array copy
         *
         * If copy happened between 2 and 3 then the consumer might have consumed our element,
         * then another producer might have written its placeholder in our slot, so we should
         * perform *unique* check that current placeholder is our to avoid overwriting another producer placeholder
         */
                    // TODO: if (old is Placeholder && old.index == index) - type checking needed
                    auto *placeholder = dynamic_cast<Placeholder *>(old);
                    if (placeholder && placeholder->index == index) {
                        array_[index & mask_].store(element);
                        // we've corrected missing element, should check if that propagated to further copies, just in case
                        return this;
                    }
                    // it is Ok, no need for further action
                    return nullptr;
                }

                // REMOVE_FROZEN | nullptr (EMPTY) | E (SUCCESS)
                void *remove_first_or_null() {
                    while (true) {
                        long state = _state.load();
                        if ((state & kFrozenMask) != 0L) return kRemoveFrozen; // frozen -- cannot modify

                        int head = static_cast<int>((state & kHeadMask) >> kHeadShift);
                        int tail = static_cast<int>((state & kTailMask) >> kTailShift);

                        if ((tail & mask_) == (head & mask_)) return nullptr; // empty

                        void *element = array_[head & mask_].load();
                        if (element == nullptr) {
                            // If queue is Single-Consumer, then element == nullptr only when add has not finished yet
                            if (single_consumer_) return nullptr; // consider it not added yet
                            // retry (spin) until consumer adds it
                            continue;
                        }

                        // TODO: element is Placeholder - type checking needed
                        auto *placeholder = dynamic_cast<Placeholder *>(element);
                        if (placeholder) return nullptr; // consider it not added yet

                        // we cannot put nullptr into array here, because copying thread could replace it with Placeholder and that is a disaster
                        int new_head = (head + 1) & kMaxCapacityMask;
                        long new_state = update_head(state, new_head);
                        if (_state.compare_exchange_weak(state, new_state)) {
                            // Array could have been copied by another thread and it is perfectly fine, since only elements
                            // between head and tail were copied and there are no extra steps we should take here
                            array_[head & mask_].store(nullptr); // now can safely put nullptr (state was updated)
                            return element; // successfully removed in fast-path
                        }

                        // Multi-Consumer queue must retry this loop on CAS failure (another consumer might have removed element)
                        if (!single_consumer_) continue;

                        // Single-consumer queue goes to slow-path for remove in case of interference
                        Core<E> *cur = this;
                        while (true) {
                            cur = cur->remove_slow_path(head, new_head);
                            if (cur == nullptr) return element;
                        }
                    }
                }

                Core<E> *remove_slow_path(int old_head, int new_head) {
                    while (true) {
                        long state = _state.load();
                        int head = static_cast<int>((state & kHeadMask) >> kHeadShift);
                        // TODO: assert { head == old_head } // "This queue can have only one consumer"

                        if ((state & kFrozenMask) != 0L) {
                            // state was already frozen, so removed element was copied to next
                            return next(); // continue to correct head in next
                        }

                        long new_state = update_head(state, new_head);
                        if (_state.compare_exchange_weak(state, new_state)) {
                            array_[head & mask_].store(nullptr); // now can safely put nullptr (state was updated)
                            return nullptr;
                        }
                    }
                }

                Core<E> *next() { return allocate_or_get_next_copy(mark_frozen()); }

                long mark_frozen() {
                    while (true) {
                        long state = _state.load();
                        if ((state & kFrozenMask) != 0L) return state; // already marked
                        long new_state = state | kFrozenMask;
                        if (_state.compare_exchange_weak(state, new_state)) return new_state;
                    }
                }

                Core<E> *allocate_or_get_next_copy(long state) {
                    while (true) {
                        Core<E> *next = _next.load();
                        if (next != nullptr) return next; // already allocated & copied
                        Core<E> *new_next = allocate_next_copy(state);
                        _next.compare_exchange_weak(next, new_next);
                    }
                }

                Core<E> *allocate_next_copy(long state) {
                    Core<E> *next = new Core<E>(capacity_ * 2, single_consumer_);
                    int head = static_cast<int>((state & kHeadMask) >> kHeadShift);
                    int tail = static_cast<int>((state & kTailMask) >> kTailShift);

                    int index = head;
                    while ((index & mask_) != (tail & mask_)) {
                        // replace nulls with placeholders on copy
                        void *value = array_[index & mask_].load();
                        if (value == nullptr) value = new Placeholder(index);
                        next->array_[index & next->mask_].store(value);
                        index++;
                    }
                    next->_state.store(state & ~kFrozenMask);
                    return next;
                }

                // Used for validation in tests only
                template<typename R>
                std::vector<R> map(std::function<R(E *)> transform) {
                    std::vector<R> res;
                    long state = _state.load();
                    int head = static_cast<int>((state & kHeadMask) >> kHeadShift);
                    int tail = static_cast<int>((state & kTailMask) >> kTailShift);

                    int index = head;
                    while ((index & mask_) != (tail & mask_)) {
                        void *element = array_[index & mask_].load();
                        // TODO: if (element != nullptr && element !is Placeholder)
                        if (element != nullptr && dynamic_cast<Placeholder *>(element) == nullptr) {
                            res.push_back(transform(static_cast<E *>(element)));
                        }
                        index++;
                    }
                    return res;
                }

                // Used for validation in tests only
                bool is_closed() { return (_state.load() & kClosedMask) != 0L; }

                // Instance of this class is placed into array when we have to copy array, but addLast is in progress --
                // it had already reserved a slot in the array (with nullptr) and have not yet put its value there.
                // Placeholder keeps the actual index (not masked) to distinguish placeholders on different wraparounds of array
                // Internal because of inlining
                class Placeholder {
                public:
                    int index;

                    explicit Placeholder(int index) : index(index) {
                    }
                };

                // TODO: @Suppress("PrivatePropertyName", "MemberVisibilityCanBePrivate")
                // Constants
                static constexpr int kInitialCapacity = 8;
                static constexpr int kCapacityBits = 30;
                static constexpr int kMaxCapacityMask = (1 << kCapacityBits) - 1;
                static constexpr int kHeadShift = 0;
                static constexpr long kHeadMask = static_cast<long>(kMaxCapacityMask) << kHeadShift;
                static constexpr int kTailShift = kHeadShift + kCapacityBits;
                static constexpr long kTailMask = static_cast<long>(kMaxCapacityMask) << kTailShift;
                static constexpr int kFrozenShift = kTailShift + kCapacityBits;
                static constexpr long kFrozenMask = 1L << kFrozenShift;
                static constexpr int kClosedShift = kFrozenShift + 1;
                static constexpr long kClosedMask = 1L << kClosedShift;
                static constexpr int kMinAddSpinCapacity = 1024;

                static Symbol *kRemoveFrozen; // TODO: Symbol("REMOVE_FROZEN")

                static constexpr int kAddSuccess = 0;
                static constexpr int kAddFrozen = 1;
                static constexpr int kAddClosed = 2;

            private:
                static long update_head(long state, int new_head) {
                    return (state & ~kHeadMask) | (static_cast<long>(new_head) << kHeadShift);
                }

                static long update_tail(long state, int new_tail) {
                    return (state & ~kTailMask) | (static_cast<long>(new_tail) << kTailShift);
                }

                static int add_fail_reason(long state) {
                    return ((state & kClosedMask) != 0L) ? kAddClosed : kAddFrozen;
                }
            };
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx