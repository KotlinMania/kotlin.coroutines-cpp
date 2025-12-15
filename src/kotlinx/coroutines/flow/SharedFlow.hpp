#pragma once
/**
 * @file SharedFlow.hpp
 * @brief SharedFlow and MutableSharedFlow interfaces and implementation
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/SharedFlow.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/DisposableHandle.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <vector>
#include <mutex>
#include <memory>
#include <functional>
#include <algorithm>
#include <cassert>
#include <limits>

namespace kotlinx::coroutines::flow {

// Forward declarations
template<typename T> class SharedFlowImpl;
template<typename T> struct StateFlow;  // StateFlow is defined as struct in StateFlow.hpp
class SharedFlowSlot;

/**
 * A _hot_ Flow that shares emitted values among all its collectors in a broadcast fashion,
 * so that all collectors get all emitted values.
 *
 * Kotlin: public interface SharedFlow<out T> : Flow<T>
 * (lines 124-151 of SharedFlow.kt)
 *
 * **Shared flow never completes**. A call to Flow.collect on a shared flow never completes normally.
 *
 * See the Kotlin documentation for full details on shared flows, buffer overflow strategies,
 * replay cache, and concurrency guarantees.
 *
 * @note The SharedFlow interface is not stable for inheritance in 3rd party libraries.
 *       Use the MutableSharedFlow(...) constructor function to create an implementation.
 */

template<typename T>
class SharedFlow : public Flow<T> {
public:
    ~SharedFlow() override = default;

    /**
     * A snapshot of the replay cache.
     *
     * Kotlin: public val replayCache: List<T>
     */
    virtual std::vector<T> get_replay_cache() const = 0;

    /**
     * Accepts the given collector and emits values into it.
     *
     * **A shared flow never completes**. A call to Flow.collect or any other terminal operator
     * on a shared flow never completes normally.
     *
     * Kotlin: override suspend fun collect(collector: FlowCollector<T>): Nothing
     *
     * @note This is a suspend function - takes Continuation parameter and returns void* (COROUTINE_SUSPENDED or result)
     */
    void* collect(FlowCollector<T>* collector, Continuation<void*>* continuation) override = 0;
};

/**
 * A mutable SharedFlow that provides functions to emit values to the flow.
 *
 * Kotlin: public interface MutableSharedFlow<T> : SharedFlow<T>, FlowCollector<T>
 * (lines 177-259 of SharedFlow.kt)
 *
 * @note The MutableSharedFlow interface is not stable for inheritance in 3rd party libraries.
 *       Use the MutableSharedFlow(...) constructor function to create an implementation.
 */
template<typename T>
class MutableSharedFlow : public SharedFlow<T>, public FlowCollector<T> {
public:
    virtual ~MutableSharedFlow() = default;

    /**
     * Emits a value to this shared flow, suspending on buffer overflow.
     *
     * Kotlin: override suspend fun emit(value: T)
     *
     * @note This is inherited from FlowCollector<T> which already declares:
     *       virtual void* emit(T value, Continuation<void*>* continuation) = 0;
     *       So no need to redeclare here - it's already available.
     */
    // emit() is inherited from FlowCollector<T>

    /**
     * Tries to emit a value to this shared flow without suspending.
     *
     * Kotlin: public fun tryEmit(value: T): Boolean
     */
    virtual bool try_emit(T value) = 0;

    /**
     * The number of subscribers (active collectors) to this shared flow.
     *
     * Kotlin: public val subscriptionCount: StateFlow<Int>
     */
    virtual StateFlow<int>* get_subscription_count() = 0;

    /**
     * Resets the replayCache of this shared flow to an empty state.
     *
     * Kotlin: @ExperimentalCoroutinesApi public fun resetReplayCache()
     */
    virtual void reset_replay_cache() = 0;
};

/**
 * Creates a MutableSharedFlow with the given configuration parameters.
 *
 * Kotlin: public fun <T> MutableSharedFlow(...): MutableSharedFlow<T>
 * (lines 277-290 of SharedFlow.kt)
 */
template<typename T>
std::shared_ptr<MutableSharedFlow<T>> make_mutable_shared_flow(
    int replay = 0,
    int extra_buffer_capacity = 0,
    channels::BufferOverflow on_buffer_overflow = channels::BufferOverflow::SUSPEND
);

} // namespace kotlinx::coroutines::flow

