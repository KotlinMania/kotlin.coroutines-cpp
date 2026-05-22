/**
 * Transliterated from: kotlinx-coroutines-core/native/src/internal/CopyOnWriteList.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Native actual of `CopyOnWriteList<E>`: a thread-safe list that snapshots the underlying
 * vector on every modification. The C++ port uses an std::shared_ptr<std::vector<E>>
 * with CAS-via-mutex semantics — readers walk the shared snapshot while writers swap a
 * fresh vector into place.
 */

#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>

namespace kotlinx::coroutines::internal {

template <typename E>
class CopyOnWriteList {
public:
    CopyOnWriteList() : snapshot_(std::make_shared<std::vector<E>>()) {}

    /**
     * Upstream:
     *   override fun add(element: E): Boolean { ... }
     *
     * Snapshot-copy-modify-swap: takes the current snapshot, builds a new vector with
     * the element appended, and atomically swaps it in under the writer mutex.
     */
    bool add(E element) {
        std::lock_guard<std::mutex> guard(writer_mutex_);
        auto next = std::make_shared<std::vector<E>>(*snapshot_);
        next->push_back(std::move(element));
        std::atomic_store(&snapshot_, next);
        return true;
    }

    /**
     * Upstream:
     *   override fun remove(element: E): Boolean { ... }
     */
    bool remove(const E& element) {
        std::lock_guard<std::mutex> guard(writer_mutex_);
        auto next = std::make_shared<std::vector<E>>(*snapshot_);
        auto it = std::find(next->begin(), next->end(), element);
        if (it == next->end()) return false;
        next->erase(it);
        std::atomic_store(&snapshot_, next);
        return true;
    }

    /** Returns a snapshot of the current contents; safe for read-only iteration. */
    std::shared_ptr<const std::vector<E>> snapshot() const {
        return std::atomic_load(&snapshot_);
    }

private:
    std::shared_ptr<std::vector<E>> snapshot_;
    std::mutex writer_mutex_;
};

} // namespace kotlinx::coroutines::internal
