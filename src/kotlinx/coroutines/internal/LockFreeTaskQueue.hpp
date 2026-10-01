#pragma once
// port-lint: source internal/LockFreeTaskQueue.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/LockFreeTaskQueue.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Lock-free single-consumer / multi-producer task queue. The Kotlin source uses
 * atomicfu's `atomicArrayOfNulls<Any?>`; the C++ port uses a vector of std::atomic<void*>
 * with explicit memory orderings on every CAS/load/store. `@JvmField` / `@JvmInline` /
 * `typealias` annotations have no C++ analogue — they translate to plain fields and
 * `using` declarations. The Michael-Scott-style algorithm is preserved verbatim;
 * Placeholder sentinels in the array distinguish "uninitialised" slots from "in-flight".
 */

#include <atomic>
#include <vector>
#include <functional>
#include <cassert>
#include "kotlinx/coroutines/internal/Symbol.hpp"

namespace kotlinx {
    namespace coroutines {
        namespace internal {
            // Forward declarations
            template<typename E>
            class LockFreeTaskQueueCore;

            // typealias Core<E> = LockFreeTaskQueueCore<E>
            template<typename E>
            using Core = LockFreeTaskQueueCore<E>;

            /**
             * Lock-free Multiply-Producer xxx-Consumer Queue for task scheduling purposes.
             * Transliterated from: internal class LockFreeTaskQueue<E : Any>
             */
            template<typename E>
            class LockFreeTaskQueue {
            private:
                std::atomic<Core<E> *> _cur;

            public:
                explicit LockFreeTaskQueue(bool single_consumer)
                    : _cur(new Core<E>(Core<E>::INITIAL_CAPACITY, single_consumer)) {
                }

                ~LockFreeTaskQueue() {
                    delete _cur.load();
                }

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
                        if (result == Core<E>::ADD_SUCCESS) return true;
                        if (result == Core<E>::ADD_CLOSED) return false;
                        if (result == Core<E>::ADD_FROZEN) {
                            _cur.compare_exchange_weak(cur, cur->next()); // move to next
                        }
                    }
                }

                E *remove_first_or_null() {
                    while (true) {
                        Core<E> *cur = _cur.load();
                        void *result = cur->remove_first_or_null();
                        if (result != Core<E>::REMOVE_FROZEN) return static_cast<E *>(result);
                        _cur.compare_exchange_weak(cur, cur->next());
                    }
                }

                template<typename R>
                std::vector<R> map(std::function<R(E *)> transform) {
                    return _cur.load()->map(transform);
                }

                bool is_closed() { return _cur.load()->is_closed(); }
            };

            /**
             * Lock-free Multiply-Producer xxx-Consumer Queue core.
             * Transliterated from: internal class LockFreeTaskQueueCore<E : Any>
             */
            template<typename E>
            class LockFreeTaskQueueCore {
            public:
                class Placeholder {
                public:
                    virtual ~Placeholder() = default;
                    int index;
                    explicit Placeholder(int idx) : index(idx) {}
                };

                static constexpr int INITIAL_CAPACITY = 8;
                static constexpr int CAPACITY_BITS = 30;
                static constexpr int MAX_CAPACITY_MASK = (1 << CAPACITY_BITS) - 1;
                static constexpr int HEAD_SHIFT = 0;
                static constexpr long HEAD_MASK = static_cast<long>(MAX_CAPACITY_MASK) << HEAD_SHIFT;
                static constexpr int TAIL_SHIFT = HEAD_SHIFT + CAPACITY_BITS;
                static constexpr long TAIL_MASK = static_cast<long>(MAX_CAPACITY_MASK) << TAIL_SHIFT;
                static constexpr int FROZEN_SHIFT = TAIL_SHIFT + CAPACITY_BITS;
                static constexpr long FROZEN_MASK = 1L << FROZEN_SHIFT;
                static constexpr int CLOSED_SHIFT = FROZEN_SHIFT + 1;
                static constexpr long CLOSED_MASK = 1L << CLOSED_SHIFT;
                static constexpr int MIN_ADD_SPIN_CAPACITY = 1024;

