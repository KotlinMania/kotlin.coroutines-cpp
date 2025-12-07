// Transliterated from: reactive/kotlinx-coroutines-rx3/src/RxScheduler.cpp

namespace kotlinx {
namespace coroutines {
namespace rx3 {

// TODO: #include equivalent
// import io.reactivex.rxjava3.core.*
// import io.reactivex.rxjava3.disposables.*
// import io.reactivex.rxjava3.plugins.*
// import kotlinx.atomicfu.*
// import kotlinx.coroutines.*
// import kotlinx.coroutines.channels.*
// import java.util.concurrent.*
// import kotlin.coroutines.*

/**
 * Converts an instance of [Scheduler] to an implementation of [CoroutineDispatcher]
 * and provides native support of [delay] and [withTimeout].
 */
CoroutineDispatcher* as_coroutine_dispatcher(Scheduler* scheduler) {
    // fun Scheduler.asCoroutineDispatcher(): CoroutineDispatcher =
    //     if (this is DispatcherScheduler) {
    //         dispatcher
    //     } else {
    //         SchedulerCoroutineDispatcher(this)
    //     }

    // TODO: Check if scheduler is DispatcherScheduler
    // TODO: Return wrapped or existing dispatcher
    return nullptr;
}

// @Deprecated(level = DeprecationLevel.HIDDEN, message = "Since 1.4.2, binary compatibility with earlier versions")
// @JvmName("asCoroutineDispatcher")
[[deprecated("Since 1.4.2, binary compatibility with earlier versions")]]
SchedulerCoroutineDispatcher* as_coroutine_dispatcher0(Scheduler* scheduler) {
    return new SchedulerCoroutineDispatcher(scheduler);
}

/**
 * Converts an instance of [CoroutineDispatcher] to an implementation of [Scheduler].
 */
Scheduler* as_scheduler(CoroutineDispatcher* dispatcher) {
    // fun CoroutineDispatcher.asScheduler(): Scheduler =
    //     if (this is SchedulerCoroutineDispatcher) {
    //         scheduler
    //     } else {
    //         DispatcherScheduler(this)
    //     }

    // TODO: Check if dispatcher is SchedulerCoroutineDispatcher
    // TODO: Return wrapped or existing scheduler
    return nullptr;
}

class DispatcherScheduler : public Scheduler {
private:
    CoroutineDispatcher* dispatcher_;
    Job* scheduler_job_;
    CoroutineScope* scope_;
    std::atomic<int64_t> worker_counter_;

public:
    // @JvmField
    explicit DispatcherScheduler(CoroutineDispatcher* dispatcher)
        : dispatcher_(dispatcher),
          scheduler_job_(new SupervisorJob()),
          scope_(nullptr),
          worker_counter_(1) {
        // val scope = CoroutineScope(schedulerJob + dispatcher)
        // TODO: Create scope with job + dispatcher
    }

    Disposable* schedule_direct(Runnable* block, int64_t delay, TimeUnit unit) override {
        // TODO: Implement scheduleTask with scope
        return nullptr;
    }

    Worker* create_worker() override {
        // override fun createWorker(): Worker = DispatcherWorker(workerCounter.getAndIncrement(), dispatcher, schedulerJob)
        return new DispatcherWorker(worker_counter_.fetch_add(1), dispatcher_, scheduler_job_);
    }

    void shutdown() override {
        // override fun shutdown() {
        //     schedulerJob.cancel()
        // }
        if (scheduler_job_) {
            scheduler_job_->cancel();
        }
    }

    std::string to_string() const override {
        // override fun toString(): String = dispatcher.toString()
        return "DispatcherScheduler";
    }

    class DispatcherWorker : public Worker {
    private:
        int64_t counter_;
        CoroutineDispatcher* dispatcher_;
        Job* worker_job_;
        CoroutineScope* worker_scope_;
        Channel<std::function<void()>>* block_channel_;

