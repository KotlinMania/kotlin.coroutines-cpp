#pragma once
#include <string>
#include <memory>
#include <atomic>
#include <vector>
#include <functional>
#include <any>
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/Job.hpp"

namespace kotlinx {
namespace coroutines {

/**
 * A concrete implementation of [Job]. It is optionally a child to a parent job.
 *
 * This is an open class designed for extension by more specific classes that might augment the
 * state and store additional state information for completed jobs, like their result values.
 *
 * == State Machine ==
 *
 * The state machine is optimized for the common case where a job is created in active state,
 * has at most one completion listener, and completes successfully without children.
 *
 * Internal states:
 * - EMPTY_NEW: New status, no listeners.
 * - EMPTY_ACTIVE: Active status, no listeners.
 * - SINGLE: Active, single listener.
 * - SINGLE+: Active, single listener + NodeList.
 * - LIST_N: New, list of listeners.
 * - LIST_A: Active, list of listeners.
 * - COMPLETING: Finishing, has list of listeners.
 * - CANCELLING: Finishing, cancelling listeners not admitted.
 * - FINAL_C: Cancelled (final).
 * - FINAL_R: Completed (final).
 */
class JobSupport : public virtual Job {
protected:
    bool active;
    std::shared_ptr<Job> parent;
    std::vector<std::shared_ptr<Job>> children_jobs;
    
public:
    JobSupport(bool active) : active(active) {}
    virtual ~JobSupport() = default;

    // Job overrides
    std::shared_ptr<Job> get_parent() const override {
        return parent;
    }

    bool is_active() const override {
        return active;
    }

    bool is_completed() const override {
        return !active; // simplistic
    }

    bool is_cancelled() const override {
        return false; // stub
    }

    std::exception_ptr get_cancellation_exception() override {
        return nullptr;
    }

    bool start() override {
        if (!active) {
            active = true;
            return true;
        }
        return false;
    }

    void cancel(std::exception_ptr cause = nullptr) override {
        active = false;
        // propagate to children
    }

    std::vector<std::shared_ptr<Job>> get_children() const override {
        return children_jobs;
    }

    std::shared_ptr<DisposableHandle> attach_child(std::shared_ptr<Job> child) override {
        children_jobs.push_back(child);
        // return handle to remove it
        return nullptr; // Todo: return a real handle
    }

    void join() override {
        // dummy impl, would wait on condition var
    }

    std::shared_ptr<DisposableHandle> invoke_on_completion(std::function<void(std::exception_ptr)> handler) override {
        // dummy impl
        return nullptr;
    }

    std::shared_ptr<DisposableHandle> invoke_on_completion(bool on_cancelling, bool invoke_immediately, std::function<void(std::exception_ptr)> handler) override {
        return nullptr;
    }

protected:
    virtual void on_start() {}
    
    void init_parent_job_internal(std::shared_ptr<Job> parent_job) {
        parent = parent_job;
        if (parent) {
            parent->attach_child(std::shared_ptr<Job>(this)); // Warning: dangerous this usage, needs shared_from_this in real impl
        }
    }
    
    // Additional helpers expected by AbstractCoroutine
    virtual std::string name_string() { return "Job"; }
    
    virtual void after_completion(std::any state) {
        // stub
    }

    virtual void on_completion_internal(std::any state) {}

    virtual std::string cancellation_exception_message() { return "Job was cancelled"; }
    
    virtual void handle_on_completion_exception(std::exception_ptr exception) {}
    
    
    std::any make_completing_once(std::any proposed_update) {
        // stub: just return the update for now or simulate success
        return proposed_update;
    }

    // Constants for state
    static inline void* const COMPLETING_WAITING_CHILDREN = (void*)1;
};

} // namespace coroutines
} // namespace kotlinx
