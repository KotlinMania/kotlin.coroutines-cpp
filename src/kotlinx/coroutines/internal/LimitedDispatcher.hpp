#pragma once
// port-lint: source internal/LimitedDispatcher.kt
/**
 * @file LimitedDispatcher.hpp
 * @brief Limited parallelism dispatcher
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/LimitedDispatcher.kt
 *
 * The result of .limitedParallelism(x) call, a dispatcher
 * that wraps the given dispatcher, but limits the parallelism level, while
 * trying to emulate fairness.
 *
 * ### Implementation details
 *
 * By design, 'LimitedDispatcher' never dispatches originally sent tasks
 * to the underlying dispatcher. Instead, it maintains its own queue of tasks sent to this dispatcher and
 * dispatches at most `parallelism` "worker-loop" tasks that poll the underlying queue and cooperatively preempt
 * in order to avoid starvation of the underlying dispatcher.
 *
 * Such behavior is crucial to be compatible with any underlying dispatcher implementation without
 * direct cooperation.
 */

#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/internal/LockFreeTaskQueue.hpp"
#include "kotlinx/coroutines/internal/SynchronizedObject.hpp"
#include <atomic>
#include <memory>
#include <string>
#include <stdexcept>

namespace kotlinx {
namespace coroutines {
namespace internal {

/**
 * Checks that parallelism is at least 1.
 *
 * Transliterated from:
 * internal fun Int.checkParallelism() = require(this >= 1) { "Expected positive parallelism level, but got $this" }
 */
inline void check_parallelism(int parallelism) {
    if (parallelism < 1) {
        throw std::invalid_argument("Expected positive parallelism level, but got " + std::to_string(parallelism));
    }
}

/**
 * The result of .limitedParallelism(x) call, a dispatcher that wraps the given dispatcher,
 * but limits the parallelism level, while trying to emulate fairness.
 *
 * Transliterated from:
 * internal class LimitedDispatcher(
 *     private val dispatcher: CoroutineDispatcher,
 *     private val parallelism: Int,
 *     private val name: String?
 * ) : CoroutineDispatcher(), Delay by (dispatcher as? Delay ?: DefaultDelay)
 */
class LimitedDispatcher : public CoroutineDispatcher {
public:
    LimitedDispatcher(
        std::shared_ptr<CoroutineDispatcher> dispatcher,
        int parallelism,
        std::string name = ""
    ) : dispatcher_(std::move(dispatcher)),
        parallelism_(parallelism),
        name_(std::move(name)),
        running_workers_(0) {
        check_parallelism(parallelism);
    }

    /**
     * Transliterated from:
     * override fun limitedParallelism(parallelism: Int, name: String?): CoroutineDispatcher {
     *     parallelism.checkParallelism()
     *     if (parallelism >= this.parallelism) return namedOrThis(name)
     *     return super.limitedParallelism(parallelism, name)
     * }
     */
    std::shared_ptr<CoroutineDispatcher> limited_parallelism(int parallelism, const std::string& name = "") override {
        check_parallelism(parallelism);
        if (parallelism >= parallelism_) {
            return named_or_this(name);
        }
        return CoroutineDispatcher::limited_parallelism(parallelism, name);
    }

    /**
     * Transliterated from:
     * override fun dispatch(context: CoroutineContext, block: Runnable) {
     *     dispatchInternal(block) { worker ->
     *         dispatcher.safeDispatch(this, worker)
     *     }
     * }
     */
    void dispatch(std::shared_ptr<CoroutineContext> context, std::shared_ptr<Runnable> block) override {
        dispatch_internal(block, [this](std::shared_ptr<Runnable> worker) {
            dispatcher_->dispatch(nullptr, worker);
        });
    }

    /**
     * Transliterated from:
     * override fun dispatchYield(context: CoroutineContext, block: Runnable) {
     *     dispatchInternal(block) { worker ->
     *         dispatcher.dispatchYield(this, worker)
     *     }
     * }
     */
    void dispatch_yield(std::shared_ptr<CoroutineContext> context, std::shared_ptr<Runnable> block) {
        dispatch_internal(block, [this](std::shared_ptr<Runnable> worker) {
            dispatcher_->dispatch_yield(nullptr, worker);
        });
    }

    /**
     * Transliterated from:
     * override fun toString() = name ?: "$dispatcher.limitedParallelism($parallelism)"
     */
    std::string to_string() const {
        if (!name_.empty()) {
            return name_;
        }
        return dispatcher_->to_string() + ".limitedParallelism(" + std::to_string(parallelism_) + ")";
    }

private:
    std::shared_ptr<CoroutineDispatcher> dispatcher_;
    int parallelism_;
    std::string name_;

    // Atomic is necessary here for the sake of K/N memory ordering,
    // there is no need in atomic operations for this property
    std::atomic<int> running_workers_;

    LockFreeTaskQueue<std::shared_ptr<Runnable>> queue_{false};  // singleConsumer = false

    // A separate object that we can synchronize on for K/N
    SynchronizedObject worker_allocation_lock_;

    /**
     * Returns this dispatcher with a name, or this if no name is provided.
     */
    std::shared_ptr<CoroutineDispatcher> named_or_this(const std::string& name) {
        if (!name.empty()) {
            return std::make_shared<LimitedDispatcher>(dispatcher_, parallelism_, name);
        }
        return std::shared_ptr<CoroutineDispatcher>(this, [](CoroutineDispatcher*){});
    }

