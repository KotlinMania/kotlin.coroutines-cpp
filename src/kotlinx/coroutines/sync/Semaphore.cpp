/**
 * @file Semaphore.cpp
 * @brief Semaphore implementation using lock-free segment queue.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt
 *
 * Contains the private SemaphoreImpl class (lines 355-357 in Kotlin).
 */

#include "kotlinx/coroutines/sync/Semaphore.hpp"
#include "kotlinx/coroutines/sync/SemaphoreAndMutexImpl.hpp"
#include <memory>
#include "kotlinx/coroutines/dsl/CancellableReusable.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"
#include <condition_variable>
#include <mutex>
#include <optional>

namespace kotlinx {
namespace coroutines {
namespace sync {

// NOTE(port): Blocking C++ adapter, using the actual coroutine queue rather than polling.
void Semaphore::acquire() {
    std::mutex mutex;
    std::condition_variable ready;
    std::optional<Result<void*>> resumed;
    FunctionalContinuation<void*> completion(EmptyCoroutineContext::instance(), [&](Result<void*> result) {
        std::lock_guard<std::mutex> lock(mutex);
        resumed.emplace(std::move(result));
        ready.notify_one();
    });
    void* result = acquire(&completion);
    if (!intrinsics::is_coroutine_suspended(result)) return;
    std::unique_lock<std::mutex> lock(mutex);
    ready.wait(lock, [&] { return resumed.has_value(); });
    resumed->get_or_throw();
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:90-149
SemaphoreAndMutexImpl::SemaphoreAndMutexImpl(int permits, int acquired_permits)
    : permits_(permits)
    , available_permits_(permits - acquired_permits) {
    if (permits <= 0) {
        throw std::invalid_argument(
            "Semaphore should have at least 1 permit, but had " + std::to_string(permits));
    }
    if (acquired_permits < 0 || acquired_permits > permits) {
        throw std::invalid_argument(
            "The number of acquired permits should be in 0.." + std::to_string(permits));
    }
    auto s = new SemaphoreSegment(0, nullptr, 2);
    head_.store(s, std::memory_order_relaxed);
    tail_.store(s, std::memory_order_relaxed);

    on_cancellation_release_ = [this](std::exception_ptr, void*, std::shared_ptr<CoroutineContext>) {
        release();
    };
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:147-147
int SemaphoreAndMutexImpl::available_permits() const {
    return std::max(available_permits_.load(std::memory_order_acquire), 0);
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:151-168
bool SemaphoreAndMutexImpl::try_acquire() {
    while (true) {
        int p = available_permits_.load(std::memory_order_acquire);

        if (p > permits_) {
            coerce_available_permits_at_maximum();
            continue;
        }

        if (p <= 0) return false;
        if (available_permits_.compare_exchange_weak(p, p - 1,
                std::memory_order_release, std::memory_order_relaxed)) {
            return true;
        }
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:170-180
void* SemaphoreAndMutexImpl::acquire(Continuation<void*>* cont) {
    int p = dec_permits();
    if (p > 0) {
        return nullptr; // Permit acquired, return Unit
    }
    return acquire_slow_path(cont);
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:242-262
void SemaphoreAndMutexImpl::release() {
    while (true) {
        int p = available_permits_.fetch_add(1, std::memory_order_acq_rel);

        if (p >= permits_) {
            coerce_available_permits_at_maximum();
            throw std::logic_error(
                "The number of released permits cannot be greater than " +
                std::to_string(permits_));
        }

        if (p >= 0) return;

        if (try_resume_next_from_queue()) return;
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:192-196
void SemaphoreAndMutexImpl::acquire_waiter(CancellableContinuation<void>* waiter) {
    acquire_internal(dynamic_cast<Waiter*>(waiter),
        [this](Waiter* cont) { return add_acquire_to_queue(cont); },
        [this](Waiter* cont) { resume_waiter_with_permit(cont); });
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:215-220
void SemaphoreAndMutexImpl::on_acquire_reg_function(selects::SelectInstanceBase* select, void*) {
    acquire_internal(dynamic_cast<Waiter*>(select),
        [this](Waiter* instance) { return add_acquire_to_queue(instance); },
        [](Waiter* instance) {
            dynamic_cast<selects::SelectInstanceBase*>(instance)->select_in_registration_phase(nullptr);
        });
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:182-189
void* SemaphoreAndMutexImpl::acquire_slow_path(Continuation<void*>* cont) {
    return dsl::suspend_cancellable_coroutine_reusable_void(cont,
        [this](CancellableContinuationImpl<void>* cancellable_cont) {
            // Try to suspend; on synchronous elimination failure restart acquire.
            if (add_acquire_to_queue(cancellable_cont)) return;
            acquire_waiter(cancellable_cont);
        });
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:199-211
void SemaphoreAndMutexImpl::acquire_internal(Waiter* waiter, std::function<bool(Waiter*)> suspend_func,
                      std::function<void(Waiter*)> on_acquired) {
    while (true) {
        int p = dec_permits();
        if (p > 0) {
            on_acquired(waiter);
            return;
        }
        if (suspend_func(waiter)) return;
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:229-240
int SemaphoreAndMutexImpl::dec_permits() {
    while (true) {
        int p = available_permits_.fetch_sub(1, std::memory_order_acq_rel);
        if (p > permits_) continue;
        return p;
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:269-275
void SemaphoreAndMutexImpl::coerce_available_permits_at_maximum() {
    while (true) {
        int cur = available_permits_.load(std::memory_order_acquire);
        if (cur <= permits_) break;
        if (available_permits_.compare_exchange_weak(cur, permits_,
                std::memory_order_release, std::memory_order_relaxed)) {
            break;
        }
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:280-310
bool SemaphoreAndMutexImpl::add_acquire_to_queue(Waiter* waiter) {
    SemaphoreSegment* cur_tail = tail_.load(std::memory_order_acquire);

    long enq_idx = enq_idx_.fetch_add(1, std::memory_order_acq_rel);

    auto result = internal::find_segment_and_move_forward(
        tail_,
        enq_idx / SEGMENT_SIZE(),
        cur_tail,
        create_segment
    );
    SemaphoreSegment* segment = result.segment();

    int i = static_cast<int>(enq_idx % SEGMENT_SIZE());

    void* expected = nullptr;
    if (segment->cas(i, expected, waiter, waiter->shared_from_this_waiter())) {
        waiter->invoke_on_cancellation(segment, i);
        return true;
    }

    if (segment->cas(i, static_cast<void*>(&PERMIT()), static_cast<void*>(&TAKEN()))) {
        resume_waiter_with_permit(waiter);
        return true;
    }

    assert(segment->get(i) == static_cast<void*>(&BROKEN()));
    return false;
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:313-337
bool SemaphoreAndMutexImpl::try_resume_next_from_queue() {
    SemaphoreSegment* cur_head = head_.load(std::memory_order_acquire);

    long deq_idx = deq_idx_.fetch_add(1, std::memory_order_acq_rel);

    long segment_id = deq_idx / SEGMENT_SIZE();

    auto result = internal::find_segment_and_move_forward(
        head_,
        segment_id,
        cur_head,
        create_segment
    );
    SemaphoreSegment* segment = result.segment();

    segment->clean_prev();

    if (segment->id > segment_id) return false;

    int i = static_cast<int>(deq_idx % SEGMENT_SIZE());

    auto cell = segment->get_and_set(i, static_cast<void*>(&PERMIT()));
    void* cell_state = cell ? cell->state : nullptr;

    if (cell_state == nullptr) {
        for (int spin = 0; spin < MAX_SPIN_CYCLES(); ++spin) {
            if (segment->get(i) == static_cast<void*>(&TAKEN())) {
                return true;
            }
        }
        return !segment->cas(i, static_cast<void*>(&PERMIT()), static_cast<void*>(&BROKEN()));
    }

    if (cell_state == static_cast<void*>(&CANCELLED())) {
        return false;
    }

    return try_resume_acquire(cell->waiter.get());
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:339-352
bool SemaphoreAndMutexImpl::try_resume_acquire(Waiter* waiter) {
    if (auto* cont = dynamic_cast<CancellableContinuation<void>*>(waiter)) {
        void* token = cont->try_resume(nullptr, on_cancellation_release_);
        if (token != nullptr) {
            cont->complete_resume(token);
            return true;
        }
        return false;
    }
    if (auto* select = dynamic_cast<selects::SelectInstanceBase*>(waiter)) {
        return select->try_select(this, nullptr);
    }
    throw std::logic_error("unexpected semaphore waiter");
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:296-305
void SemaphoreAndMutexImpl::resume_waiter_with_permit(Waiter* waiter) {
    if (auto* cont = dynamic_cast<CancellableContinuation<void>*>(waiter)) {
        cont->resume([this](std::exception_ptr) { release(); });
    } else if (auto* select = dynamic_cast<selects::SelectInstanceBase*>(waiter)) {
        select->select_in_registration_phase(nullptr);
    } else {
        throw std::logic_error("unexpected semaphore waiter");
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:361-363
SemaphoreSegment::SemaphoreSegment(long id, SemaphoreSegment* prev, int pointers)
    : Segment<SemaphoreSegment>(id, prev, pointers),
      acquirers_(std::make_unique<std::shared_ptr<Cell>[]>(SEGMENT_SIZE())) {}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:363-363
int SemaphoreSegment::number_of_slots() const { return SEGMENT_SIZE(); }

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:366-366
void* SemaphoreSegment::get(int index) const {
    auto cell = std::atomic_load(&acquirers_[index]);
    return cell ? cell->state : nullptr;
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:369-371
void SemaphoreSegment::set(int index, void* value) {
    std::atomic_store(&acquirers_[index], std::make_shared<Cell>(Cell{value, {}}));
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:374-374
bool SemaphoreSegment::cas(int index, void* expected, void* value, std::shared_ptr<Waiter> waiter) {
    auto current = std::atomic_load(&acquirers_[index]);
    auto update = std::make_shared<Cell>(Cell{value, std::move(waiter)});
    while ((current ? current->state : nullptr) == expected) {
        if (std::atomic_compare_exchange_strong(&acquirers_[index], &current, update)) return true;
    }
    return false;
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:377-377
std::shared_ptr<SemaphoreSegment::Cell> SemaphoreSegment::get_and_set(int index, void* value) {
    return std::atomic_exchange(&acquirers_[index], std::make_shared<Cell>(Cell{value, {}}));
}

// Cleans the acquirer slot and removes this segment when all slots are cleaned.
// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:381-386
void SemaphoreSegment::on_cancellation(int index, std::exception_ptr,
                                      std::shared_ptr<CoroutineContext>) {
    set(index, &CANCELLED());
    on_slot_cleaned();
}

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:388-388
std::string SemaphoreSegment::to_string() const {
    return "SemaphoreSegment[id=" + std::to_string(id) + ", hashCode=" +
        std::to_string(std::hash<const SemaphoreSegment*>{}(this)) + "]";
}

/**
 * Line 355-357: SemaphoreImpl
 *
 * private class SemaphoreImpl(
 *     permits: Int, acquiredPermits: Int
 * ): SemaphoreAndMutexImpl(permits, acquiredPermits), Semaphore
 *
 * This private implementation class inherits from both the implementation
 * base and the public interface to provide the complete Semaphore functionality.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:355-357
class SemaphoreImpl : public SemaphoreAndMutexImpl, public Semaphore {
public:
    SemaphoreImpl(int permits, int acquired_permits)
        : SemaphoreAndMutexImpl(permits, acquired_permits)
    {}

    int available_permits() const override {
        return SemaphoreAndMutexImpl::available_permits();
    }

    void* acquire(Continuation<void*>* cont) override {
        return SemaphoreAndMutexImpl::acquire(cont);
    }

    bool try_acquire() override {
        return SemaphoreAndMutexImpl::try_acquire();
    }

    void release() override {
        SemaphoreAndMutexImpl::release();
    }
};

// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:68-68
std::shared_ptr<Semaphore> create_semaphore(int permits, int acquired_permits) {
    return std::make_shared<SemaphoreImpl>(permits, acquired_permits);
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
