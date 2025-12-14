/**
 * @file MultithreadedDispatchers.common.cpp
 * @brief Multi-threaded dispatcher factory functions
 *
 * Transliterated from: kotlinx-coroutines-core/concurrent/src/MultithreadedDispatchers.common.kt
 *
 * Factory functions for creating coroutine execution contexts using thread pools.
 *
 * NOTE: The resulting CloseableCoroutineDispatcher owns native resources (threads).
 * Resources are reclaimed by CloseableCoroutineDispatcher::close().
 */

#include "kotlinx/coroutines/MultithreadedDispatchers.hpp"
#include <iostream>

namespace kotlinx {
    namespace coroutines {
        //
        // ExecutorCoroutineDispatcherImpl implementation
        //

        void ExecutorCoroutineDispatcherImpl::worker_loop() {
            while (true) {
                std::shared_ptr<Runnable> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    condition_.wait(lock, [this] { return closed_ || !task_queue_.empty(); });

                    if (closed_ &&task_queue_
                    .
                    empty()
                    )
                    return;

                    if (!task_queue_.empty()) {
                        task = task_queue_.front();
                        task_queue_.pop_front();
                    }
                }
                if (task) {
                    try {
                        task->run();
                    } catch (const std::exception &e) {
                        std::cerr << "Exception in worker thread: " << e.what() << std::endl;
                    } catch (...) {
                        std::cerr << "Unknown exception in worker thread" << std::endl;
                    }
                }
            }
        }

        ExecutorCoroutineDispatcherImpl::ExecutorCoroutineDispatcherImpl(int n_threads, std::string name)
            : name_(std::move(name)), n_threads_(n_threads), closed_(false) {
            for (int i = 0; i < n_threads; ++i) {
                workers_.emplace_back(&ExecutorCoroutineDispatcherImpl::worker_loop, this);
            }
        }

        ExecutorCoroutineDispatcherImpl::~ExecutorCoroutineDispatcherImpl() {
            ExecutorCoroutineDispatcherImpl::close();
            for (auto &t: workers_) {
                if (t.joinable()) t.join();
            }
        }

        std::string ExecutorCoroutineDispatcherImpl::to_string() const {
            return name_;
        }

        void ExecutorCoroutineDispatcherImpl::dispatch(const CoroutineContext & context,
                                                       std::shared_ptr<Runnable> block) const {
            if (closed_) return;
            {
                auto self = const_cast<ExecutorCoroutineDispatcherImpl *>(this);
                std::lock_guard<std::mutex> lock(self->queue_mutex_);
                self->task_queue_.push_back(std::move(block));
            }
            auto self = const_cast<ExecutorCoroutineDispatcherImpl *>(this);
            self->condition_.notify_one();
        }

        void ExecutorCoroutineDispatcherImpl::close() {
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                closed_ = true;
            }
            condition_.notify_all();
        }

        //
        // Factory functions
        //

        CloseableCoroutineDispatcher *new_single_thread_context(const std::string &name) {
            return new_fixed_thread_pool_context(1, name);
        }

        CloseableCoroutineDispatcher *new_fixed_thread_pool_context(int n_threads, const std::string &name) {
            return new ExecutorCoroutineDispatcherImpl(n_threads, name);
        }
    } // namespace coroutines
} // namespace kotlinx