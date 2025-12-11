#pragma once
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"
#include "kotlinx/coroutines/DisposableHandle.hpp"
#include "kotlinx/coroutines/internal/LockFreeLinkedList.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include <atomic>
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#include <string>
#include <algorithm>

namespace kotlinx {
namespace coroutines {

class JobSupport; 

// --------------------------------------------------------------------------
// Internal Helper Classes & Constants
// --------------------------------------------------------------------------

struct Symbol { 
    const char* name; 
    std::string to_string() const;
};

inline static Symbol _symbol_completing_already{"COMPLETING_ALREADY"};
static constexpr void* COMPLETING_ALREADY = &_symbol_completing_already;

inline static Symbol _symbol_waiting_children{"COMPLETING_WAITING_CHILDREN"};
static constexpr void* COMPLETING_WAITING_CHILDREN = &_symbol_waiting_children;

inline static Symbol _symbol_completing_retry{"COMPLETING_RETRY"};
static constexpr void* COMPLETING_RETRY = &_symbol_completing_retry;

inline static Symbol _symbol_too_late{"TOO_LATE_TO_CANCEL"};
static constexpr void* TOO_LATE_TO_CANCEL = &_symbol_too_late;

inline static Symbol _symbol_sealed{"SEALED"};
static constexpr void* SEALED = &_symbol_sealed;

static constexpr int RETRY = -1;
static constexpr int FALSE = 0;
static constexpr int TRUE = 1;

// Base class for all state objects to enable RTTI
// JobState defined in CompletedExceptionally.hpp

class NodeList;

// --------------------------------------------------------------------------
// Incomplete Interface
// --------------------------------------------------------------------------
class Incomplete : public JobState {
public:
    virtual bool is_active() const = 0;
    virtual NodeList* get_list() const = 0; // Returns raw pointer (not owning)
    virtual ~Incomplete() = default;
};

// --------------------------------------------------------------------------
// NodeList (List Head)
// --------------------------------------------------------------------------
class NodeList : public Incomplete, public internal::LockFreeLinkedListHead {
public:
    bool is_active() const override { return true; }
    NodeList* get_list() const override { return const_cast<NodeList*>(this); }

    std::string to_string() const;

    // Transliterated from JobSupport.kt:355-358
    void notify_completion(std::exception_ptr cause);
};

// --------------------------------------------------------------------------
// JobNode (List Item)
// --------------------------------------------------------------------------
class JobNode : public Incomplete, public internal::LockFreeLinkedListNode, public DisposableHandle {
public:
    JobSupport* job; 

    // Abstract methods from Kotlin JobNode
    virtual bool get_on_cancelling() const = 0;
    virtual void invoke(std::exception_ptr cause) = 0;

    bool is_active() const override { return true; }
    NodeList* get_list() const override { return nullptr; }

    // Transliterated from JobSupport.kt:1475
    void dispose() override;

    std::string to_string() const;
    
    internal::LockFreeLinkedListNode* get_next() { return next_node(); } 
};

// --------------------------------------------------------------------------
// Empty State
// --------------------------------------------------------------------------
class Empty : public Incomplete {
    bool is_active_;
public:
    explicit Empty(bool is_active) : is_active_(is_active) {}
    bool is_active() const override { return is_active_; }
    NodeList* get_list() const override { return nullptr; }
    std::string to_string() const;
};

inline static Empty _empty_new(false);
inline static Empty _empty_active(true);

// --------------------------------------------------------------------------
// InactiveNodeList State
// --------------------------------------------------------------------------
class InactiveNodeList : public Incomplete {
    NodeList* list_;
public:
    explicit InactiveNodeList(NodeList* list) : list_(list) {}
    bool is_active() const override { return false; }
    NodeList* get_list() const override { return list_; }
    std::string to_string() const;
};

// --------------------------------------------------------------------------
// Finishing State
// --------------------------------------------------------------------------
class Finishing : public Incomplete {
public:
    NodeList* list;
    std::atomic<bool> isCompleting;
    std::atomic<void*> _rootCause; 

    std::atomic<void*> exceptionsHolder; // null | exception_ptr | vector<exception_ptr> | SEALED
    std::recursive_mutex mutex;

    Finishing(NodeList* list, bool is_completing, std::exception_ptr root_cause)
        : list(list), isCompleting(is_completing), exceptionsHolder(nullptr) {
            if (root_cause) _rootCause.store(new std::exception_ptr(root_cause));
            else _rootCause.store(nullptr);
        }
        
    ~Finishing() {
        void* root = _rootCause.load();
        if (root) delete static_cast<std::exception_ptr*>(root);
    }

    NodeList* get_list() const override { return list; }
    
    // Helper to get root cause safely
    std::exception_ptr root_cause() const;

    bool is_active() const override { return _rootCause.load() == nullptr; }
    bool is_cancelling() const { return _rootCause.load() != nullptr; }
    bool is_sealed() const { return exceptionsHolder.load() == SEALED; }
    
    // Inline implementation acceptable for inner helper
    std::vector<std::exception_ptr> seal_locked(std::exception_ptr proposed_exception);