    /**
     * Tries to dispatch the given [block].
     * If there are not enough workers, it starts a new one via [startWorker].
     *
     * Transliterated from:
     * private inline fun dispatchInternal(block: Runnable, startWorker: (Worker) -> Unit)
     */
    template<typename StartWorkerFunc>
    void dispatch_internal(std::shared_ptr<Runnable> block, StartWorkerFunc start_worker) {
        // Add task to queue so running workers will be able to see that
        queue_.add_last(block);
        if (running_workers_.load() >= parallelism_) return;
        // allocation may fail if some workers were launched in parallel or a worker temporarily decreased
        // `runningWorkers` when they observed an empty queue.
        if (!try_allocate_worker()) return;
        auto task = obtain_task_or_deallocate_worker();
        if (!task) return;
        try {
            start_worker(std::make_shared<Worker>(this, task));
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
     *
     * Transliterated from:
     * private fun tryAllocateWorker(): Boolean {
     *     synchronized(workerAllocationLock) {
     *         if (runningWorkers.value >= parallelism) return false
     *         runningWorkers.incrementAndGet()
     *         return true
     *     }
     * }
     */
    bool try_allocate_worker() {
        std::lock_guard<SynchronizedObject> lock(worker_allocation_lock_);
        if (running_workers_.load() >= parallelism_) return false;
        running_workers_.fetch_add(1);
        return true;
    }

    /**
     * Obtains the next task from the queue, or logically deallocates the worker if the queue is empty.
     *
     * Transliterated from:
     * private fun obtainTaskOrDeallocateWorker(): Runnable? {
     *     while (true) {
     *         when (val nextTask = queue.removeFirstOrNull()) {
     *             null -> synchronized(workerAllocationLock) {
     *                 runningWorkers.decrementAndGet()
     *                 if (queue.size == 0) return null
     *                 runningWorkers.incrementAndGet()
     *             }
     *             else -> return nextTask
     *         }
     *     }
     * }
     */
    std::shared_ptr<Runnable> obtain_task_or_deallocate_worker() {
        while (true) {
            auto next_task = queue_.remove_first_or_null();
            if (!next_task) {
                std::lock_guard<SynchronizedObject> lock(worker_allocation_lock_);
                running_workers_.fetch_sub(1);
                if (queue_.size() == 0) return nullptr;
                running_workers_.fetch_add(1);
            } else {
                return next_task;
            }
        }
    }

    /**
     * A worker that polls the queue and runs tasks until there are no more of them.
     *
     * It always stores the next task to run. This is done in order to prevent the possibility of the fairness
     * re-dispatch happening when there are no more tasks in the queue. This is important because, after all the
     * actual tasks are done, nothing prevents the user from closing the dispatcher and making it incorrect to
     * perform any more dispatches.
     *
     * Transliterated from:
     * private inner class Worker(private var currentTask: Runnable) : Runnable
     */
    class Worker : public Runnable {
    public:
        Worker(LimitedDispatcher* parent, std::shared_ptr<Runnable> current_task)
            : parent_(parent), current_task_(std::move(current_task)) {}

        /**
         * Transliterated from:
         * override fun run() {
         *     try {
         *         var fairnessCounter = 0
         *         while (true) {
         *             try {
         *                 currentTask.run()
         *             } catch (e: Throwable) {
         *                 handleCoroutineException(EmptyCoroutineContext, e)
         *             }
         *             currentTask = obtainTaskOrDeallocateWorker() ?: return
         *             // 16 is our out-of-thin-air constant to emulate fairness. Used in JS dispatchers as well
         *             if (++fairnessCounter >= 16 && dispatcher.safeIsDispatchNeeded(this@LimitedDispatcher)) {
         *                 // Do "yield" to let other views execute their runnable as well
         *                 // Note that we do not decrement 'runningWorkers' as we are still committed to our part of work
         *                 dispatcher.safeDispatch(this@LimitedDispatcher, this)
         *                 return
         *             }
         *         }
         *     } catch (e: Throwable) {
         *         // If the worker failed, we should deallocate its slot
         *         synchronized(workerAllocationLock) {
         *             runningWorkers.decrementAndGet()
         *         }
         *         throw e
         *     }
         * }
         */
        void run() override {
            try {
                int fairness_counter = 0;
                while (true) {
                    try {
                        current_task_->run();
                    } catch (...) {
                        // handleCoroutineException(EmptyCoroutineContext, e)
                        // For now, just swallow - proper handling requires exception handler infrastructure
                    }
                    current_task_ = parent_->obtain_task_or_deallocate_worker();
                    if (!current_task_) return;
                    // 16 is our out-of-thin-air constant to emulate fairness. Used in JS dispatchers as well
                    if (++fairness_counter >= 16 && parent_->dispatcher_->is_dispatch_needed(nullptr)) {
                        // Do "yield" to let other views execute their runnable as well
                        // Note that we do not decrement 'runningWorkers' as we are still committed to our part of work
                        parent_->dispatcher_->dispatch(nullptr, std::shared_ptr<Runnable>(this, [](Runnable*){}));
                        return;
                    }
                }
            } catch (...) {
                // If the worker failed, we should deallocate its slot
                std::lock_guard<SynchronizedObject> lock(parent_->worker_allocation_lock_);
                parent_->running_workers_.fetch_sub(1);
                throw;
            }
        }

    private:
        LimitedDispatcher* parent_;
        std::shared_ptr<Runnable> current_task_;
    };
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
