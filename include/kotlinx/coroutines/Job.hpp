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

struct Job;
struct ChildJob;
struct ParentJob;

struct ChildHandle : public DisposableHandle {
    virtual bool child_cancelled(std::exception_ptr cause) = 0;
};

struct Job : public CoroutineContext::Element {
    inline static CoroutineContext::KeyTyped<Job> key_instance;
    static constexpr Key* typeKey = &key_instance;

    virtual ~Job() = default;

    // State Query
    virtual bool is_active() const = 0;
    virtual bool is_completed() const = 0;
    virtual bool is_cancelled() const = 0;
    virtual std::exception_ptr get_cancellation_exception() = 0;

    // State Update
    virtual bool start() = 0;
    virtual void cancel(std::exception_ptr cause = nullptr) = 0;
    
    // Parent-Child
    virtual std::shared_ptr<Job> get_parent() const = 0;
    virtual std::vector<std::shared_ptr<Job>> get_children() const = 0;
    virtual std::shared_ptr<DisposableHandle> attach_child(std::shared_ptr<ChildJob> child) = 0; // Signature update to ChildJob?

    // Waiting
    virtual void join() = 0;

    // Low-level Notification
    virtual std::shared_ptr<DisposableHandle> invoke_on_completion(std::function<void(std::exception_ptr)> handler) = 0;
    virtual std::shared_ptr<DisposableHandle> invoke_on_completion(bool on_cancelling, bool invoke_immediately, std::function<void(std::exception_ptr)> handler) = 0;

    // Key overrides
    virtual CoroutineContext::Key* key() const override { return Job::typeKey; }
    
    // Helpers
    virtual void cancel_and_join();
    virtual void ensure_active();
};

struct ChildJob : public virtual Job {
    virtual void parent_cancelled(ParentJob* parent) = 0;
};

struct ParentJob : public virtual Job {
    virtual bool child_cancelled(std::exception_ptr cause) = 0;
};

// Factory
std::shared_ptr<Job> Job(std::shared_ptr<Job> parent = nullptr);

} // namespace coroutines
} // namespace kotlinx