    void add_exception_locked(std::exception_ptr exception);
    
private:
    bool is_vector(void* ptr) {
        return ptr != nullptr && ptr != SEALED && 
               reinterpret_cast<uintptr_t>(ptr) % alignof(std::vector<std::exception_ptr>) == 0; 
    }
};

class ChildHandleNode : public JobNode, public ChildHandle {
public:
     ChildJob* child_job;
     ChildHandleNode(ChildJob* child) : child_job(child) {}

     bool get_on_cancelling() const override { return true; }
     // Transliterated from JobSupport.kt:1580
     void invoke(std::exception_ptr cause) override;
     // Transliterated from JobSupport.kt:1581
     bool child_cancelled(std::exception_ptr cause) override;
     // Transliterated from JobSupport.kt:1578
     std::shared_ptr<Job> get_parent() const override;
     // Transliterated from JobSupport.kt:1575
     void dispose() override;
};

class InvokeOnCompletion : public JobNode {
    std::function<void(std::exception_ptr)> handler;
public:
     InvokeOnCompletion(std::function<void(std::exception_ptr)> handler) : handler(handler) {}
     bool get_on_cancelling() const override { return false; }
     void invoke(std::exception_ptr cause) override { handler(cause); }
};

class InvokeOnCancelling : public JobNode {
    std::function<void(std::exception_ptr)> handler;
    std::atomic<bool> invoked;
public:
     InvokeOnCancelling(std::function<void(std::exception_ptr)> handler) : handler(handler), invoked(false) {}
     bool get_on_cancelling() const override { return true; }
     void invoke(std::exception_ptr cause) override {
         bool expected = false;
         if (invoked.compare_exchange_strong(expected, true)) handler(cause);
     }
};

// Used by parent waiting for child completion
// Transliterated from JobSupport.kt ChildCompletion inner class
class ChildCompletion : public JobNode {
public:
    JobSupport* parent;
    Finishing* state;
    ChildHandleNode* child;
    void* proposed_update;

    ChildCompletion(JobSupport* parent, Finishing* state, ChildHandleNode* child, void* proposed_update)
        : parent(parent), state(state), child(child), proposed_update(proposed_update) {}

    bool get_on_cancelling() const override { return false; }
    // Transliterated from JobSupport.kt:1267-1269
    void invoke(std::exception_ptr cause) override;
};


// --------------------------------------------------------------------------
// JobSupport
// --------------------------------------------------------------------------
class JobSupport : public ParentJob, public ChildJob, public std::enable_shared_from_this<JobSupport> {
    friend class JobNode;
    friend class ChildHandleNode;
// ...
    // ChildJob override
    void parent_cancelled(ParentJob* parent) override {
        cancel_internal(default_cancellation_exception("Parent cancelled"));
    }
    friend class NodeList;
public:
    static constexpr uintptr_t STATE_MASK = 0xFFFFFFFFFFFFFFF8ULL; 
    static constexpr uintptr_t TYPE_MASK = 0x7ULL;
    
    static constexpr int LIST_ON_COMPLETION_PERMISSION = 1;
    static constexpr int LIST_CHILD_PERMISSION = 2;
    static constexpr int LIST_CANCELLATION_PERMISSION = 4;
    
protected:
    std::atomic<void*> state_;
    std::shared_ptr<Job> parent_; 
    std::atomic<DisposableHandle*> parent_handle_;

public:
    JobSupport(bool active) : parent_handle_(nullptr) {
        state_.store(active ? (void*)&_empty_active : (void*)&_empty_new);
    }
    
    virtual ~JobSupport() = default;

    virtual bool handle_job_exception(std::exception_ptr exception) { return false; }

    // Job overrides
    std::shared_ptr<Job> get_parent() const override { return parent_; }
    std::shared_ptr<Element> get(Key* k) const override {
        if (const_cast<JobSupport*>(this)->key() == k) {
             return std::static_pointer_cast<Element>(const_cast<JobSupport*>(this)->shared_from_this());
        }
        return nullptr;
    }

    bool is_active() const override;
    bool is_completed() const override;
    // Transliterated from JobSupport.kt:181-184
    bool is_cancelled() const override;
    
    // Transliterated from JobSupport.kt:412-419
    std::exception_ptr get_cancellation_exception() override;

    bool start() override;
    void cancel(std::exception_ptr cause = nullptr) override { cancel_internal(cause); }
    void join() override;

    std::vector<std::shared_ptr<struct Job>> get_children() const override;
    std::shared_ptr<ChildHandle> attach_child(std::shared_ptr<ChildJob> child) override;

    // Low-level Notification
    std::shared_ptr<DisposableHandle> invoke_on_completion(std::function<void(std::exception_ptr)> handler) override {
        return invoke_on_completion(false, true, handler);
    }
    std::shared_ptr<DisposableHandle> invoke_on_completion(bool on_cancelling, bool invoke_immediately, std::function<void(std::exception_ptr)> handler) override;
    
    // ParentJob override
    std::exception_ptr get_child_job_cancellation_cause() override {
        return get_cancellation_exception();
    }

