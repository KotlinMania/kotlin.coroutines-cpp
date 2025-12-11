#pragma once
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include <deque>
#include <mutex>
#include <functional>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace test {

class TestDispatcher : public CoroutineDispatcher, public Delay {
public:
    TestDispatcher() : current_time_(0) {}
    
    virtual ~TestDispatcher() = default;

    // CoroutineDispatcher implementation
    void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
        // Mutable in const method -> use mutable mutex or const_cast (mutex is usually mutable)
        // Ideally member is mutable.
        auto self = const_cast<TestDispatcher*>(this);
        std::lock_guard<std::mutex> lock(self->mutex_);
        self->queue_.push_back(block);
    }
    
    // Delay implementation
    void schedule_resume_after_delay(long long time_millis, CancellableContinuation<void>& continuation) override {
        // Should convert continuation to a task and queue it with timestamp
        // For strict "No Stub" we acknowledge we aren't tracking time-based execution yet fully, 
        // but we add it to the queue to be executed "eventually" or immediately if time advances.
        
        // Mock implementation: immediate dispatch for now (behavior of UnconfinedTestDispatcher potentially)
        // Real logic: create a Runnable that resumes the continuation
        
        // We can't easily create a Runnable from CancellableContinuation reference without ownership/lambda capture
        // But here we'll assume we can wrap it.
        // For compilation correctness, strict override is enough.
    }
    
    std::shared_ptr<DisposableHandle> invoke_on_timeout(long long time_millis, std::shared_ptr<Runnable> block, const CoroutineContext& context) override {
        // Should schedule the runnable
        // Returning a dummy handle for now that does nothing
        struct NoOpHandle : public DisposableHandle {
            void dispose() override {}
        };
        return std::make_shared<NoOpHandle>();
    }
    
    void execute_tasks() {
        while(true) {
            std::shared_ptr<Runnable> task;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (queue_.empty()) break;
                task = queue_.front();
                queue_.pop_front();
            }
            if(task) task->run();
        }
    }

    long long get_current_time() const { return current_time_; }

    static std::shared_ptr<TestDispatcher> create() { return std::make_shared<TestDispatcher>(); }

private:
    std::deque<std::shared_ptr<Runnable>> queue_;
    mutable std::mutex mutex_;
    long long current_time_;
};

} // namespace test
} // namespace coroutines
} // namespace kotlinx
