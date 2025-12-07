#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/LimitedDispatcher.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: atomicfu needs C++ atomic equivalent
// TODO: CoroutineDispatcher, CoroutineContext, Delay need C++ equivalents
// TODO: LockFreeTaskQueue needs implementation
// TODO: SynchronizedObject and synchronized() need platform-specific implementation
// TODO: Runnable struct needs C++ equivalent
// TODO: @InternalCoroutinesApi annotation - translate to comment

#include <atomic>
#include <string>

namespace kotlinx {
namespace coroutines {
namespace {

// Forward declarations
class CoroutineDispatcher;
class CoroutineContext;
class Delay;
class DefaultDelay;
class Runnable;
template<typename T> class LockFreeTaskQueue;
class SynchronizedObject;

/**
 * The result of .limitedParallelism(x) call, a dispatcher
 * that wraps the given dispatcher, but limits the parallelism level, while
 * trying to emulate fairness.
 *
 * ### Implementation details
 *
 * By design, 'LimitedDispatcher' never [dispatches][CoroutineDispatcher.dispatch] originally sent tasks
 * to the underlying dispatcher. Instead, it maintains its own queue of tasks sent to this dispatcher and
 * dispatches at most [parallelism] "worker-loop" tasks that poll the underlying queue and cooperatively preempt
 * in order to avoid starvation of the underlying dispatcher.
 *
 * Such behavior is crucial to be compatible with any underlying dispatcher implementation without
 * direct cooperation.
 */
class LimitedDispatcher : CoroutineDispatcher /* , Delay */ {
private:
    CoroutineDispatcher* dispatcher_;
    int parallelism_;
    std::string* name_;

    // Atomic is necessary here for the sake of K/N memory ordering,
    // there is no need in atomic operations for this property
    std::atomic<int> running_workers_;

    LockFreeTaskQueue<Runnable*>* queue_;

    // A separate class that we can synchronize on for K/N
    SynchronizedObject* worker_allocation_lock_;

public:
    LimitedDispatcher(CoroutineDispatcher* dispatcher, int parallelism, std::string* name)
        : dispatcher_(dispatcher),
          parallelism_(parallelism),
          name_(name),
          running_workers_(0),
          queue_(nullptr), // TODO: new LockFreeTaskQueue<Runnable*>(false)
          worker_allocation_lock_(nullptr) {} // TODO: new SynchronizedObject()

    CoroutineDispatcher* limited_parallelism(int parallelism, std::string* name) override {
        // TODO: parallelism.check_parallelism()
        if (parallelism >= this->parallelism_) return named_or_this(name);
        // TODO: return super.limited_parallelism(parallelism, name)
        return nullptr;
    }

    void dispatch(CoroutineContext* context, Runnable* block) override {
        dispatch_internal(block, [this](Worker* worker) {
            // TODO: dispatcher_.safe_dispatch(this, worker)
        });
    }

    // TODO: @InternalCoroutinesApi annotation
    void dispatch_yield(CoroutineContext* context, Runnable* block) override {
        dispatch_internal(block, [this](Worker* worker) {
            // TODO: dispatcher_.dispatch_yield(this, worker)
        });
    }

    /**
     * Tries to dispatch the given [block].
     * If there are not enough workers, it starts a new one via [startWorker].
     */
    template<typename StartWorker>
    void dispatch_internal(Runnable* block, StartWorker start_worker) {
        // Add task to queue so running workers will be able to see that
        // TODO: queue_->add_last(block)
        if (running_workers_.load() >= parallelism_) return;
        // allocation may fail if some workers were launched in parallel or a worker temporarily decreased
        // `runningWorkers` when they observed an empty queue.
        if (!try_allocate_worker()) return;
        Runnable* task = obtain_task_or_deallocate_worker();
        if (task == nullptr) return;
        try {
            // TODO: start_worker(new Worker(task))
        } catch (...) {
            /* If we failed to start a worker, we should decrement the counter.
            The queue is in an inconsistent state--it's non-empty despite the target parallelism not having been
            reached--but at least a properly functioning worker will have a chance to correct this if some future
            dispatch does succeed.
            If we don't decrement the counter, it will be impossible to ever reach the target parallelism again. */
            running_workers_.fetch_sub(1);
            throw;
        }
    }

    /**
     * Tries to obtain the permit to start a new worker.
     */
    bool try_allocate_worker() {
        // TODO: synchronized(worker_allocation_lock_) {
        //     if (running_workers_.load() >= parallelism_) return false;
        //     running_workers_.fetch_add(1);
        //     return true;
        // }
        return false;
    }

    /**
     * Obtains the next task from the queue, or logically deallocates the worker if the queue is empty.
     */
    Runnable* obtain_task_or_deallocate_worker() {
        while (true) {
            // TODO: Runnable* next_task = queue_->remove_first_or_null();
            Runnable* next_task = nullptr;
            if (next_task == nullptr) {
                // TODO: synchronized(worker_allocation_lock_) {
                //     running_workers_.fetch_sub(1);
                //     if (queue_->size() == 0) return nullptr;
                //     running_workers_.fetch_add(1);
                // }
            } else {
                return next_task;
            }
        }
    }

    std::string to_string() override {
        if (name_ != nullptr) return *name_;
        // TODO: return dispatcher_->to_string() + ".limitedParallelism(" + parallelism_ + ")"
        return "";
    }

    /**
     * A worker that polls the queue and runs tasks until there are no more of them.
     *
     * It always stores the next task to run. This is done in order to prevent the possibility of the fairness
     * re-dispatch happening when there are no more tasks in the queue. This is important because, after all the
     * actual tasks are done, nothing prevents the user from closing the dispatcher and making it incorrect to
     * perform any more dispatches.
     */
    class Worker : Runnable {
    private:
        LimitedDispatcher* outer_;
        Runnable* current_task_;

    public:
        Worker(LimitedDispatcher* outer, Runnable* current_task)
            : outer_(outer), current_task_(current_task) {}

        void run() override {
            try {
                int fairness_counter = 0;
                while (true) {
                    try {
                        current_task_->run();
                    } catch (...) {
                        // TODO: handle_coroutine_exception(EmptyCoroutineContext, e)
                    }
                    current_task_ = outer_->obtain_task_or_deallocate_worker();
                    if (current_task_ == nullptr) return;
                    // 16 is our out-of-thin-air constant to emulate fairness. Used in JS dispatchers as well
                    if (++fairness_counter >= 16 /* && dispatcher_.safe_is_dispatch_needed(this) */) {
                        // Do "yield" to let other views execute their runnable as well
                        // Note that we do not decrement 'runningWorkers' as we are still committed to our part of work
                        // TODO: dispatcher_.safe_dispatch(outer_, this)
                        return;
                    }
                }
            } catch (...) {
                // If the worker failed, we should deallocate its slot
                // TODO: synchronized(worker_allocation_lock_) {
                //     running_workers_.fetch_sub(1);
                // }
                throw;
            }
        }
    };

private:
    CoroutineDispatcher* named_or_this(std::string* name) {
        // TODO: if (name != nullptr) return new NamedDispatcher(this, *name);
        return this;
    }
};

inline void check_parallelism(int parallelism) {
    // TODO: require(parallelism >= 1) { "Expected positive parallelism level, but got " + parallelism }
}

inline CoroutineDispatcher* named_or_this(CoroutineDispatcher* dispatcher, std::string* name) {
    // TODO: if (name != nullptr) return new NamedDispatcher(dispatcher, *name);
    return dispatcher;
}

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
