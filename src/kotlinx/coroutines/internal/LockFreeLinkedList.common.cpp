/**
 * Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt
 */
#include "kotlinx/coroutines/internal/LockFreeLinkedList.hpp"
#include <atomic>
#include <cassert>
#include <cstdint>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <typeinfo>
#include <vector>

namespace kotlinx::coroutines::internal {
namespace {

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:260-262
// NOTE(port): Derive the marker from Node to represent Kotlin's Node | Removed atomic slot.
class Removed final : public LockFreeLinkedListNode {
public:
    explicit Removed(LockFreeLinkedListNode* node) : ref(node) {}
    LockFreeLinkedListNode* const ref;
    // Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:261
    std::string to_string() const override { return "Removed[" + ref->to_string() + "]"; }
};

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:287
class ListClosed final : public LockFreeLinkedListNode {
public:
    explicit ListClosed(int forbidden) : forbidden_elements_bitmask(forbidden) {}
    const int forbidden_elements_bitmask;
};

} // namespace

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:30-35
struct LockFreeLinkedListNode::Impl {
    explicit Impl(LockFreeLinkedListNode* node) : next(node), previous(node) {}
    ~Impl() { delete removed.load(); }
    std::atomic<LockFreeLinkedListNode*> next;
    std::atomic<LockFreeLinkedListNode*> previous;
    std::atomic<LockFreeLinkedListNode*> removed{nullptr};
    // NOTE(port): Marker lifetimes replace GC ownership; list users keep nodes alive during traversal.
    std::mutex markers_mutex;
    std::vector<std::unique_ptr<LockFreeLinkedListNode>> closed_markers;
};

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:29-32
LockFreeLinkedListNode::LockFreeLinkedListNode() : impl_(std::make_unique<Impl>(this)) {}
LockFreeLinkedListNode::~LockFreeLinkedListNode() = default;

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:34-35
LockFreeLinkedListNode* LockFreeLinkedListNode::removed_ref() {
    auto* cached = impl_->removed.load(std::memory_order_acquire);
    if (cached) return cached;
    auto marker = std::make_unique<Removed>(this);
    if (impl_->removed.compare_exchange_strong(cached, marker.get())) return marker.release();
    return cached;
}

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:37
bool LockFreeLinkedListNode::is_removed() const {
    return dynamic_cast<Removed*>(impl_->next.load(std::memory_order_acquire)) != nullptr;
}

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:40
void* LockFreeLinkedListNode::get_next() const { return impl_->next.load(std::memory_order_acquire); }

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:43-44
LockFreeLinkedListNode* LockFreeLinkedListNode::next_node() const {
    auto* next = impl_->next.load(std::memory_order_acquire);
    if (auto* removed = dynamic_cast<Removed*>(next)) return removed->ref;
    return next;
}

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:51-57
LockFreeLinkedListNode* LockFreeLinkedListNode::prev_node() const {
    if (auto* corrected = correct_prev()) return corrected;
    auto* previous = impl_->previous.load(std::memory_order_acquire);
    while (previous->is_removed()) previous = previous->impl_->previous.load(std::memory_order_acquire);
    return previous;
}

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:61-74
bool LockFreeLinkedListNode::add_one_if_empty(LockFreeLinkedListNode* node) {
    node->impl_->previous.store(this, std::memory_order_relaxed);
    node->impl_->next.store(this, std::memory_order_relaxed);
    while (true) {
        auto* next = impl_->next.load(std::memory_order_acquire);
        if (next != this) return false;
        if (impl_->next.compare_exchange_strong(next, node)) {
            node->finish_add(this);
            return true;
        }
    }
}

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:82-92
bool LockFreeLinkedListNode::add_last(LockFreeLinkedListNode* node, int permissions_bitmask) {
    while (true) {
        auto* previous = prev_node();
        if (auto* closed = dynamic_cast<ListClosed*>(previous)) {
            return (closed->forbidden_elements_bitmask & permissions_bitmask) == 0 &&
                   closed->add_last(node, permissions_bitmask);
        }
        if (previous->add_next(node, this)) return true;
    }
}

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:98-100
void LockFreeLinkedListNode::close(int forbidden_elements_bit) {
    auto marker = std::make_unique<ListClosed>(forbidden_elements_bit);
    auto* node = marker.get();
    {
        std::lock_guard<std::mutex> lock(impl_->markers_mutex);
        impl_->closed_markers.push_back(std::move(marker));
    }
    (void)add_last(node, forbidden_elements_bit);
}

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:129-137
bool LockFreeLinkedListNode::add_next(LockFreeLinkedListNode* node, LockFreeLinkedListNode* next) {
    node->impl_->previous.store(this, std::memory_order_relaxed);
    node->impl_->next.store(next, std::memory_order_relaxed);
    auto* expected = next;
    if (!impl_->next.compare_exchange_strong(expected, node)) return false;
    node->finish_add(next);
    return true;
}

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:151-152
bool LockFreeLinkedListNode::remove() { return remove_or_next() == nullptr; }

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:156-168
LockFreeLinkedListNode* LockFreeLinkedListNode::remove_or_next() {
    while (true) {
        auto* next = impl_->next.load(std::memory_order_acquire);
        if (auto* removed = dynamic_cast<Removed*>(next)) return removed->ref;
        if (next == this) return next;
        auto* marker = next->removed_ref();
        if (impl_->next.compare_exchange_strong(next, marker)) {
            next->correct_prev();
            return nullptr;
        }
    }
}

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:201-213
void LockFreeLinkedListNode::finish_add(LockFreeLinkedListNode* next) {
    while (true) {
        auto* previous = next->impl_->previous.load(std::memory_order_acquire);
        if (impl_->next.load(std::memory_order_acquire) != next) return;
        if (next->impl_->previous.compare_exchange_strong(previous, this)) {
            if (is_removed()) next->correct_prev();
            return;
        }
    }
}

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:225-249
LockFreeLinkedListNode* LockFreeLinkedListNode::correct_prev() const {
    while (true) {
        auto* old_previous = impl_->previous.load(std::memory_order_acquire);
        auto* previous = old_previous;
        LockFreeLinkedListNode* last = nullptr;
        while (true) {
            auto* previous_next = previous->impl_->next.load(std::memory_order_acquire);
            if (previous_next == this) {
                if (old_previous == previous) return previous;
                if (!impl_->previous.compare_exchange_strong(old_previous, previous)) break;
                return previous;
            }
            if (is_removed()) return nullptr;
            if (auto* removed = dynamic_cast<Removed*>(previous_next)) {
                if (last) {
                    auto* expected = previous;
                    if (!last->impl_->next.compare_exchange_strong(expected, removed->ref)) break;
                    previous = last;
                    last = nullptr;
                } else {
                    previous = previous->impl_->previous.load(std::memory_order_acquire);
                }
            } else {
                last = previous;
                previous = previous_next;
            }
        }
    }
}

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:251-254
void LockFreeLinkedListNode::validate_node(LockFreeLinkedListNode* previous, LockFreeLinkedListNode* next) const {
    assert(previous == impl_->previous.load());
    assert(next == impl_->next.load());
}

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:256
std::string LockFreeLinkedListNode::to_string() const {
    // NOTE(port): Match the Native debug representation using host RTTI and the object address.
    std::ostringstream output;
    output << typeid(*this).name() << '@' << std::hex << reinterpret_cast<std::uintptr_t>(this);
    return output.str();
}

// NOTE(port): Compatibility helpers delegate to upstream's consolidated correction operation.
void LockFreeLinkedListNode::help_remove() { next_node()->correct_prev(); }
void LockFreeLinkedListNode::remove_help_needed(LockFreeLinkedListNode* node) { node->correct_prev(); }

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:270
LockFreeLinkedListHead::LockFreeLinkedListHead() = default;

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:282
bool LockFreeLinkedListHead::remove() { throw std::logic_error("head cannot be removed"); }

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:285
bool LockFreeLinkedListHead::is_removed() const { return false; }

bool LockFreeLinkedListHead::is_empty() const { return next_node() == this; }

} // namespace kotlinx::coroutines::internal
