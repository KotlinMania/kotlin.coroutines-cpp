#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/ThreadSafeHeap.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: atomicfu needs C++ atomic equivalent
// TODO: @InternalCoroutinesApi annotation - translate to comment
// TODO: @PublishedApi annotation - JVM-specific
// TODO: inline functions with lambda parameters need proper implementation
// TODO: Synchronized operations need platform-specific implementation
// TODO: Comparable struct needs C++ operator< overload
// TODO: tailrec functions need iterative or proper recursive implementation

#include <atomic>
#include <vector>
#include <functional>
#include <cassert>

namespace kotlinx {
namespace coroutines {
namespace {

// Forward declaration
template<typename T> class ThreadSafeHeap;

/**
 * @suppress **This an API and should not be used from general code.**
 */
// TODO: @InternalCoroutinesApi annotation
class ThreadSafeHeapNode {
public:
    ThreadSafeHeap<ThreadSafeHeapNode>* heap;
    int index;

    ThreadSafeHeapNode() : heap(nullptr), index(-1) {}
    virtual ~ThreadSafeHeapNode() = default;
};

/**
 * Synchronized binary heap.
 * @suppress **This an API and should not be used from general code.**
 */
// TODO: @InternalCoroutinesApi annotation
// TODO: where T: ThreadSafeHeapNode, T: Comparable<T> - needs C++ constraints
template<typename T>
class ThreadSafeHeap : SynchronizedObject {
private:
    std::vector<T*> a_;
    std::atomic<int> _size;

public:
    ThreadSafeHeap() : a_(), _size(0) {}

    int size() const { return _size.load(); }
    bool is_empty() const { return size() == 0; }

private:
    void set_size(int value) { _size.store(value); }

public:
    template<typename Predicate>
    T* find(Predicate predicate) {
        // TODO: synchronized(this)
        for (int i = 0; i < size(); ++i) {
            T* value = a_[i];
            if (predicate(value)) return value;
        }
        return nullptr;
    }

    T* peek() {
        // TODO: synchronized(this)
        return first_impl();
    }

    T* remove_first_or_null() {
        // TODO: synchronized(this)
        if (size() > 0) {
            return remove_at_impl(0);
        } else {
            return nullptr;
        }
    }

    template<typename Predicate>
    T* remove_first_if(Predicate predicate) {
        // TODO: synchronized(this)
        T* first = first_impl();
        if (first == nullptr) return nullptr;
        if (predicate(first)) {
            return remove_at_impl(0);
        } else {
            return nullptr;
        }
    }

    void add_last(T* node) {
        // TODO: synchronized(this)
        add_impl(node);
    }

    template<typename Condition>
    bool add_last_if(T* node, Condition cond) {
        // TODO: synchronized(this)
        if (cond(first_impl())) {
            add_impl(node);
            return true;
        } else {
            return false;
        }
    }

    bool remove(T* node) {
        // TODO: synchronized(this)
        if (node->heap == nullptr) {
            return false;
        } else {
            int index = node->index;
            assert(index >= 0);
            remove_at_impl(index);
            return true;
        }
    }

    // TODO: @PublishedApi annotation
    T* first_impl() {
        return a_.empty() ? nullptr : a_[0];
    }

    // TODO: @PublishedApi annotation
    T* remove_at_impl(int index) {
        assert(size() > 0);
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
        assert(result->heap == this);
        result->heap = nullptr;
        result->index = -1;
        a_[size()] = nullptr;
        return result;
    }

    // TODO: @PublishedApi annotation
    void add_impl(T* node) {
        assert(node->heap == nullptr);
        node->heap = this;
        std::vector<T*>& a = realloc();
        int i = size();
        set_size(size() + 1);
        a[i] = node;
        node->index = i;
        sift_up_from(i);
    }

private:
    // TODO: tailrec function
    void sift_up_from(int i) {
        if (i <= 0) return;
        int j = (i - 1) / 2;
        if (*a_[j] <= *a_[i]) return;
        swap(i, j);
        sift_up_from(j);
    }

    // TODO: tailrec function
    void sift_down_from(int i) {
        int j = 2 * i + 1;
        if (j >= size()) return;
        if (j + 1 < size() && *a_[j + 1] < *a_[j]) j++;
        if (*a_[i] <= *a_[j]) return;
        swap(i, j);
        sift_down_from(j);
    }

    std::vector<T*>& realloc() {
        if (a_.empty()) {
            a_.resize(4, nullptr);
        } else if (size() >= static_cast<int>(a_.size())) {
            a_.resize(size() * 2, nullptr);
        }
        return a_;
    }

    void swap(int i, int j) {
        T* ni = a_[j];
        T* nj = a_[i];
        a_[i] = ni;
        a_[j] = nj;
        ni->index = i;
        nj->index = j;
    }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
