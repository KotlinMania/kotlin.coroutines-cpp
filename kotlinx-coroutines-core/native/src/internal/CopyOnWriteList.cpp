// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/internal/CopyOnWriteList.kt
//
// TODO: @Suppress annotation
// TODO: AbstractMutableList base class
// TODO: kotlinx.atomicfu atomic references
// TODO: MutableIterator and MutableListIterator interfaces

namespace kotlinx {
namespace coroutines {
namespace internal {

// TODO: Remove imports, fully qualify or add includes:
// import kotlinx.atomicfu.*

// TODO: @Suppress("UNCHECKED_CAST")
// TODO: internal class extending AbstractMutableList<E>
template<typename E>
class CopyOnWriteList {
private:
    std::atomic<std::vector<void*>*> _array;

    std::vector<void*>* get_array() {
        return _array.load();
    }

    void set_array(std::vector<void*>* value) {
        _array.store(value);
    }

public:
    CopyOnWriteList() {
        _array.store(new std::vector<void*>());
    }

    int size() const {
        return get_array()->size();
    }

    bool add(E element) {
        auto* arr = get_array();
        int n = arr->size();
        auto* update = new std::vector<void*>(*arr);
        update->resize(n + 1);
        (*update)[n] = static_cast<void*>(element);
        set_array(update);
        return true;
    }

    void add(int index, E element) {
        range_check(index);
        auto* arr = get_array();
        int n = arr->size();
        auto* update = new std::vector<void*>(n + 1);

        // Copy elements before index
        for (int i = 0; i < index; i++) {
            (*update)[i] = (*arr)[i];
        }
        (*update)[index] = static_cast<void*>(element);
        // Copy elements after index
        for (int i = index; i < n; i++) {
            (*update)[i + 1] = (*arr)[i];
        }
        set_array(update);
    }

    bool remove(E element) {
        auto* arr = get_array();
        auto it = std::find(arr->begin(), arr->end(), static_cast<void*>(element));
        if (it == arr->end()) return false;
        int index = std::distance(arr->begin(), it);
        remove_at(index);
        return true;
    }

    E remove_at(int index) {
        range_check(index);
        auto* arr = get_array();
        int n = arr->size();
        E element = static_cast<E>((*arr)[index]);
        auto* update = new std::vector<void*>(n - 1);

        // Copy elements before index
        for (int i = 0; i < index; i++) {
            (*update)[i] = (*arr)[i];
        }
        // Copy elements after index
        for (int i = index + 1; i < n; i++) {
            (*update)[i - 1] = (*arr)[i];
        }
        set_array(update);
        return element;
    }

    // TODO: iterator(), listIterator() implementations

    bool is_empty() const {
        return size() == 0;
    }

    E set(int index, E element) {
        throw std::runtime_error("Operation is not supported");
    }

    E get(int index) {
        range_check(index);
        return static_cast<E>((*get_array())[index]);
    }

private:
    // TODO: private class IteratorImpl

    int range_check(int index) {
        int sz = size();
        if (index < 0 || index >= sz) {
            throw std::out_of_range("index: " + std::to_string(index) + ", size: " + std::to_string(sz));
        }
        return index;
    }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
