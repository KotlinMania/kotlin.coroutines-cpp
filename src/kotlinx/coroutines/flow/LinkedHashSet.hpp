#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collection.kt:16-17
 * (Supporting Kotlin's LinkedHashSet container contract)
 */

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <unordered_set>
#include <utility>
#include <vector>

namespace kotlinx::coroutines::flow {

/**
 * An insertion-ordered set transliterating Kotlin's LinkedHashSet.
 * Preserves insertion order while providing O(1) deduplication via hash lookup.
 * Does NOT require operator< (relies solely on Hash and KeyEqual).
 */
template <typename T, typename Hash = std::hash<T>, typename KeyEqual = std::equal_to<T>>
class LinkedHashSet {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    LinkedHashSet() = default;

    LinkedHashSet(std::initializer_list<T> init) {
        for (const auto& item : init) {
            insert(item);
        }
    }

    template <typename InputIt>
    LinkedHashSet(InputIt first, InputIt last) {
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

    LinkedHashSet(const LinkedHashSet&) = default;
    LinkedHashSet(LinkedHashSet&&) noexcept = default;
    LinkedHashSet& operator=(const LinkedHashSet&) = default;
    LinkedHashSet& operator=(LinkedHashSet&&) noexcept = default;
    ~LinkedHashSet() = default;

    // Iterators (iterate in insertion order)
    iterator begin() noexcept { return elements_.begin(); }
    iterator end() noexcept { return elements_.end(); }
    const_iterator begin() const noexcept { return elements_.begin(); }
    const_iterator end() const noexcept { return elements_.end(); }
    const_iterator cbegin() const noexcept { return elements_.cbegin(); }
    const_iterator cend() const noexcept { return elements_.cend(); }

    // Capacity
    [[nodiscard]] bool empty() const noexcept { return elements_.empty(); }
    [[nodiscard]] size_type size() const noexcept { return elements_.size(); }

    void clear() noexcept {
        elements_.clear();
        set_.clear();
    }

    // Lookup
    bool contains(const T& value) const {
        return set_.find(value) != set_.end();
    }

    const_iterator find(const T& value) const {
        if (!contains(value)) return elements_.end();
        return std::find(elements_.begin(), elements_.end(), value);
    }

    iterator find(const T& value) {
        if (!contains(value)) return elements_.end();
        return std::find(elements_.begin(), elements_.end(), value);
    }

    // Insertion
    std::pair<iterator, bool> insert(const T& value) {
        auto [set_it, inserted] = set_.insert(value);
        (void)set_it;
        if (inserted) {
            elements_.push_back(value);
            return {elements_.end() - 1, true};
        }
        return {find(value), false};
    }

    std::pair<iterator, bool> insert(T&& value) {
        if (set_.find(value) != set_.end()) {
            return {find(value), false};
        }
        set_.insert(value);
        elements_.push_back(std::move(value));
        return {elements_.end() - 1, true};
    }

    // Hinted insertion required by ToCollectionFrame::emit
    iterator insert(const_iterator /*hint*/, const T& value) {
        auto res = insert(value);
        return res.first;
    }

    iterator insert(const_iterator /*hint*/, T&& value) {
        auto res = insert(std::move(value));
        return res.first;
    }

    // Element access
    const std::vector<T>& elements() const noexcept { return elements_; }
    const T& operator[](size_type index) const { return elements_[index]; }

    // Equality: sets with identical elements (and size)
    bool operator==(const LinkedHashSet& other) const {
        if (this == &other) return true;
        if (size() != other.size()) return false;
        for (const auto& item : elements_) {
            if (!other.contains(item)) return false;
        }
        return true;
    }

    bool operator!=(const LinkedHashSet& other) const {
        return !(*this == other);
    }

private:
    std::vector<T> elements_;
    std::unordered_set<T, Hash, KeyEqual> set_;
};

} // namespace kotlinx::coroutines::flow

namespace kotlinx::coroutines {
using flow::LinkedHashSet;
} // namespace kotlinx::coroutines
