#pragma once

/**
 * @file MultithreadedDispatchers.hpp
 * @brief Multi-threaded dispatcher declarations
 *
 * Transliterated from: kotlinx-coroutines-core/concurrent/src/MultithreadedDispatchers.common.kt
 *
 * Declarations for creating coroutine execution contexts using thread pools.
 *
 * NOTE: The resulting CloseableCoroutineDispatcher owns native resources (threads).
 * Resources are reclaimed by CloseableCoroutineDispatcher::close().
 */

#include "kotlinx/coroutines/CloseableCoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include <string>
#include <vector>
#include <thread>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace kotlinx {
namespace coroutines {

/**
 * Implementation of a thread pool-based coroutine dispatcher.
 *
 * This dispatcher maintains a fixed-size pool of worker threads that
 * execute submitted tasks. Tasks are queued and executed in FIFO order
 * by available workers.
 */
class ExecutorCoroutineDispatcherImpl : public CloseableCoroutineDispatcher {
public:
    /**
     * Creates a new dispatcher with the specified number of threads.
     *
     * @param n_threads the number of worker threads to create
     * @param name the name of this dispatcher (used in to_string())
     */
    ExecutorCoroutineDispatcherImpl(int n_threads, std::string name);

    /**
     * Destructor. Closes the dispatcher and joins all worker threads.
     */
    ~ExecutorCoroutineDispatcherImpl() override;

    // Non-copyable, non-movable
    ExecutorCoroutineDispatcherImpl(const ExecutorCoroutineDispatcherImpl&) = delete;
    ExecutorCoroutineDispatcherImpl& operator=(const ExecutorCoroutineDispatcherImpl&) = delete;

    /**
     * Returns the name of this dispatcher.
     */
    std::string to_string() const override;

    /**
     * Dispatches a task for execution on one of the worker threads.
     *
     * @param context the coroutine context (unused)
     * @param block the task to execute
     */
    void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override;

    /**
     * Closes this dispatcher, preventing new tasks from being submitted
     * and signaling all worker threads to complete.
     */
    void close() override;

private:
    void worker_loop();

    std::string name_;
    int n_threads_;
    std::vector<std::thread> workers_;
    std::deque<std::shared_ptr<Runnable>> task_queue_;
    mutable std::mutex queue_mutex_;
    mutable std::condition_variable condition_;
    std::atomic<bool> closed_;
};

/**
 * Creates a coroutine execution context with a single thread and built-in yield support.
 *
 * @param name the base name of the created thread.
 * @return a CloseableCoroutineDispatcher with a single thread
 */
CloseableCoroutineDispatcher* new_single_thread_context(const std::string& name);

/**
 * Creates a coroutine execution context with the fixed-size thread-pool and built-in yield support.
 *
 * @param n_threads the number of threads.
 * @param name the base name of the created threads.
 * @return a CloseableCoroutineDispatcher with the specified number of threads
 */
CloseableCoroutineDispatcher* new_fixed_thread_pool_context(int n_threads, const std::string& name);

} // namespace coroutines
} // namespace kotlinx
