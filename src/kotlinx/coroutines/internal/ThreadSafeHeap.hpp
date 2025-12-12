#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include <vector>
#include <atomic>
#include <mutex>
#include <cassert>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace internal {

// Forward declaration
template<typename T> class ThreadSafeHeap;

class ThreadSafeHeapNode {
public:
    ThreadSafeHeap<ThreadSafeHeapNode>* heap;
    int index;

    ThreadSafeHeapNode() : heap(nullptr), index(-1) {}
    virtual ~ThreadSafeHeapNode() = default;
};

template<typename T>
class ThreadSafeHeap {
private:
    std::vector<T*> a_;
    std::atomic<int> _size;
    std::mutex mutex_; // Basic synchronization

public:
    ThreadSafeHeap() : _size(0) {}

    int size() const { return _size.load(); }
    bool is_empty() const { return size() == 0; }

    template<typename Predicate>
    T* find(Predicate predicate) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (int i = 0; i < size(); ++i) {
            T* value = a_[i];
            if (predicate(value)) return value;
        }
        return nullptr;
    }

    T* peek() {
        std::lock_guard<std::mutex> lock(mutex_);
        return first_impl();
    }

    T* remove_first_or_null() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (size() > 0) {
            return remove_at_impl(0);
        } else {
            return nullptr;
        }
    }

    void add_last(T* node) {
        std::lock_guard<std::mutex> lock(mutex_);
        add_impl(node);
    }

    bool remove(T* node) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (node->heap == nullptr) {
            return false;
        } else {
            int index = node->index;
            if (index >= 0) {
                 remove_at_impl(index);
                 return true;
            }
            return false;
        }
    }

private:
    void set_size(int value) { _size.store(value); }

    T* first_impl() {
        return a_.empty() ? nullptr : a_[0];
    }

    T* remove_at_impl(int index) {
        set_size(size() - 1);
        if (index < size()) {
            swap(index, size());
            int j = (index - 1) / 2;
            if (index > 0 && *a_[index] < *a_[j]) {
                swap(index, j);
                sift_up_from(j);
            } else {
                sift_down_from(index);
            }
        }
        T* result = a_[size()];
        if (result) {
            // result->heap = nullptr; // TODO
            result->index = -1;
        }
        a_[size()] = nullptr;
        return result;
    }

    void add_impl(T* node) {
        // node->heap = this; // TODO
        realloc();
        int i = size();
        set_size(size() + 1);
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
        int j = 2 * i + 1;
        if (j >= size()) return;
        if (j + 1 < size() && *a_[j + 1] < *a_[j]) j++;
        if (*a_[i] <= *a_[j]) return;
        swap(i, j);
        sift_down_from(j);
    }

    void realloc() {
        if (a_.empty()) {
            a_.resize(4, nullptr);
        } else if (size() >= static_cast<int>(a_.size())) {
            a_.resize(size() * 2, nullptr);
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

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