// Include StateFlow AFTER SharedFlow is defined (StateFlow : public SharedFlow<T>)
// #include "kotlinx/coroutines/flow/StateFlow.hpp" // Removed circular include

// Include AbstractSharedFlow so SharedFlowImpl can inherit from it
#include "kotlinx/coroutines/flow/internal/AbstractSharedFlow.hpp"

namespace kotlinx::coroutines::flow {

// ------------------------------------ Implementation ------------------------------------

/**
 * Non-template base interface for SharedFlowImpl to allow type-erased slot management.
 * This provides the methods that SharedFlowSlot needs without depending on the element type T.
 */
class SharedFlowImplBase {
public:
    virtual ~SharedFlowImplBase() = default;
    virtual long long update_new_collector_index_locked() = 0;
    virtual std::vector<Continuation<Unit>*> update_collector_index_locked(long long old_index) = 0;
};

/**
 * Internal slot for SharedFlowImpl collectors.
 *
 * Kotlin: internal class SharedFlowSlot : AbstractSharedFlowSlot<SharedFlowImpl<*>>()
 * (lines 294-314 of SharedFlow.kt)
 */
class SharedFlowSlot : public internal::AbstractSharedFlowSlot<SharedFlowImplBase> {
public:
    // Kotlin: @JvmField var index = -1L
    long long index = -1LL; // current "to-be-emitted" index, -1 means the slot is free now

    // Kotlin: @JvmField var cont: Continuation<Unit>? = null
    Continuation<Unit>* cont = nullptr; // collector waiting for new value

    /**
     * Kotlin: override fun allocateLocked(flow: SharedFlowImpl<*>): Boolean
     */
    bool allocate_locked(SharedFlowImplBase* flow) override;

