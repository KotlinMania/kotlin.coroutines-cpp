#include <string>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt
//
// TODO: @file:OptIn annotation
// TODO: Worker API from Kotlin/Native
// TODO: AtomicReference from Kotlin concurrent
// TODO: Channel API from kotlinx.coroutines
// TODO: suspend functions and coroutine semantics
// TODO: runBlocking implementation

namespace kotlinx {
namespace coroutines {

// TODO: Remove imports, fully qualify or add includes:
// import kotlinx.atomicfu.*
// import kotlinx.coroutines.channels.*
// import kotlinx.coroutines.internal.*
// import kotlin.coroutines.*
// import kotlin.concurrent.AtomicReference
// import kotlin.native.concurrent.*
// import kotlin.time.*
// import kotlin.time.Duration.Companion.milliseconds

// TODO: @DelicateCoroutinesApi annotation
CloseableCoroutineDispatcher* new_fixed_thread_pool_context(int n_threads, std::string name) {
    if (n_threads < 1) {
        throw std::invalid_argument("Expected at least one thread, but got: " + std::to_string(n_threads));
    }
    return new MultiWorkerDispatcher(name, n_threads);
}

// TODO: class
class WorkerDispatcher : CloseableCoroutineDispatcher, Delay {
private:
    void* worker; // TODO: Worker type

public:
    WorkerDispatcher(std::string name) {
        // TODO: worker = Worker.start(name = name)
    }

    void dispatch(CoroutineContext context, Runnable block) override {
        // TODO: worker.executeAfter(0L) { block.run() }
    }

    void schedule_resume_after_delay(long time_millis, CancellableContinuation<void>& continuation) override {
        auto handle = schedule(time_millis, Runnable([&continuation]() {
            continuation.resume_undispatched(/* Unit */);
        }));
        continuation.dispose_on_cancellation(handle);
    }

    DisposableHandle invoke_on_timeout(long time_millis, Runnable block, CoroutineContext context) override {
        return schedule(time_millis, block);
    }

private:
    DisposableHandle schedule(long time_millis, Runnable block) {
        // Workers don't have an API to cancel sent "executeAfter" block, but we are trying
        // to control the damage and reduce reachable objects by nulling out `block`
        // that may retain a lot of references, and leaving only an empty shell after a timely disposal
        // This is a class and not an class with `block` in a closure because that would defeat the purpose.
        class DisposableBlock : DisposableHandle {
        private:
            // TODO: AtomicReference<Runnable*>
            std::atomic<Runnable*> disposable_holder;

        public:
            DisposableBlock(Runnable block) : disposable_holder(new Runnable(block)) {}

            void operator()() {
                auto* block_ptr = disposable_holder.load();
                if (block_ptr != nullptr) {
                    block_ptr->run();
                }
            }

            void dispose() override {
                disposable_holder.store(nullptr);
            }

            bool is_disposed() {
                return disposable_holder.load() == nullptr;
            }
        };

        // TODO: Worker.runAfterDelay implementation with TimeSource
        auto disposable_block = new DisposableBlock(block);
        // TODO: auto targetMoment = TimeSource.Monotonic.markNow() + timeMillis.milliseconds
        // TODO: worker.runAfterDelay(disposableBlock, targetMoment)
        return disposable_block;
    }

public:
    void close() override {
        // TODO: worker.requestTermination().result // Note: calling "result" blocks
    }
};

// TODO: class
class MultiWorkerDispatcher : CloseableCoroutineDispatcher {
private:
    std::string name;
    int workers_count;
    Channel<Runnable>* tasks_queue;
    Channel<CancellableContinuation<Runnable>*>* available_workers;
    OnDemandAllocatingPool* worker_pool;

    /**
     * (number of tasks - number of workers) * 2 + (1 if closed)
     */
    std::atomic<long> tasks_and_workers_counter;

    inline bool is_closed(long value) const { return (value & 1L) == 1L; }
    inline bool has_tasks(long value) const { return value >= 2; }
    inline bool has_workers(long value) const { return value < 0; }

    void worker_run_loop() {
        // TODO: runBlocking implementation
        while (true) {
            auto state = tasks_and_workers_counter.fetch_sub(2);
            if (is_closed(state) && !has_tasks(state)) {
                return;
            }

            if (has_tasks(state)) {
                // we promised to process a task, and there are some
                tasks_queue->receive().run();
            } else {
                try {
                    // TODO: suspendCancellableCoroutine implementation
                } catch (CancellationException& e) {
                    /** we are cancelled from [close] and thus will never get back to this branch of code,
                    but there may still be pending work, so we can't just exit here. */
                }
            }
        }
    }

    // a worker that promised to be here and should actually arrive, so we wait for it in a blocking manner.
    CancellableContinuation<Runnable>* obtain_worker() {
        auto result = available_workers->try_receive();
        // TODO: getOrNull equivalent
        if (/* has value */) {
            return /* value */nullptr;
        } else {
            // TODO: runBlocking { availableWorkers.receive() }
            return nullptr;
        }
    }

public:
    MultiWorkerDispatcher(std::string name, int workers_count)
        : name(name)
        , workers_count(workers_count)
        , tasks_and_workers_counter(0L)
    {
        // TODO: tasks_queue = Channel<Runnable>(Channel.UNLIMITED)
        // TODO: available_workers = Channel<CancellableContinuation<Runnable>>(Channel.UNLIMITED)
        // TODO: worker_pool = OnDemandAllocatingPool implementation
    }

    void dispatch(CoroutineContext context, Runnable block) override {
        auto state = tasks_and_workers_counter.fetch_add(2);
        if (is_closed(state)) {
            throw std::runtime_error("Dispatcher " + name + " was closed, attempted to schedule: " + /* block.tostd::string() */ "");
        }

        if (has_workers(state)) {
            // there are workers that have nothing to do, let's grab one of them
            obtain_worker()->resume(block);
        } else {
            worker_pool->allocate();
            // no workers are available, we must queue the task
            auto result = tasks_queue->try_send(block);
            check_channel_result(result);
        }
    }

    CoroutineDispatcher* limited_parallelism(int parallelism, std::string* name = nullptr) override {
        check_parallelism(parallelism);
        if (parallelism >= workers_count) {
            return named_or_this(name);
        }
        return CloseableCoroutineDispatcher::limited_parallelism(parallelism, name);
    }

    void close() override {
        tasks_and_workers_counter.fetch_or(1L);
        auto workers = worker_pool->close(); // no new workers will be created;

        while (true) {
            // check if there are workers that await tasks in their personal channels, we need to wake them up
            auto state = tasks_and_workers_counter.load();
            if (has_workers(state)) {
                tasks_and_workers_counter.fetch_add(2);
            } else {
                break;
            }
            if (!has_workers(state)) {
                break;
            }
            obtain_worker()->cancel();
        }

        /*
         * Here we cannot avoid waiting on `.result`, otherwise it will lead
         * to a native memory leak, including a pthread handle.
         */
        // TODO: auto requests = workers.map { it.requestTermination() }
        // TODO: requests.map { it.result }
    }

private:
    void check_channel_result(/* ChannelResult<*> */ void* result) {
        // TODO: if (!result.isSuccess)
        //     throw IllegalStateException(...)
    }
};

} // namespace coroutines
} // namespace kotlinx