    /**
     * Child was cancelled with a cause.
     * In this method parent decides whether it cancels itself (e.g. on a critical failure)
     * and whether it handles the exception of the child.
     * It is overridden in supervisor implementations to completely ignore any child cancellation.
     * Returns true if exception is handled, false otherwise (then caller is responsible for handling an exception)
     *
     * Invariant: never returns false for instances of CancellationException, otherwise such exception
     * may leak to the CoroutineExceptionHandler.
     *
     * Transliterated from JobSupport.kt:680-683
     */
    virtual bool child_cancelled(std::exception_ptr cause);

    // Helper methods
    std::exception_ptr create_cause_exception(std::exception_ptr cause);

    std::exception_ptr default_cancellation_exception(const char* message = nullptr);
    
    void* make_cancelling(std::exception_ptr cause);
    bool try_make_cancelling(Incomplete* state, std::exception_ptr root_cause);
    NodeList* get_or_promote_cancelling_list(Incomplete* state);
    void promote_single_to_node_list(JobNode* state);
    void promote_empty_to_node_list(Empty* empty);
    
    // cancel_parent was declared line 522 in previous HPP but I forgot it in cleanup?
    // It should be here.
    bool cancel_parent(std::exception_ptr cause); // Added declaration
    
    void notify_cancelling(NodeList* list, std::exception_ptr cause);
    
    int start_internal(void* state);
    // Transliterated from JobSupport.kt:410
    virtual void on_start() {}

    // Transliterated from JobSupport.kt:651
    virtual std::string cancellation_exception_message() { return "Job was cancelled"; }

    // Transliterated from JobSupport.kt:689
    bool cancel_coroutine(std::exception_ptr cause);
    
    void cancel_internal(std::exception_ptr cause);
    virtual void on_cancelling(std::exception_ptr cause) {}
    
    bool make_completing(void* proposed_update);
    bool make_completing(std::exception_ptr ex); // Overload
    void* try_make_completing(void* state, void* proposed_update);
    void* try_make_completing_slow_path(Incomplete* state, void* proposed_update);
    
    virtual void on_completion_internal(void* final_state) {}
    virtual void after_completion(void* state) {}
    virtual void handle_on_completion_exception(std::exception_ptr exception) { std::rethrow_exception(exception); }

    // State finalization methods (transliterated from JobSupport.kt)
    void* finalize_finishing_state(Finishing* state, void* proposed_update);
    std::exception_ptr get_final_root_cause(Finishing* state, const std::vector<std::exception_ptr>& exceptions);
    void add_suppressed_exceptions(std::exception_ptr root_cause, const std::vector<std::exception_ptr>& exceptions);
    bool try_finalize_simple_state(Incomplete* state, void* update);
    void complete_state_finalization(Incomplete* state, void* update);

    // Parent-child relationship
    void init_parent_job(std::shared_ptr<Job> parent);

    // Completion handler registration
    std::shared_ptr<DisposableHandle> invoke_on_completion_internal(bool invoke_immediately, JobNode* node);

    // Child waiting methods
    bool try_wait_for_child(Finishing* state, ChildHandleNode* child, void* proposed_update);
    void continue_completing(Finishing* state, ChildHandleNode* last_child, void* proposed_update);
    ChildHandleNode* next_child(internal::LockFreeLinkedListNode* node);

    // One-shot completion (throws on repeat)
    void* make_completing_once(void* proposed_update);

    // Cancel and complete atomically
    void* cancel_make_completing(void* cause);

    // Full cancel implementation
    bool cancel_impl(void* cause);

    // State properties
    virtual bool get_on_cancel_complete() const { return false; }
    virtual bool get_is_scoped_coroutine() const { return false; }
    virtual bool get_handles_exception() const { return true; }

    // State query helpers
    bool is_completed_exceptionally() const;
    std::exception_ptr get_completion_exception_or_null() const;
    void* get_completed_internal() const;
    std::exception_ptr get_completion_cause() const;
    bool get_completion_cause_handled() const;

    // Debug helpers
    std::string to_debug_string() const;
    virtual std::string name_string() const;
    std::string state_string(void* state) const;
    
protected:
    bool try_put_node_into_list(JobNode* node, std::function<bool(Incomplete*, NodeList*)> try_add);
    // Transliterated from JobSupport.kt:619-636
    void remove_node(JobNode* node);
    
    template<typename Predicate>
    void notify_handlers(NodeList* list, std::exception_ptr cause, Predicate predicate) {
        std::exception_ptr exception = nullptr;
        list->for_each([&](internal::LockFreeLinkedListNode* node_raw) {
             auto* node = dynamic_cast<JobNode*>(node_raw);
             if (node && predicate(node)) {
                 try {
                     node->invoke(cause);
                 } catch (...) {
                     auto ex = std::current_exception();
                     if (!exception) exception = ex;
                 }
             }
        });
        if (exception) handle_on_completion_exception(exception);
    }

    // NOTE: suspend functions (join, awaitInternal) use blocking in C++ for now.
    // Kotlin Native uses LLVM indirectbr for suspend point resume - see SUSPEND_COMPARISON.md
    // Select support (onJoin, onAwaitInternal) deferred until select{} implementation.
};

} // namespace coroutines
} // namespace kotlinx
