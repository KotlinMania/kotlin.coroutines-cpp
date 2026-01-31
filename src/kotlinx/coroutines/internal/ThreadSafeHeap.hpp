#pragma once

/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/ThreadSafeHeap.kt
 */

#include <vector>
#include <atomic>
#include <mutex>
#include <cassert>
#include <functional>
#include <type_traits>
#include "kotlinx/coroutines/internal/SynchronizedObject.hpp"

namespace kotlinx::coroutines::internal {

// Forward declaration
template<typename T> class ThreadSafeHeap;

/**
 * @suppress **This an internal API and should not be used from general code.**
 */
class ThreadSafeHeapNode {
public:
    ThreadSafeHeap<ThreadSafeHeapNode>* heap;
    int index;

    ThreadSafeHeapNode() : heap(nullptr), index(-1) {}
    virtual ~ThreadSafeHeapNode() = default;
};

/**
 * Synchronized binary heap.
 * @suppress **This an internal API and should not be used from general code.**
 */
template<typename T>
class ThreadSafeHeap : public SynchronizedObject {
private:
    std::vector<T*> a_;
    std::atomic<int> _size;

public:
    ThreadSafeHeap() : _size(0) {}

    virtual ~ThreadSafeHeap() = default;

    int size() const { return _size.load(); }
    bool is_empty() const { return size() == 0; }
    
    // Clear functionality if needed by C++ specific lifecycle
    void clear() {
        synchronized(static_cast<SynchronizedObject&>(*this), [&]() {
            _size.store(0);
            a_.clear();
            return 0; // dummy return
        });
    }

    template<typename Predicate>
    T* find(Predicate predicate) {
        return synchronized(static_cast<SynchronizedObject&>(*this), [&]() -> T* {
             int s = size();
             for (int i = 0; i < s; ++i) {
                 T* value = a_[i];
                 if (predicate(value)) return value;
             }
             return nullptr;
        });
    }

    T* peek() {
        return synchronized(static_cast<SynchronizedObject&>(*this), [&]() -> T* {
            return first_impl();
        });
    }

    T* remove_first_or_null() {
         return synchronized(static_cast<SynchronizedObject&>(*this), [&]() -> T* {
            if (size() > 0) {
                return remove_at_impl(0);
            }
            return nullptr;
        });
    }

    template<typename Predicate>
    T* remove_first_if(Predicate predicate) {
        return synchronized(static_cast<SynchronizedObject&>(*this), [&]() -> T* {
            auto first = first_impl();
            if (!first) return nullptr;
            if (predicate(first)) {
                return remove_at_impl(0);
            }
            return nullptr;
        });
    }

    void add_last(T* node) {
        synchronized(static_cast<SynchronizedObject&>(*this), [&]() {
            add_impl(node);
            return 0;
        });
    }

    template<typename Condition>
    bool add_last_if(T* node, Condition cond) {
         return synchronized(static_cast<SynchronizedObject&>(*this), [&]() -> bool {
             if (cond(first_impl())) {
                 add_impl(node);
                 return true;
             }
             return false;
         });
    }

    bool remove(T* node) {
        return synchronized(static_cast<SynchronizedObject&>(*this), [&]() -> bool {
            if (node->heap == nullptr) {
                return false;
            }
            int index = node->index;
            // assert(index >= 0);
            if (index >= 0) {
                remove_at_impl(index);
                return true;
            }
            return false;
        });
    }
    
    // For manual locking scenarios if public access is needed
    T* first_impl() {
        return a_.empty() || size() == 0 ? nullptr : a_[0];
    }

    T* remove_at_impl(int index) {
        // assert(size() > 0);
        int s = size();
        set_size(s - 1);
        s = size(); // new size
        
        if (index < s) {
            swap(index, s);
            int j = (index - 1) / 2;
            if (index > 0 && *a_[index] < *a_[j]) {
                swap(index, j);
                sift_up_from(j);
            } else {
                sift_down_from(index);
            }
        }
        
        T* result = a_[s];
        // assert(result->heap == this);
        result->heap = nullptr;
        result->index = -1;
        a_[s] = nullptr;
        return result;
    }

private:
    void set_size(int value) { _size.store(value); }

    void add_impl(T* node) {
        // assert(node->heap == nullptr);
        // Cast to correct heap type helper
        node->heap = reinterpret_cast<ThreadSafeHeap<ThreadSafeHeapNode>*>(this);
        realloc();
        int i = size();
        set_size(i + 1);
        a_[i] = node;
        node->index = i;
        sift_up_from(i);
    }

    void sift_up_from(int i) {
        if (i <= 0) return;
        int j = (i - 1) / 2;
        if (*a_[j] <= *a_[i]) return;
        swap(i, j);
        sift_up_from(j);
    }

    void sift_down_from(int i) {
        int s = size();
        int j = 2 * i + 1;
        if (j >= s) return;
        if (j + 1 < s && *a_[j + 1] < *a_[j]) j++;
        if (*a_[i] <= *a_[j]) return;
        swap(i, j);
        sift_down_from(j);
    }

    void realloc() {
        int s = size();
        if (a_.empty()) {
            a_.resize(4, nullptr);
        } else if (s >= static_cast<int>(a_.size())) {
            a_.resize(s * 2, nullptr);
        }
    }

    void swap(int i, int j) {
        T* ni = a_[j];
        T* nj = a_[i];
        a_[i] = ni;
        a_[j] = nj;
        if (ni) ni->index = i;
        if (nj) nj->index = j;
    }
};

} // namespace kotlinx::coroutines::internal
