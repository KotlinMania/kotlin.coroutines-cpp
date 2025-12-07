#pragma once
#include <string>
#include <memory>
#include <atomic>
#include <vector>
#include <functional>
#include <exception>
#include <mutex>
#include <condition_variable>
#include "../../../include/kotlinx/coroutines/core_fwd.hpp"
#include "../../../include/kotlinx/coroutines/Job.hpp"

namespace kotlinx {
namespace coroutines {

// Forward declarations
class JobNode;
class NodeList;
class Finishing;
class ChildHandle;
class Incomplete;

/**
 * A concrete implementation of Job with atomic state machine.
 * 
 * This implementation is designed for extension by more specific classes that might augment the
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
 * - SINGLE_PLUS: Active, single listener + NodeList.
 * - LIST_N: New, list of listeners.
 * - LIST_A: Active, list of listeners.
 * - COMPLETING: Finishing, has list of listeners.
 * - CANCELLING: Finishing, cancelling listeners not admitted.
 * - FINAL_C: Cancelled (final).
 * - FINAL_R: Completed (final).
 */
class JobSupport : public virtual Job, public std::enable_shared_from_this<JobSupport> {
public:
    // State encoding constants
    static constexpr uintptr_t STATE_MASK = 0xFFFFFFFFFFFFFFF8ULL; // Low 3 bits for type
    static constexpr uintptr_t TYPE_MASK = 0x7ULL;
    
    // State type constants (low 3 bits)
    static constexpr uintptr_t EMPTY_NEW_TYPE = 0x0ULL;
    static constexpr uintptr_t EMPTY_ACTIVE_TYPE = 0x1ULL;
    static constexpr uintptr_t SINGLE_NODE_TYPE = 0x2ULL;
    static constexpr uintptr_t SINGLE_PLUS_TYPE = 0x3ULL;
    static constexpr uintptr_t LIST_NODE_TYPE = 0x4ULL;
    static constexpr uintptr_t COMPLETING_TYPE = 0x5ULL;
    static constexpr uintptr_t CANCELLING_TYPE = 0x6ULL;
    static constexpr uintptr_t FINAL_C_TYPE = 0x7ULL;
    static constexpr uintptr_t FINAL_R_TYPE = 0x8ULL; // Need additional bit for R type
    
    // Permission flags for node lists
    static constexpr int LIST_ON_COMPLETION_PERMISSION = 1;
    static constexpr int LIST_CHILD_PERMISSION = 2;
    static constexpr int LIST_CANCELLATION_PERMISSION = 4;
    
    // Special state symbols
    static void* const COMPLETING_WAITING_CHILDREN;
    static void* const COMPLETING_RETRY;
    static void* const TOO_LATE_TO_CANCEL;
    static void* const SEALED;

protected:
    // Atomic state machine
    std::atomic<uintptr_t> _state;
    
    // Parent handle for child job relationship
    std::atomic<void*> _parent_handle;
    
    // Mutex for condition variable
    mutable std::mutex _completion_mutex;
    std::condition_variable _completion_cv;
    
    // Cached cancellation exception
    mutable std::exception_ptr _cached_cancellation_exception;
    
    // Constructor
    JobSupport(bool active);

public:
    virtual ~JobSupport() = default;

    // Job interface implementation
    std::shared_ptr<Job> get_parent() const override;
    bool is_active() const override;
    bool is_completed() const override;
    bool is_cancelled() const override;
    std::exception_ptr get_cancellation_exception() override;
    bool start() override;
    void cancel(std::exception_ptr cause = nullptr) override;
    std::vector<std::shared_ptr<Job>> get_children() const override;
    std::shared_ptr<DisposableHandle> attach_child(std::shared_ptr<Job> child) override;
    void join() override;
    std::shared_ptr<DisposableHandle> invoke_on_completion(std::function<void(std::exception_ptr)> handler) override;
    std::shared_ptr<DisposableHandle> invoke_on_completion(bool on_cancelling, bool invoke_immediately, std::function<void(std::exception_ptr)> handler) override;

protected:
    // Core state machine operations
    virtual void init_parent_job(std::shared_ptr<Job> parent);
    virtual bool start_internal();
    virtual void cancel_internal(std::exception_ptr cause);
    virtual std::shared_ptr<DisposableHandle> invoke_on_completion_internal(bool invoke_immediately, std::shared_ptr<JobNode> node);
    
    // State manipulation helpers
    void* get_state() const;
    bool cas_state(void* expected, void* update);
    void* force_finish(void* state);
    void* try_make_completing(void* state);
    void* try_make_completing_once(void* proposed_update);
    bool try_wait_for_child(std::shared_ptr<Job> child, void* state);
    
    // Completion handling
    virtual void complete_state_finalization(void* state);
    virtual void after_completion(void* state);
    virtual void on_completion_internal(void* state);
    virtual void handle_on_completion_exception(std::exception_ptr exception);
    virtual void clear_handles();
    
    // Child job management
    virtual void child_cancelled(std::exception_ptr cause);
    virtual void parent_cancelled(std::shared_ptr<Job> parent);
    virtual bool child_completed(std::shared_ptr<Job> child, void* child_state);
    
    // Utility methods
    virtual std::string name_string() const;
    virtual std::string to_string() const;
    virtual std::string cancellation_exception_message() const;
    virtual bool handles_exception() const;
    