    public:
        DispatcherWorker(int64_t counter, CoroutineDispatcher* dispatcher, Job* parent_job)
            : counter_(counter),
              dispatcher_(dispatcher),
              worker_job_(new SupervisorJob(parent_job)),
              worker_scope_(nullptr),
              block_channel_(new Channel<std::function<void()>>(Channel::kUnlimited)) {
            // TODO: Initialize worker_scope = CoroutineScope(workerJob + dispatcher)
            // TODO: Launch consumption coroutine
        }

        Disposable* schedule(Runnable* block, int64_t delay, TimeUnit unit) override {
            // TODO: Implement scheduleTask with workerScope
            return nullptr;
        }

        bool is_disposed() const override {
            // override fun isDisposed(): Boolean = !workerScope.isActive
            return worker_scope_ ? !worker_scope_->is_active() : true;
        }

        void dispose() override {
            // override fun dispose() {
            //     blockChannel.close()
            //     workerJob.cancel()
            // }
            if (block_channel_) block_channel_->close();
            if (worker_job_) worker_job_->cancel();
        }

        std::string to_string() const override {
            return "DispatcherWorker";
        }
    };
};

using Task = std::function<void()>;

/**
 * Schedule [block] so that an adapted version of it, wrapped in [adaptForScheduling], executes after [delayMillis]
 * milliseconds.
 */
Disposable* schedule_task(
    CoroutineScope* scope,
    Runnable* block,
    int64_t delay_millis,
    std::function<Runnable*(Task)> adapt_for_scheduling
) {
    // TODO: Implement scheduling logic with delay
    // TODO: Handle RxJavaPlugins.onSchedule
    // TODO: Use runInterruptible
    return nullptr;
}

/**
 * Implements [CoroutineDispatcher] on top of an arbitrary [Scheduler].
 */
class SchedulerCoroutineDispatcher : public CoroutineDispatcher, public Delay {
private:
    Scheduler* scheduler_;

public:
    /**
     * Underlying scheduler of current [CoroutineDispatcher].
     */
    explicit SchedulerCoroutineDispatcher(Scheduler* scheduler)
        : scheduler_(scheduler) {}

    Scheduler* get_scheduler() const { return scheduler_; }

    void dispatch(const CoroutineContext& context, Runnable* block) override {
        // override fun dispatch(context: CoroutineContext, block: Runnable) {
        //     scheduler.scheduleDirect(block)
        // }
        scheduler_->schedule_direct(block, 0, TimeUnit::kMilliseconds);
    }

    void schedule_resume_after_delay(int64_t time_millis, CancellableContinuation<void>* continuation) override {
        // TODO: Implement delay with scheduler
    }

    DisposableHandle* invoke_on_timeout(int64_t time_millis, Runnable* block, const CoroutineContext& context) override {
        // TODO: Implement timeout with scheduler
        return nullptr;
    }

    std::string to_string() const override {
        return scheduler_->to_string();
    }

    bool equals(const SchedulerCoroutineDispatcher& other) const {
        // override fun equals(other: Any?): Boolean = other is SchedulerCoroutineDispatcher && other.scheduler === scheduler
        return scheduler_ == other.scheduler_;
    }

    size_t hash_code() const {
        // override fun hashCode(): Int = System.identityHashCode(scheduler)
        return reinterpret_cast<size_t>(scheduler_);
    }
};

} // namespace rx3
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO: Semantic Implementation Tasks
 *
 * 1. Implement Scheduler and Worker from RxJava3
 * 2. Implement CoroutineDispatcher and Delay interfaces
 * 3. Implement SupervisorJob and CoroutineScope
 * 4. Implement Channel<T> with UNLIMITED capacity
 * 5. Implement Disposable and DisposableHandle
 * 6. Implement TimeUnit enum
 * 7. Implement RxJavaPlugins.onSchedule
 * 8. Implement runInterruptible
 * 9. Implement delay and timeout mechanisms
 * 10. Add unit tests
 */