                static inline Symbol REMOVE_FROZEN_SYMBOL{"REMOVE_FROZEN"};
                static inline void* REMOVE_FROZEN = &REMOVE_FROZEN_SYMBOL;

                static constexpr int ADD_SUCCESS = 0;
                static constexpr int ADD_FROZEN = 1;
                static constexpr int ADD_CLOSED = 2;

            private:
                const int capacity_;
                const bool single_consumer_;
                const int mask_;

                std::atomic<Core<E> *> _next;
                std::atomic<long> _state;
                std::vector<std::atomic<void *> > array_;

                static long update_head(long state, int new_head) {
                    return (state & ~HEAD_MASK) | (static_cast<long>(new_head) << HEAD_SHIFT);
                }

                static long update_tail(long state, int new_tail) {
                    return (state & ~TAIL_MASK) | (static_cast<long>(new_tail) << TAIL_SHIFT);
                }

                static int add_fail_reason(long state) {
                    return ((state & CLOSED_MASK) != 0L) ? ADD_CLOSED : ADD_FROZEN;
                }

            public:
                LockFreeTaskQueueCore(int capacity, bool single_consumer)
                    : capacity_(capacity),
                      single_consumer_(single_consumer),
                      mask_(capacity - 1),
                      _next(nullptr),
                      _state(0L),
                      array_(capacity) {
                    assert(mask_ <= MAX_CAPACITY_MASK);
                    assert((capacity & mask_) == 0);
                    for (int i = 0; i < capacity; ++i) {
                        array_[i].store(nullptr);
                    }
                }

                bool is_empty() const {
                    const long state = _state.load();
                    const int head = static_cast<int>((state & HEAD_MASK) >> HEAD_SHIFT);
                    const int tail = static_cast<int>((state & TAIL_MASK) >> TAIL_SHIFT);
                    return head == tail;
                }

                int size() const {
                    const long state = _state.load();
                    const int head = static_cast<int>((state & HEAD_MASK) >> HEAD_SHIFT);
                    const int tail = static_cast<int>((state & TAIL_MASK) >> TAIL_SHIFT);
                    return (tail - head) & MAX_CAPACITY_MASK;
                }

                bool close() {
                    while (true) {
                        long state = _state.load();
                        if ((state & CLOSED_MASK) != 0L) return true;
                        if ((state & FROZEN_MASK) != 0L) return false;
                        long new_state = state | CLOSED_MASK;
                        if (_state.compare_exchange_weak(state, new_state)) return true;
                    }
                }

                int add_last(E *element) {
                    while (true) {
                        long state = _state.load();
                        if ((state & (FROZEN_MASK | CLOSED_MASK)) != 0L) {
                            return add_fail_reason(state);
                        }

                        const int head = static_cast<int>((state & HEAD_MASK) >> HEAD_SHIFT);
                        const int tail = static_cast<int>((state & TAIL_MASK) >> TAIL_SHIFT);
                        const int mask = this->mask_;

                        if (((tail + 2) & mask) == (head & mask)) return ADD_FROZEN;

                        if (!single_consumer_ && array_[tail & mask].load() != nullptr) {
                            if (capacity_ < MIN_ADD_SPIN_CAPACITY || ((tail - head) & MAX_CAPACITY_MASK) > (capacity_ >> 1)) {
                                return ADD_FROZEN;
                            }
                            continue;
                        }

                        int new_tail = (tail + 1) & MAX_CAPACITY_MASK;
                        long new_state = update_tail(state, new_tail);
                        if (_state.compare_exchange_weak(state, new_state)) {
                            array_[tail & mask].store(element);
                            Core<E> *cur = this;
                            while (true) {
                                if ((cur->_state.load() & FROZEN_MASK) == 0L) break;
                                cur = cur->next()->fill_placeholder(tail, element);
                                if (cur == nullptr) break;
                            }
                            return ADD_SUCCESS;
                        }
                    }
                }