    // State predicates
    bool is_active_state(void* state) const;
    bool is_completed_state(void* state) const;
    bool is_cancelling_state(void* state) const;
    bool is_finishing_state(void* state) const;
    bool is_final_state(void* state) const;

private:
    // Internal state creation helpers
    static void* make_empty_state(bool is_active);
    static void* make_single_node_state(std::shared_ptr<JobNode> node);
    static void* make_list_node_state(std::shared_ptr<NodeList> list, bool is_active);
    static void* make_finishing_state(std::shared_ptr<Finishing> finishing, bool is_cancelling);
    static void* make_final_state(bool is_cancelled, std::exception_ptr exception = nullptr);
    
    // State extraction helpers
    std::shared_ptr<Incomplete> get_incomplete_state(void* state) const;
    std::shared_ptr<Finishing> get_finishing_state(void* state) const;
    std::exception_ptr get_exception_from_final_state(void* state) const;
    
    // Notification helpers
    void notify_completion(void* state);
    void notify_cancelling(std::shared_ptr<Finishing> finishing);
    void complete_with_final_state(void* state, bool handle_exception = true);
    void finalize_finishing_state(std::shared_ptr<Finishing> finishing, std::exception_ptr proposed_exception);
    
    // List management
    std::shared_ptr<NodeList> promote_to_list(std::shared_ptr<Incomplete> incomplete);
    std::shared_ptr<DisposableHandle> add_node_to_list(std::shared_ptr<NodeList> list, std::shared_ptr<JobNode> node);
    
    // Cancellation helpers
    std::exception_ptr create_cancellation_exception(std::exception_ptr cause) const;
    void cancel_root_cause(std::exception_ptr cause);
    void cancel_parent(std::exception_ptr cause);
};

/**
 * Base class for completion handler nodes.
 */
class JobNode {
public:
    std::shared_ptr<Job> job;
    bool on_cancelling;
    
    JobNode(std::shared_ptr<Job> job, bool on_cancelling = false) 
        : job(job), on_cancelling(on_cancelling) {}
    
    virtual ~JobNode() = default;
    virtual void invoke(std::exception_ptr cause) = 0;
    virtual std::shared_ptr<DisposableHandle> dispose() = 0;
};

/**
 * Node list for managing multiple completion handlers.
 */
class NodeList : public JobNode {
public:
    std::atomic<JobNode*> head;
    int permissions;
    
    NodeList(std::shared_ptr<Job> job, int permissions = 0) 
        : JobNode(job), permissions(permissions), head(nullptr) {}
    
    void invoke(std::exception_ptr cause) override;
    std::shared_ptr<DisposableHandle> dispose() override;
    
    bool is_empty() const;
    std::shared_ptr<DisposableHandle> add_last(std::shared_ptr<JobNode> node);
    std::shared_ptr<DisposableHandle> remove(std::shared_ptr<JobNode> node);
};

/**
 * Represents a job in the process of completion.
 */
class Finishing {
public:
    std::shared_ptr<NodeList> list;
    std::atomic<bool> is_sealed;
    std::atomic<bool> is_completing;
    std::exception_ptr root_cause;
    
    Finishing(std::shared_ptr<NodeList> list, std::exception_ptr root_cause = nullptr)
        : list(list), is_sealed(false), is_completing(true), root_cause(root_cause) {}
    
    bool get_is_sealed() const { return is_sealed.load(); }
    bool get_is_completing() const { return is_completing.load(); }
    void set_sealed() { is_sealed.store(true); }
    void set_completing(bool completing) { is_completing.store(completing); }
};

/**
 * Interface for incomplete job states.
 */
class Incomplete {
public:
    virtual ~Incomplete() = default;
    virtual bool is_active() const = 0;
    virtual std::shared_ptr<NodeList> get_list() = 0;
};

/**
 * Empty state implementation (no listeners).
 */
class EmptyState : public Incomplete {
private:
    bool _is_active;
    
public:
    EmptyState(bool is_active) : _is_active(is_active) {}
    
    bool is_active() const override { return _is_active; }
    std::shared_ptr<NodeList> get_list() override { return nullptr; }
};

/**
 * Single node state (one listener).
 */
class SingleNodeState : public Incomplete {
private:
    std::shared_ptr<JobNode> node;
    
public:
    SingleNodeState(std::shared_ptr<JobNode> node) : node(node) {}
    
    bool is_active() const override { return true; }
    std::shared_ptr<NodeList> get_list() override { return nullptr; }
    std::shared_ptr<JobNode> get_node() const { return node; }
};

/**
 * Inactive node list state (multiple listeners, new job).
 */
class InactiveNodeListState : public Incomplete {
private:
    std::shared_ptr<NodeList> list;
    
public:
    InactiveNodeListState(std::shared_ptr<NodeList> list) : list(list) {}
    
    bool is_active() const override { return false; }
    std::shared_ptr<NodeList> get_list() override { return list; }
};

/**
 * Active node list state (multiple listeners, active job).
 */
class NodeListState : public Incomplete {
private:
    std::shared_ptr<NodeList> list;
    
public:
    NodeListState(std::shared_ptr<NodeList> list) : list(list) {}
    
    bool is_active() const override { return true; }
    std::shared_ptr<NodeList> get_list() override { return list; }
};

} // namespace coroutines
} // namespace kotlinx