    /**
     * Kotlin: override fun freeLocked(flow: SharedFlowImpl<*>): Array<Continuation<Unit>?>
     */
    std::vector<Continuation<Unit>*> free_locked(SharedFlowImplBase* flow) override;
};

/**
 * Internal implementation of SharedFlow.
 *
 * Kotlin: @OptIn(ExperimentalForInheritanceCoroutinesApi::class)
 * internal open class SharedFlowImpl<T>(...) : AbstractSharedFlow<SharedFlowSlot>(),
 *     MutableSharedFlow<T>, CancellableFlow<T>, FusibleFlow<T>
 * (lines 317-725 of SharedFlow.kt)
 */
template<typename T>
class SharedFlowImpl
    : public SharedFlowImplBase,
      public internal::AbstractSharedFlow<SharedFlowSlot, SharedFlowImpl<T>>,
      public MutableSharedFlow<T>
{
private:
    // Configuration (Kotlin: constructor parameters)
    int replay_;
    int buffer_capacity_;
    channels::BufferOverflow on_buffer_overflow_;

    // Stored state
    std::vector<void*>* buffer_ = nullptr; // allocated when needed, size always power of two
    long long replay_index_ = 0LL; // minimal index from which new collector gets values
    long long min_collector_index_ = 0LL; // minimal index of active collectors
    int buffer_size_ = 0; // number of buffered values
    int queue_size_ = 0; // number of queued emitters

    // Computed state
    long long get_head() const { return std::min(min_collector_index_, replay_index_); }
    int get_replay_size() const { return static_cast<int>(get_head() + buffer_size_ - replay_index_); }
    int get_total_size() const { return buffer_size_ + queue_size_; }
    long long get_buffer_end_index() const { return get_head() + buffer_size_; }
    long long get_queue_end_index() const { return get_head() + buffer_size_ + queue_size_; }

    // Helper class for suspended emitters
    class Emitter : public ::kotlinx::coroutines::DisposableHandle {
    public:
        SharedFlowImpl<T>* flow;
        long long index;
        void* value;
        Continuation<Unit>* cont;

        Emitter(SharedFlowImpl<T>* f, long long idx, void* val, Continuation<Unit>* c)
            : flow(f), index(idx), value(val), cont(c) {}

        void dispose() override { flow->cancel_emitter(this); }
    };

    // Buffer access helpers
    void* get_buffer_at(long long index) const {
        if (!buffer_) return nullptr;
        return (*buffer_)[static_cast<size_t>(index) & (buffer_->size() - 1)];
    }

    void set_buffer_at(long long index, void* item) {
        if (!buffer_) return;
        (*buffer_)[static_cast<size_t>(index) & (buffer_->size() - 1)] = item;
    }

protected:
    /**
     * A tweak for SubscriptionCountStateFlow to get the latest value.
     *
     * Kotlin: @Suppress("UNCHECKED_CAST")
     * protected val lastReplayedLocked: T
     */
    T get_last_replayed_locked() const {
        void* item = get_buffer_at(replay_index_ + get_replay_size() - 1);
        return *static_cast<T*>(item);
    }

public:
    /**
     * Kotlin: internal open class SharedFlowImpl<T>(
     *     private val replay: Int,
     *     private val bufferCapacity: Int,
     *     private val onBufferOverflow: BufferOverflow
     * )
     */
    SharedFlowImpl(int replay, int buffer_capacity, channels::BufferOverflow on_buffer_overflow)
        : replay_(replay), buffer_capacity_(buffer_capacity), on_buffer_overflow_(on_buffer_overflow) {}

    ~SharedFlowImpl() {
        if (buffer_) {
            // Clean up boxed values in buffer
            long long head = get_head();
            for (int i = 0; i < buffer_size_; ++i) {
                void* item = get_buffer_at(head + i);
                if (item && item != get_no_value()) {
                    delete static_cast<T*>(item);
                }
            }
            delete buffer_;
        }
    }

    /**
     * Kotlin: override val replayCache: List<T>
     */
    std::vector<T> get_replay_cache() const override {
        std::lock_guard<std::recursive_mutex> lock(this->mutex());
        int replay_size = get_replay_size();
        if (replay_size == 0) return {};

        std::vector<T> result;
        result.reserve(replay_size);
        for (int i = 0; i < replay_size; ++i) {
            void* item = get_buffer_at(replay_index_ + i);
            result.push_back(*static_cast<T*>(item));
        }
        return result;
    }

    /**
     * Kotlin: @Suppress("UNCHECKED_CAST")
     * override suspend fun collect(collector: FlowCollector<T>): Nothing
     *
     * @note This is a suspend function - returns void* (COROUTINE_SUSPENDED or result)
     */
    void* collect(FlowCollector<T>* collector, Continuation<void*>* continuation) override {
        using namespace ::kotlinx::coroutines::dsl;

        SharedFlowSlot* slot = this->allocate_slot();
        try {
            // Kotlin: if (collector is SubscribedFlowCollector) collector.onSubscription()
            // NOTE: SubscribedFlowCollector is defined in Share.kt and will be ported there.

            auto ctx = continuation->get_context();
            std::shared_ptr<Job> collector_job = nullptr;
            if (ctx) {
                auto job_element = ctx->get(Job::type_key);
                collector_job = std::dynamic_pointer_cast<Job>(job_element);
            }

            while (true) {
                void* new_value = nullptr;
                while (true) {
                    new_value = try_take_value(slot);
                    if (new_value != get_no_value()) break;
                    auto awaited = suspend(await_value(slot, continuation));
                    if (awaited == COROUTINE_SUSPENDED) return COROUTINE_SUSPENDED;
                }

                if (collector_job) ensure_active(*collector_job);
                auto emitted = suspend(collector->emit(*static_cast<T*>(new_value), continuation));
                if (emitted == COROUTINE_SUSPENDED) return COROUTINE_SUSPENDED;
            }
        } catch (...) {
            this->free_slot(slot);
            throw;
        }
        this->free_slot(slot);
        return nullptr;
    }

    /**
     * Kotlin: override fun tryEmit(value: T): Boolean
     */
    bool try_emit(T value) override {
        std::vector<Continuation<Unit>*> resumes = internal::EMPTY_RESUMES;
        bool emitted = false;

        {
            std::lock_guard<std::recursive_mutex> lock(this->mutex());
            if (try_emit_locked(value)) {
                resumes = find_slots_to_resume_locked(resumes);
                emitted = true;
            }
        }

        for (auto cont : resumes) {
            if (cont) cont->resume_with(Result<Unit>::success(Unit{}));
        }
        return emitted;
    }

    /**
     * Kotlin: override suspend fun emit(value: T)
     *
     * @note This is inherited from FlowCollector<T>, which has the suspend signature
     */
    void* emit(T value, Continuation<void*>* continuation) override {
        if (try_emit(value)) return nullptr; // fast-path
        return emit_suspend(value, continuation);
    }

    /**
     * Kotlin: override fun resetReplayCache()
     */
    void reset_replay_cache() override {
        std::lock_guard<std::recursive_mutex> lock(this->mutex());
        update_buffer_locked(
            get_buffer_end_index(),
            min_collector_index_,
            get_buffer_end_index(),
            get_queue_end_index()
        );
    }

    StateFlow<int>* get_subscription_count() override {
        return this->template AbstractSharedFlow<SharedFlowSlot, SharedFlowImpl<T>>::get_subscription_count();
    }

    // AbstractSharedFlow overrides
    std::unique_ptr<SharedFlowSlot> create_slot() override {
        return std::make_unique<SharedFlowSlot>();
    }

    std::unique_ptr<std::vector<std::unique_ptr<SharedFlowSlot>>> create_slot_array(int size) override {
        auto arr = std::make_unique<std::vector<std::unique_ptr<SharedFlowSlot>>>(size);
        // All elements initialized to nullptr by default
        return arr;
    }

    SharedFlowImpl<T>* as_flow() override {
        return this;
    }

    // Internal methods for slot management (called by SharedFlowSlot)

    /**
     * Kotlin: internal fun updateNewCollectorIndexLocked(): Long
     */
    long long update_new_collector_index_locked() override {
        long long index = replay_index_;
        if (index < min_collector_index_) min_collector_index_ = index;
        return index;
    }

    /**
     * Kotlin: internal fun updateCollectorIndexLocked(oldIndex: Long): Array<Continuation<Unit>?>
     * (lines 536-603 of SharedFlow.kt)
     */
    std::vector<Continuation<Unit>*> update_collector_index_locked(long long old_index) override {
        assert(old_index >= min_collector_index_);
        if (old_index > min_collector_index_) return internal::EMPTY_RESUMES;

        long long head = get_head();
        long long new_min_collector_index = head + buffer_size_;
        if (buffer_capacity_ == 0 && queue_size_ > 0) new_min_collector_index++;

        this->for_each_slot_locked([&](SharedFlowSlot* slot) {
            if (slot->index >= 0 && slot->index < new_min_collector_index) {
                new_min_collector_index = slot->index;
            }
        });

        assert(new_min_collector_index >= min_collector_index_);
        if (new_min_collector_index <= min_collector_index_) return internal::EMPTY_RESUMES;

        long long new_buffer_end_index = get_buffer_end_index();
        int max_resume_count = 0;
        if (this->get_n_collectors() > 0) {
            int new_buffer_size0 = static_cast<int>(new_buffer_end_index - new_min_collector_index);
            max_resume_count = std::min(queue_size_, buffer_capacity_ - new_buffer_size0);
        } else {
            max_resume_count = queue_size_;
        }

        std::vector<Continuation<Unit>*> resumes = internal::EMPTY_RESUMES;
        long long new_queue_end_index = new_buffer_end_index + queue_size_;
        if (max_resume_count > 0) {
            resumes = std::vector<Continuation<Unit>*>(max_resume_count, nullptr);
            int resume_count = 0;
            for (long long cur_emitter_index = new_buffer_end_index; cur_emitter_index < new_queue_end_index; ++cur_emitter_index) {
                void* emitter_item = get_buffer_at(cur_emitter_index);
                if (emitter_item != get_no_value()) {
                    auto* emitter = static_cast<Emitter*>(emitter_item);
                    resumes[resume_count++] = emitter->cont;
                    set_buffer_at(cur_emitter_index, get_no_value());
                    set_buffer_at(new_buffer_end_index, emitter->value);
                    new_buffer_end_index++;
                    if (resume_count >= max_resume_count) break;
                }
            }
        }

        int new_buffer_size1 = static_cast<int>(new_buffer_end_index - head);
        if (this->get_n_collectors() == 0) new_min_collector_index = new_buffer_end_index;

        long long new_replay_index = std::max(
            replay_index_,
            new_buffer_end_index - std::min<long long>(replay_, new_buffer_size1)
        );

        if (buffer_capacity_ == 0 && new_replay_index < new_queue_end_index && get_buffer_at(new_replay_index) == get_no_value()) {
            new_buffer_end_index++;
            new_replay_index++;
        }

        update_buffer_locked(new_replay_index, new_min_collector_index, new_buffer_end_index, new_queue_end_index);
        cleanup_tail_locked();
        if (!resumes.empty()) resumes = find_slots_to_resume_locked(resumes);
        return resumes;
    }

private:
    // NO_VALUE symbol
    static void* get_no_value() {
        static int no_value_marker = 0;
        return &no_value_marker;
    }

    /**
     * Kotlin: @Suppress("UNCHECKED_CAST")
     * private fun tryEmitLocked(value: T): Boolean
     * (lines 424-445 of SharedFlow.kt)
     */
    bool try_emit_locked(T value) {
        if (this->get_n_collectors() == 0) {
            return try_emit_no_collectors_locked(value);
        }

        if (buffer_size_ >= buffer_capacity_ && min_collector_index_ <= replay_index_) {
            switch (on_buffer_overflow_) {
                case channels::BufferOverflow::SUSPEND:
                    return false;
                case channels::BufferOverflow::DROP_LATEST:
                    return true;
                case channels::BufferOverflow::DROP_OLDEST:
                    break;
            }
        }

        enqueue_locked(new T(value));
        buffer_size_++;

        if (buffer_size_ > buffer_capacity_) {
            drop_oldest_locked();
        }

        if (get_replay_size() > replay_) {
            update_buffer_locked(replay_index_ + 1, min_collector_index_, get_buffer_end_index(), get_queue_end_index());
        }

        return true;
    }

    /**
     * Kotlin: private fun tryEmitNoCollectorsLocked(value: T): Boolean
     */
    bool try_emit_no_collectors_locked(T value) {
        assert(this->get_n_collectors() == 0);
        if (replay_ == 0) return true;

        enqueue_locked(new T(value));
        buffer_size_++;

        if (buffer_size_ > replay_) {
            drop_oldest_locked();
        }

        min_collector_index_ = get_head() + buffer_size_;
        return true;
    }

    /**
     * Kotlin: private fun dropOldestLocked()
     */
    void drop_oldest_locked() {
        long long head = get_head();
        set_buffer_at(head, nullptr);
        buffer_size_--;

        long long new_head = head + 1;
        if (replay_index_ < new_head) replay_index_ = new_head;
        if (min_collector_index_ < new_head) {
            correct_collector_indexes_on_drop_oldest(new_head);
        }

        assert(get_head() == new_head);
    }

    /**
     * Kotlin: private fun correctCollectorIndexesOnDropOldest(newHead: Long)
     */
    void correct_collector_indexes_on_drop_oldest(long long new_head) {
        this->for_each_slot_locked([&](SharedFlowSlot* slot) {
            if (slot->index >= 0 && slot->index < new_head) {
                slot->index = new_head;
            }
        });
        min_collector_index_ = new_head;
    }

    /**
     * Kotlin: private fun enqueueLocked(item: Any?)
     */
    void enqueue_locked(void* item) {
        int cur_size = get_total_size();
        std::vector<void*>* buf = buffer_;

        if (!buf) {
            buf = grow_buffer(nullptr, 0, 2);
        } else if (cur_size >= static_cast<int>(buf->size())) {
            buf = grow_buffer(buf, cur_size, buf->size() * 2);
        }

        set_buffer_at(get_head() + cur_size, item);
    }

    /**
     * Kotlin: private fun growBuffer(curBuffer: Array<Any?>?, curSize: Int, newSize: Int): Array<Any?>
     */
    std::vector<void*>* grow_buffer(std::vector<void*>* cur_buffer, int cur_size, int new_size) {
        if (new_size <= 0) {
            throw std::runtime_error("Buffer size overflow");
        }

        auto* new_buffer = new std::vector<void*>(new_size, nullptr);
        buffer_ = new_buffer;

        if (!cur_buffer) return new_buffer;

        long long head = get_head();
        for (int i = 0; i < cur_size; ++i) {
            (*new_buffer)[static_cast<size_t>((head + i)) & (new_size - 1)] =
                (*cur_buffer)[static_cast<size_t>((head + i)) & (cur_buffer->size() - 1)];
        }

        delete cur_buffer;
        return new_buffer;
    }

    /**
     * Kotlin: private fun updateBufferLocked(...)
     */
    void update_buffer_locked(
        long long new_replay_index,
        long long new_min_collector_index,
        long long new_buffer_end_index,
        long long new_queue_end_index
    ) {
        long long new_head = std::min(new_min_collector_index, new_replay_index);
        assert(new_head >= get_head());

        for (long long index = get_head(); index < new_head; ++index) {
            set_buffer_at(index, nullptr);
        }

        replay_index_ = new_replay_index;
        min_collector_index_ = new_min_collector_index;
        buffer_size_ = static_cast<int>(new_buffer_end_index - new_head);
        queue_size_ = static_cast<int>(new_queue_end_index - new_buffer_end_index);

        assert(buffer_size_ >= 0);
        assert(queue_size_ >= 0);
        assert(replay_index_ <= get_head() + buffer_size_);
    }

    /**
     * Kotlin: private fun tryTakeValue(slot: SharedFlowSlot): Any?
     */
    void* try_take_value(SharedFlowSlot* slot) {
        std::vector<Continuation<Unit>*> resumes = internal::EMPTY_RESUMES;
        void* value = nullptr;

        {
            std::lock_guard<std::recursive_mutex> lock(this->mutex());
            long long index = try_peek_locked(slot);
            if (index < 0) {
                value = get_no_value();
            } else {
                long long old_index = slot->index;
                value = get_peeked_value_locked_at(index);
                slot->index = index + 1;
                resumes = update_collector_index_locked(old_index);
            }
        }

        for (auto resume : resumes) {
            if (resume) resume->resume_with(Result<Unit>::success(Unit{}));
        }

        return value;
    }

    /**
     * Kotlin: private fun tryPeekLocked(slot: SharedFlowSlot): Long
     */
    long long try_peek_locked(SharedFlowSlot* slot) const {
        long long index = slot->index;
        if (index < get_buffer_end_index()) return index;
        if (buffer_capacity_ > 0) return -1LL;
        if (index > get_head()) return -1LL;
        if (queue_size_ == 0) return -1LL;
        return index;
    }

    /**
     * Kotlin: private fun getPeekedValueLockedAt(index: Long): Any?
     */
    void* get_peeked_value_locked_at(long long index) const {
        void* item = get_buffer_at(index);
        if (item == get_no_value()) return item;
        if (index >= get_buffer_end_index()) {
            auto* emitter = static_cast<Emitter*>(item);
            return emitter->value;
        }
        return item;
    }

    /**
     * Kotlin: private fun findSlotsToResumeLocked(resumesIn: Array<Continuation<Unit>?>): Array<Continuation<Unit>?>
     */
    std::vector<Continuation<Unit>*> find_slots_to_resume_locked(std::vector<Continuation<Unit>*> resumes_in) {
        std::vector<Continuation<Unit>*> resumes = resumes_in;
        size_t resume_count = resumes_in.size();

        this->for_each_slot_locked([&](SharedFlowSlot* slot) {
            Continuation<Unit>* cont = slot->cont;
            if (!cont) return;
            if (try_peek_locked(slot) < 0) return;

            if (resume_count >= resumes.size()) {
                resumes.resize(std::max(size_t(2), 2 * resumes.size()), nullptr);
            }
            resumes[resume_count++] = cont;
            slot->cont = nullptr;
        });

        return resumes;
    }

    /**
     * Kotlin: private fun cancelEmitter(emitter: Emitter)
     */
    void cancel_emitter(Emitter* emitter) {
        std::lock_guard<std::recursive_mutex> lock(this->mutex());
        if (emitter->index < get_head()) return;
        if (!buffer_) return;

        void* item = get_buffer_at(emitter->index);
        if (item != emitter) return;

        set_buffer_at(emitter->index, get_no_value());
        cleanup_tail_locked();
    }

    /**
     * Kotlin: private fun cleanupTailLocked()
     */
    void cleanup_tail_locked() {
        if (buffer_capacity_ == 0 && queue_size_ <= 1) return;
        if (!buffer_) return;

        while (queue_size_ > 0 && get_buffer_at(get_head() + get_total_size() - 1) == get_no_value()) {
            queue_size_--;
            set_buffer_at(get_head() + get_total_size(), nullptr);
        }
    }

    /**
     * Kotlin: private suspend fun awaitValue(slot: SharedFlowSlot): Unit
     */
    void* await_value(SharedFlowSlot* slot, Continuation<void*>* continuation) {
        return suspend_cancellable_coroutine<Unit>(
            [this, slot](CancellableContinuation<Unit>& cont) {
                std::lock_guard<std::recursive_mutex> lock(this->mutex());
                long long index = try_peek_locked(slot);
                if (index < 0) {
                    slot->cont = static_cast<Continuation<Unit>*>(&cont);
                } else {
                    cont.resume_with(Result<Unit>::success(Unit{}));
                    return;
                }
                slot->cont = static_cast<Continuation<Unit>*>(&cont);
            },
            continuation);
    }

    /**
     * Kotlin: private suspend fun emitSuspend(value: T)
     */
    void* emit_suspend(T value, Continuation<void*>* continuation) {
        return suspend_cancellable_coroutine<Unit>(
            [this, value](CancellableContinuation<Unit>& cont) mutable {
                std::vector<Continuation<Unit>*> resumes = internal::EMPTY_RESUMES;
                Emitter* emitter = nullptr;

                {
                    std::lock_guard<std::recursive_mutex> lock(this->mutex());
                    if (try_emit_locked(value)) {
                        cont.resume_with(Result<Unit>::success(Unit{}));
                        resumes = find_slots_to_resume_locked(resumes);
                    } else {
                        auto* boxed_value = new T(value);
                        emitter = new Emitter(this, get_head() + get_total_size(), boxed_value, static_cast<Continuation<Unit>*>(&cont));
                        enqueue_locked(emitter);
                        queue_size_++;
                        if (buffer_capacity_ == 0) {
                            resumes = find_slots_to_resume_locked(resumes);
                        }
                    }
                }

                if (emitter) {
                    dispose_on_cancellation(cont, emitter);
                }

                for (auto r : resumes) {
                    if (r) r->resume_with(Result<Unit>::success(Unit{}));
                }
            },
            continuation);
    }
};

// SharedFlowSlot implementation (depends on SharedFlowImpl)
inline bool SharedFlowSlot::allocate_locked(SharedFlowImplBase* flow) {
    if (index >= 0) return false;
    index = flow->update_new_collector_index_locked();
    return true;
}

inline std::vector<Continuation<Unit>*> SharedFlowSlot::free_locked(SharedFlowImplBase* flow) {
    assert(index >= 0);
    long long old_index = index;
    index = -1LL;
    cont = nullptr;
    return flow->update_collector_index_locked(old_index);
}

// Factory function implementation
template<typename T>
std::shared_ptr<MutableSharedFlow<T>> make_mutable_shared_flow(
    int replay,
    int extra_buffer_capacity,
    channels::BufferOverflow on_buffer_overflow
) {
    if (replay < 0) {
        throw std::invalid_argument("replay cannot be negative, but was " + std::to_string(replay));
    }
    if (extra_buffer_capacity < 0) {
        throw std::invalid_argument("extraBufferCapacity cannot be negative, but was " + std::to_string(extra_buffer_capacity));
    }
    if (replay == 0 && extra_buffer_capacity == 0 && on_buffer_overflow != channels::BufferOverflow::SUSPEND) {
        throw std::invalid_argument(
            "replay or extraBufferCapacity must be positive with non-default onBufferOverflow strategy"
        );
    }

    int buffer_capacity_0 = replay + extra_buffer_capacity;
    int buffer_capacity = (buffer_capacity_0 < 0) ? std::numeric_limits<int>::max() : buffer_capacity_0;

    return std::make_shared<SharedFlowImpl<T>>(replay, buffer_capacity, on_buffer_overflow);
}

} // namespace kotlinx::coroutines::flow

// SubscriptionCountStateFlow and AbstractSharedFlow implementations moved to StateFlow.hpp