                Core<E> *fill_placeholder(int index, E *element) {
                    void *old = array_[index & mask_].load();
                    auto *placeholder = dynamic_cast<Placeholder *>(static_cast<Placeholder *>(old));
                    if (placeholder && placeholder->index == index) {
                        array_[index & mask_].store(element);
                        return this;
                    }
                    return nullptr;
                }

                void *remove_first_or_null() {
                    while (true) {
                        long state = _state.load();
                        if ((state & FROZEN_MASK) != 0L) return REMOVE_FROZEN;

                        int head = static_cast<int>((state & HEAD_MASK) >> HEAD_SHIFT);
                        int tail = static_cast<int>((state & TAIL_MASK) >> TAIL_SHIFT);

                        if ((tail & mask_) == (head & mask_)) return nullptr;

                        void *element = array_[head & mask_].load();
                        if (element == nullptr) {
                            if (single_consumer_) return nullptr;
                            continue;
                        }

                        auto *placeholder = dynamic_cast<Placeholder *>(static_cast<Placeholder *>(element));
                        if (placeholder) return nullptr;

                        int new_head = (head + 1) & MAX_CAPACITY_MASK;
                        long new_state = update_head(state, new_head);
                        if (_state.compare_exchange_weak(state, new_state)) {
                            array_[head & mask_].store(nullptr);
                            return element;
                        }

                        if (!single_consumer_) continue;

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
                        int head = static_cast<int>((state & HEAD_MASK) >> HEAD_SHIFT);
                        assert(head == old_head);

                        if ((state & FROZEN_MASK) != 0L) {
                            return next();
                        }

                        long new_state = update_head(state, new_head);
                        if (_state.compare_exchange_weak(state, new_state)) {
                            array_[head & mask_].store(nullptr);
                            return nullptr;
                        }
                    }
                }

                Core<E> *next() { return allocate_or_get_next_copy(mark_frozen()); }

                long mark_frozen() {
                    while (true) {
                        long state = _state.load();
                        if ((state & FROZEN_MASK) != 0L) return state;
                        long new_state = state | FROZEN_MASK;
                        if (_state.compare_exchange_weak(state, new_state)) return new_state;
                    }
                }

                Core<E> *allocate_or_get_next_copy(long state) {
                    while (true) {
                        Core<E> *next = _next.load();
                        if (next != nullptr) return next;
                        Core<E> *new_next = allocate_next_copy(state);
                        _next.compare_exchange_weak(next, new_next);
                    }
                }

                Core<E> *allocate_next_copy(long state) {
                    Core<E> *next = new Core<E>(capacity_ * 2, single_consumer_);
                    int head = static_cast<int>((state & HEAD_MASK) >> HEAD_SHIFT);
                    int tail = static_cast<int>((state & TAIL_MASK) >> TAIL_SHIFT);

                    int index = head;
                    while ((index & mask_) != (tail & mask_)) {
                        void *value = array_[index & mask_].load();
                        if (value == nullptr) value = new Placeholder(index);
                        next->array_[index & next->mask_].store(value);
                        index++;
                    }
                    next->_state.store(state & ~FROZEN_MASK);
                    return next;
                }

                template<typename R>
                std::vector<R> map(std::function<R(E *)> transform) {
                    std::vector<R> res;
                    long state = _state.load();
                    int head = static_cast<int>((state & HEAD_MASK) >> HEAD_SHIFT);
                    int tail = static_cast<int>((state & TAIL_MASK) >> TAIL_SHIFT);

                    int index = head;
                    while ((index & mask_) != (tail & mask_)) {
                        void *element = array_[index & mask_].load();
                        auto *placeholder = dynamic_cast<Placeholder *>(static_cast<Placeholder *>(element));
                        if (element != nullptr && placeholder == nullptr) {
                            res.push_back(transform(static_cast<E *>(element)));
                        }
                        index++;
                    }
                    return res;
                }

                bool is_closed() { return (_state.load() & CLOSED_MASK) != 0L; }
            };
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx
