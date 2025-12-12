#include <string>
#include <functional>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt
//
// TODO: Implement atomic operations (atomicfu library) - use std::atomic
// TODO: Implement atomicArrayOfNulls - use std::vector<std::atomic<T*>> or custom array
// TODO: Map @Suppress("NOTHING_TO_INLINE") to force inline or comment
// TODO: Implement loop {} helper function for infinite loops
// TODO: Implement List<T> return type - use std::vector<T>
// TODO: Implement lambda with receiver syntax
// TODO: Implement Kotlin's bitwise operations (shl, and, or, inv)
// TODO: Handle const auto - use constexpr or #define

namespace kotlinx {
    namespace coroutines {
        namespace {
            // KT-25023
            // TODO: inline function with Nothing return type
            inline void loop(std::function<void()> block) {
                while (true) {
                    block();
                }
            }

            constexpr int kIsClosedMask = 1 << 31;

            /**
 * A thread-safe resource pool.
 *
 * [maxCapacity] is the maximum amount of elements.
 * [create] is the function that creates a new element.
 *
 * This is only used in the Native implementation,
 * but is part of the `concurrent` source set in order to test it on the JVM.
 */
            template<typename T>
            class OnDemandAllocatingPool {
            private:
                int max_capacity_;
                std::function<T(int)> create_;

                /**
     * Number of existing elements + isClosed flag in the highest bit.
     * Once the flag is set, the value is guaranteed not to change anymore.
     */
                std::atomic<int> control_state_;
                std::vector<std::atomic<T *> > elements_;

                /**
     * Returns the number of elements that need to be cleaned up due to the pool being closed.
     */
                // @Suppress("NOTHING_TO_INLINE")
                inline int try_forbid_new_elements() {
                    // TODO: Implement loop with lambda
                    while (true) {
                        int current = control_state_.load(std::memory_order_acquire);
                        if (is_closed(current)) return 0; // already closed
                        int new_val = current | kIsClosedMask;
                        if (control_state_.compare_exchange_strong(current, new_val,
                                                                   std::memory_order_release,
                                                                   std::memory_order_acquire)) {
                            return current;
                        }
                    }
                }

                // @Suppress("NOTHING_TO_INLINE")
                inline bool is_closed(int value) const {
                    return (value & kIsClosedMask) != 0;
                }

            public:
                OnDemandAllocatingPool(int max_capacity, std::function<T(int)> create)
                    : max_capacity_(max_capacity)
                      , create_(create)
                      , control_state_(0)
                      , elements_(max_capacity) {
                    // Initialize atomic array with nullptrs
                    for (int i = 0; i < max_capacity; ++i) {
                        elements_[i].store(nullptr, std::memory_order_relaxed);
                    }
                }

                /**
     * Request that a new element is created.
     *
     * Returns `false` if the pool is closed.
     *
     * Note that it will still return `true` even if an element was not created due to reaching [maxCapacity].
     *
     * Rethrows the exceptions thrown from [create]. In this case, this operation has no effect.
     */
                bool allocate() {
                    // TODO: Implement loop with lambda
                    while (true) {
                        int ctl = control_state_.load(std::memory_order_acquire);
                        if (is_closed(ctl)) return false;
                        if (ctl >= max_capacity_) return true;
                        if (control_state_.compare_exchange_strong(ctl, ctl + 1,
                                                                   std::memory_order_release,
                                                                   std::memory_order_acquire)) {
                            T *element = new T(create_(ctl));
                            elements_[ctl].store(element, std::memory_order_release);
                            return true;
                        }
                    }
                }

                /**
     * Close the pool.
     *
     * This will prevent any new elements from being created.
     * All the elements present in the pool will be returned.
     *
     * The function is thread-safe.
     *
     * [close] can be called multiple times, but only a single call will return a non-empty list.
     * This is due to the elements being cleaned out from the pool on the first invocation to avoid memory leaks,
     * and no new elements being created after.
     */
                std::vector<T> close() {
                    int elements_existing = try_forbid_new_elements();
                    std::vector<T> result;
                    for (int i = 0; i < elements_existing; ++i) {
                        // we wait for the element to be created, because we know that eventually it is going to be there
                        while (true) {
                            T *element = elements_[i].exchange(nullptr, std::memory_order_acquire);
                            if (element != nullptr) {
                                result.push_back(*element);
                                delete element;
                                break;
                            }
                        }
                    }
                    return result;
                }

                // for tests
                std::string state_representation() const {
                    int ctl = control_state_.load(std::memory_order_acquire);
                    std::string elements_str = "[";
                    int num_elements = ctl & (~kIsClosedMask);
                    for (int i = 0; i < num_elements; ++i) {
                        if (i > 0) elements_str += ", ";
                        T *elem = elements_[i].load(std::memory_order_acquire);
                        if (elem != nullptr) {
                            elements_str += std::to_string(*elem); // TODO: proper tostd::string implementation
                        } else {
                            elements_str += "nullptr";
                        }
                    }
                    elements_str += "]";
                    std::string closed_str = is_closed(ctl) ? "[closed]" : "";
                    return elements_str + closed_str;
                }

                std::string to_string() const {
                    return "OnDemandAllocatingPool(" + state_representation() + ")";
                }
            };
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx