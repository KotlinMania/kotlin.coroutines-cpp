#pragma once
#include <string>
#include <memory>
#include <vector>
#include <exception>
#include <functional>
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/DisposableHandle.hpp"

namespace kotlinx {
namespace coroutines {

struct Job : public CoroutineContext::Element {
    inline static CoroutineContext::KeyTyped<Job> key_instance;
    static constexpr Key* typeKey = &key_instance;

    virtual ~Job() = default;

    virtual std::shared_ptr<Job> get_parent() const = 0;
    virtual bool is_active() const = 0;
    virtual bool is_completed() const = 0;
    virtual bool is_cancelled() const = 0;
    virtual std::exception_ptr get_cancellation_exception() = 0;
    virtual bool start() = 0;
    virtual void cancel(std::exception_ptr cause = nullptr) = 0;
    
    virtual void cancel_and_join() {
        cancel();
        join();
    }

    virtual void ensure_active() {
        if (!is_active()) {
             std::exception_ptr ex = get_cancellation_exception();
             if (ex) std::rethrow_exception(ex);
             throw std::runtime_error("Job cancelled but no exception cause");
        }
    }

    virtual std::vector<std::shared_ptr<Job>> get_children() const = 0;
    virtual std::shared_ptr<DisposableHandle> attach_child(std::shared_ptr<Job> child) = 0;
    virtual void join() = 0;

    virtual std::shared_ptr<DisposableHandle> invoke_on_completion(std::function<void(std::exception_ptr)> handler) = 0;
    virtual std::shared_ptr<DisposableHandle> invoke_on_completion(bool on_cancelling, bool invoke_immediately, std::function<void(std::exception_ptr)> handler) = 0;
    
    // Key overrides
    virtual CoroutineContext::Key* key() const override { return Job::typeKey; }
};

} // namespace coroutines
} // namespace kotlinx
