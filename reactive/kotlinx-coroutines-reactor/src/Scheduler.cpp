// Transliterated from: reactive/kotlinx-coroutines-reactor/src/Scheduler.cpp

// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <reactor/core/disposable.hpp>
// TODO: #include <reactor/core/scheduler.hpp>
// TODO: #include <java/util/concurrent/time_unit.hpp>
// TODO: #include <kotlin/coroutines/coroutine_context.hpp>

namespace kotlinx { namespace coroutines { namespace reactor {

/**
 * Converts an instance of [Scheduler] to an implementation of [CoroutineDispatcher].
 */
SchedulerCoroutineDispatcher as_coroutine_dispatcher(Scheduler* scheduler) {
    return SchedulerCoroutineDispatcher(scheduler);
}

/**
 * Implements [CoroutineDispatcher] on top of an arbitrary [Scheduler].
 * @param scheduler a scheduler.
 */
class SchedulerCoroutineDispatcher : public CoroutineDispatcher, public Delay {
public:
    /**
     * Underlying scheduler of current [CoroutineDispatcher].
     */
    Scheduler* scheduler;

    SchedulerCoroutineDispatcher(Scheduler* scheduler_param) : scheduler(scheduler_param) {
    }

    /** @suppress */
    void dispatch(CoroutineContext context, Runnable* block) override {
        scheduler->schedule(block);
    }

    /** @suppress */
    void schedule_resume_after_delay(int64_t time_millis, CancellableContinuation<Unit>* continuation) override {
        Disposable* disposable = scheduler->schedule([continuation]() {
            continuation->resume_undispatched(Unit{});
        }, time_millis, TimeUnit::kMilliseconds);
        continuation->dispose_on_cancellation(disposable->as_disposable_handle());
    }

    /** @suppress */
    DisposableHandle* invoke_on_timeout(int64_t time_millis, Runnable* block, CoroutineContext context) override {
        return scheduler->schedule(block, time_millis, TimeUnit::kMilliseconds)->as_disposable_handle();
    }

    /** @suppress */
    std::string to_string() const override {
        return scheduler->to_string();
    }

    /** @suppress */
    bool equals(const SchedulerCoroutineDispatcher* other) const {
        return other != nullptr && other->scheduler == scheduler;
    }

    /** @suppress */
    size_t hash_code() const override {
        return std::hash<void*>{}(scheduler);
    }
};

DisposableHandle* as_disposable_handle(Disposable* disposable) {
    return new class : public DisposableHandle {
    private:
        Disposable* disp;
    public:
        explicit DisposableHandleImpl(Disposable* d) : disp(d) {}
        void dispose() override {
            disp->dispose();
        }
    }(disposable);
}

} } } // namespace kotlinx::coroutines::reactor

// TODO: Semantic implementation tasks:
// 1. Implement Scheduler interface from Reactor
// 2. Implement CoroutineDispatcher base class
// 3. Implement Delay interface
// 4. Implement Disposable and DisposableHandle interfaces
// 5. Implement schedule() methods on Scheduler
// 6. Implement TimeUnit enum with kMilliseconds
// 7. Implement CancellableContinuation<T>
// 8. Implement resume_undispatched() method
// 9. Implement dispose_on_cancellation() method
// 10. Implement Unit type
// 11. Implement Runnable interface
// 12. Implement as_disposable_handle() conversion
// 13. Implement to_string() for Scheduler
// 14. Implement hash_code() and equals() properly
// 15. Handle System.identityHashCode equivalent
