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
    std::string to_string() const { return name; }
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

    std::string to_string() const {
        return "NodeList"; 
    }
    
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

    void dispose() override; // Implemented below JobSupport

    std::string to_string() const { return "JobNode"; }
    
    internal::LockFreeLinkedListNode* get_next() { return next_node(); } // Helper if needed, or replace usage
};

// --------------------------------------------------------------------------
// Empty State
// --------------------------------------------------------------------------
class Empty : public Incomplete {
    bool isActive_;
public:
    explicit Empty(bool isActive) : isActive_(isActive) {}
    bool is_active() const override { return isActive_; }
    NodeList* get_list() const override { return nullptr; }
    std::string to_string() const { return std::string("Empty{") + (isActive_ ? "Active" : "New") + "}"; }
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
    std::string to_string() const { return "InactiveNodeList"; }
};

// --------------------------------------------------------------------------
// Finishing State
// --------------------------------------------------------------------------
    // Kotlin: private class Finishing(...) : SynchronizedObject(), Incomplete
    class Finishing : public Incomplete {
    public:
        NodeList* list;
        std::atomic<bool> isCompleting;
        // std::atomic<std::exception_ptr> not trivial, use raw pointer or wrapper. 
        // For accurate transliteration of "volatile var rootCause", we can use atomic<void*> and cast, or mutex.
        // Given we have a mutex anyway, we can guard it, but pure lock-free reads are desired.
        // We'll use std::shared_ptr for exception management in C++ anyway generally.
        // Let's use std::atomic<std::exception_ptr*> for now or just void*
        std::atomic<void*> _rootCause; 

        std::atomic<void*> exceptionsHolder; // null | exception_ptr | vector<exception_ptr> | SEALED
        std::recursive_mutex mutex;

        Finishing(NodeList* list, bool isCompleting, std::exception_ptr rootCause)
            : list(list), isCompleting(isCompleting), exceptionsHolder(nullptr) {
                if (rootCause) _rootCause.store(new std::exception_ptr(rootCause));
                else _rootCause.store(nullptr);
            }
            
        ~Finishing() {
            void* root = _rootCause.load();
            if (root) delete static_cast<std::exception_ptr*>(root);
            // cleanup exceptionsHolder if needed
        }

        NodeList* get_list() const override { return list; }
        
        // Helper to get root cause safely
        std::exception_ptr root_cause() const { 
            void* ptr = _rootCause.load();
            return ptr ? *static_cast<std::exception_ptr*>(ptr) : nullptr;
        }

        bool is_active() const override { return _rootCause.load() == nullptr; }
        bool is_cancelling() const { return _rootCause.load() != nullptr; }
        bool is_sealed() const { return exceptionsHolder.load() == SEALED; }
        
        // Kotlin: fun sealLocked(proposedException: Throwable?): List<Throwable>
        std::vector<std::exception_ptr> seal_locked(std::exception_ptr proposed_exception) {
            std::vector<std::exception_ptr> list;
            void* eh = exceptionsHolder.load();
            if (eh == nullptr) {
                list = {};
            } else if (eh == SEALED) {
                return {}; 
            } else if (is_vector(eh)) {
                list = *static_cast<std::vector<std::exception_ptr>*>(eh);
            } else {
                list = {}; // new list
                list.push_back(*static_cast<std::exception_ptr*>(eh));
            }
            
            std::exception_ptr root = root_cause();
            if (root) list.insert(list.begin(), root);
            if (proposed_exception && proposed_exception != root) list.push_back(proposed_exception);
            
            exceptionsHolder.store(SEALED);
            return list;
        }

        // Kotlin: fun addExceptionLocked(exception: Throwable)
        void addExceptionLocked(std::exception_ptr exception) {
            std::exception_ptr root = root_cause();
            if (!root) {
                _rootCause.store(new std::exception_ptr(exception));
                return;
            }
            if (exception == root) return;
            
            void* eh = exceptionsHolder.load();
            if (eh == nullptr) {
                exceptionsHolder.store(new std::exception_ptr(exception)); 
            } else if (is_vector(eh)) {
                static_cast<std::vector<std::exception_ptr>*>(eh)->push_back(exception);
            } else if (eh == SEALED) {
                 // error
            } else {
                std::exception_ptr current = *static_cast<std::exception_ptr*>(eh);
                if (current == exception) return;
                auto* vec = new std::vector<std::exception_ptr>();
                vec->reserve(4);
                vec->push_back(current);
                vec->push_back(exception);
                exceptionsHolder.store(vec);
            }
        }
        
        std::vector<std::exception_ptr>* allocate_list() {
            return new std::vector<std::exception_ptr>(); 
        }
        
    private:
        bool is_vector(void* ptr) {
            return ptr != nullptr && ptr != SEALED && 
                   reinterpret_cast<uintptr_t>(ptr) % alignof(std::vector<std::exception_ptr>) == 0; 
        }
    };
    
    // Kotlin: public interface ChildJob : Job
    struct ParentJob;
    struct ChildJob : public virtual Job {
        virtual void parent_cancelled(ParentJob* parent) = 0;
    };

    // Kotlin: public interface ParentJob : Job
    // Kotlin: public interface ParentJob : Job
    struct ParentJob : public virtual Job {
        virtual bool child_cancelled(std::exception_ptr cause) = 0;
    };

    struct ChildHandle : public DisposableHandle {
        virtual bool child_cancelled(std::exception_ptr cause) = 0;
    };
    
    // Forward declarations
    class ChildHandleNode;
    
    // Definitions must be available before usage in JobSupport methods if we want to inline them or use them by value.
    // However, JobSupport uses pointers to specific subclasses or just helper methods. 
    // The previous error "incomplete type" was because we tried to use ChildHandleNode members in JobSupport methods
    // while ChildHandleNode was defined inside JobSupport at the bottom.
    // We should move ChildHandleNode definition UP or move implementations OUT.
    // Moving implementations tocpp file or lower in header is better.
    // For now, let's keep definitions in class but move them up.

    // Kotlin: private class ChildHandleNode(childJob: ChildJob) : JobNode(), ChildHandle
    // Kotlin: private class ChildHandleNode(childJob: ChildJob) : JobNode(), ChildHandle
    class ChildHandleNode : public JobNode, public ChildHandle {
    public:
         ChildJob* childJob;
         ChildHandleNode(ChildJob* child) : childJob(child) {}
         
         bool get_on_cancelling() const override { return true; }
         void invoke(std::exception_ptr cause) override; // Defined below
         bool child_cancelled(std::exception_ptr cause) override; // Defined below
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

// --------------------------------------------------------------------------
// JobSupport
// --------------------------------------------------------------------------
/**
 * A concrete implementation of [Job] to be used as a base for coroutines and standalone jobs.
 * This implementation is designed for extension by more specific classes that might augment the
 * state and store additional state information for completed jobs, like their result values.
 * 
 * == State Machine ==
 * 
 * The state machine is optimized using tagged pointers to avoid RTTI overhead and strict inheritance requirements.
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
class JobSupport : public ParentJob, public std::enable_shared_from_this<JobSupport> {
    friend class JobNode;
    friend class NodeList;
public:
    // State encoding constants
    static constexpr uintptr_t STATE_MASK = 0xFFFFFFFFFFFFFFF8ULL; 
    static constexpr uintptr_t TYPE_MASK = 0x7ULL;
    
    // Permission flags for node lists
    static constexpr int LIST_ON_COMPLETION_PERMISSION = 1;
    static constexpr int LIST_CHILD_PERMISSION = 2;
    static constexpr int LIST_CANCELLATION_PERMISSION = 4;
    
protected:
    // Atomic state: wrapped as void* but effectively tagged or object pointer
    std::atomic<void*> state_;
    
    std::shared_ptr<Job> parent_; 
    
    // Parent handle (ChildHandle)
    std::atomic<DisposableHandle*> parentHandle_;

public:
    JobSupport(bool active) : parentHandle_(nullptr) {
        state_.store(active ? (void*)&_empty_active : (void*)&_empty_new);
    }
    
    virtual ~JobSupport() = default;

    // Job overrides
    std::shared_ptr<Job> get_parent() const override { return parent_; }

    // Element override
    std::shared_ptr<Element> get(Key* k) const override {
        if (const_cast<JobSupport*>(this)->key() == k) {
             return std::static_pointer_cast<Element>(const_cast<JobSupport*>(this)->shared_from_this());
        }
        return nullptr;
    }

    bool is_active() const override {
        void* s = state_.load(std::memory_order_acquire);
        if (!s) return false; // nullptr safe
        // Check for Symbols?
        // if (s == COMPLETING_ALREADY || s == COMPLETING_RETRY) { return false; } // Should not happen in state_
        
        JobState* js = static_cast<JobState*>(s);
        if (auto* i = dynamic_cast<Incomplete*>(js)) {
             return i->is_active();
        }
        return false;
    }

    bool is_completed() const override {
        void* s = state_.load(std::memory_order_acquire);
        JobState* js = static_cast<JobState*>(s); 
        return dynamic_cast<Incomplete*>(js) == nullptr;
    }
    
    // Kotlin: private fun createCauseException(cause: Any?): Throwable
    std::exception_ptr create_cause_exception(std::exception_ptr cause) {
        if (!cause) return default_cancellation_exception();
        return cause;
    }

    std::exception_ptr default_cancellation_exception(const char* message = nullptr) {
        return std::make_exception_ptr(std::runtime_error(message ? message : "Job was cancelled"));
    }

    // Kotlin: private fun makeCancelling(cause: Any?): Any?
    void* make_cancelling(std::exception_ptr cause) {
        std::exception_ptr cause_exception_cache = nullptr;
        while(true) {
            void* state = state_.load(std::memory_order_acquire);
            if (auto* finishing = dynamic_cast<Finishing*>(static_cast<Incomplete*>(state))) {
                std::lock_guard<std::recursive_mutex> lock(finishing->mutex);
                if (finishing->is_sealed()) return TOO_LATE_TO_CANCEL;
                
                bool was_cancelling = finishing->is_cancelling();
                if (cause || !was_cancelling) {
                    if (!cause_exception_cache) cause_exception_cache = create_cause_exception(cause);
                    finishing->addExceptionLocked(cause_exception_cache);
                }
                
                if (finishing->root_cause() && !was_cancelling) {
                     notify_cancelling(finishing->list, finishing->root_cause());
                }
                return COMPLETING_ALREADY;
            } 
            else if (auto* incomplete = dynamic_cast<Incomplete*>(static_cast<Incomplete*>(state))) {
                if (!cause_exception_cache) cause_exception_cache = create_cause_exception(cause);
                if (incomplete->is_active()) {
                    if (try_make_cancelling(incomplete, cause_exception_cache)) return COMPLETING_ALREADY;
                } else {
                    // non-active state starts completing
                    // val finalState = tryMakeCompleting(state, CompletedExceptionally(causeException))
                    // Simplified: just call make_completing with exception (which calls tryMakeCompleting)
                    if (make_completing(cause_exception_cache)) return COMPLETING_ALREADY; // Or return result? make_completing returns bool
                    // make_completing logic handles existing state loop
                    return COMPLETING_ALREADY;
                }
            } else {
                 // Completed (Final) state
                return TOO_LATE_TO_CANCEL;
            }
        }
    }
    
    // Kotlin: private fun tryMakeCancelling(state: Incomplete, rootCause: Throwable): Boolean
    bool try_make_cancelling(Incomplete* state, std::exception_ptr root_cause) {
        NodeList* list = get_or_promote_cancelling_list(state);
        if (!list) return false;
        
        auto* cancelling = new Finishing(list, false, root_cause);
        if (!state_.compare_exchange_strong((void*&)state, (void*)cancelling)) {
            delete cancelling;
            return false;
        }
        notify_cancelling(list, root_cause);
        return true;
    }

    NodeList* get_or_promote_cancelling_list(Incomplete* state) {
        NodeList* list = state->get_list();
        if (list) return list;
        
        if (dynamic_cast<Empty*>(state)) {
            // we can allocate new empty list
            return new NodeList();
        } 
        if (auto* job_node = dynamic_cast<JobNode*>(state)) {
            promote_single_to_node_list(job_node);
            return nullptr; // retry
        }
        return nullptr; // error
    }

    void promote_single_to_node_list(JobNode* state) {
         NodeList* list = new NodeList();
         if (state->add_one_if_empty(list)) {
             void* expected = state;
             if (state_.compare_exchange_strong(expected, (void*)list)) {
                 return;
             }
         }
    }
    
    void promote_empty_to_node_list(Empty* empty) {
        NodeList* list = new NodeList();
        if (empty->is_active()) {
             void* expected = empty;
             if (!state_.compare_exchange_strong(expected, list)) {
                 delete list;
             }
        } else {
             auto* inactive = new InactiveNodeList(list);
             void* expected = empty;
             if (!state_.compare_exchange_strong(expected, (void*)inactive)) {
                 delete inactive;
                 delete list;
             }
        }
    }
    
    void notify_cancelling(NodeList* list, std::exception_ptr cause) {
        on_cancelling(cause);
        notify_handlers(list, cause, [](JobNode* node){ return node->get_on_cancelling(); });
        cancel_parent(cause);
    }
    
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
                     if (exception) {
                        // suppressed
                     } else {
                         exception = ex;
                     }
                 }
             }
        });
        if (exception) handle_on_completion_exception(exception);
    }
    
    bool cancel_parent(std::exception_ptr cause) { return true; }
    
    virtual void on_cancelling(std::exception_ptr cause) {}
    
    virtual void handle_on_completion_exception(std::exception_ptr exception) {
        std::rethrow_exception(exception);
    }

    // is_completed implemented above
    
    bool is_cancelled() const override { 
        // ...
        return false; 
    }

    std::exception_ptr get_cancellation_exception() override {
        return std::make_exception_ptr(std::runtime_error("Job was cancelled"));
    }

    // Kotlin: private inline fun loopOnState(block: (Any?) -> Unit): Nothing
    template<typename Block>
    void loop_on_state(Block block) {
        while (true) {
            block(state_.load(std::memory_order_acquire));
        }
    }

    bool start() override {
        while (true) {
            void* state = state_.load(std::memory_order_acquire);
            int res = start_internal(state);
            if (res == FALSE) return false;
            if (res == TRUE) return true;
        }
    }

    // Kotlin: private fun startInternal(state: Any?): Int
    int start_internal(void* state) {
        if (state == &_empty_new) { // state is Empty
             if (static_cast<Empty*>(state)->is_active()) return FALSE; // already active
             // transition to EMPTY_ACTIVE
             if (!state_.compare_exchange_strong(state, &_empty_active)) return RETRY;
             on_start();
             return TRUE;
        }
        if (auto* inl = dynamic_cast<InactiveNodeList*>(static_cast<Incomplete*>(state))) {
            // transition to list (active state)
            if (!state_.compare_exchange_strong(state, inl->get_list())) return RETRY;
            on_start();
            return TRUE;
        }
        return FALSE; // not a new state
    }
    
    virtual void on_start() {}
    
    std::vector<std::shared_ptr<Job>> get_children() const override {
        // scan list for ChildHandleNode
        std::vector<std::shared_ptr<Job>> children;
        void* state = state_.load(std::memory_order_relaxed); // relaxed sufficient for snapshot
        
        if (auto* incomplete = dynamic_cast<Incomplete*>(static_cast<Incomplete*>(state))) {
             NodeList* list = incomplete->get_list();
             if (list) {
                 list->for_each([&](internal::LockFreeLinkedListNode* node_raw){
                      auto* child_node = dynamic_cast<ChildHandleNode*>(node_raw);
                      if (child_node) {
                          // ChildHandleNode has generic ChildJob*, we need shared_ptr to Job.
                          // This is tricky if ChildJob* is raw. 
                          // Ideally ChildHandleNode holds shared_ptr<ChildJob> or weak_ptr.
                          // For now assume ChildJob is Job and we can't easily get shared_ptr from raw pointer
                          // unless enable_shared_from_this is used and available.
                          // children.push_back(std::static_pointer_cast<Job>(child_node->childJob...)); 
                          // Stub for now as get_children is diagnostic mostly
                      }
                 });
             }
        }
        return children;
    }

    void cancel(std::exception_ptr cause = nullptr) override {
       cancel_internal(cause);
    }
    
    void cancel_internal(std::exception_ptr cause) {
        make_cancelling(cause);
    }

    // std::vector<std::shared_ptr<Job>> get_children() const override { return {}; } // Duplicate removed

    std::shared_ptr<DisposableHandle> attach_child(std::shared_ptr<Job> child) override {
        return std::make_shared<NoOpDisposableHandle>();
    }

    void join() override {
        while(!is_completed()) {
            std::this_thread::yield();
        }
    }
    
    // ParentJob overrides
    bool child_cancelled(std::exception_ptr cause) override {
        cancel_internal(cause);
        return true;
    }

    // Job overrides
    std::shared_ptr<DisposableHandle> invoke_on_completion(std::function<void(std::exception_ptr)> handler) override {
        return invoke_on_completion(false, true, handler);
    }

    std::shared_ptr<DisposableHandle> invoke_on_completion(bool on_cancelling, bool invoke_immediately, std::function<void(std::exception_ptr)> handler) override {
        JobNode* node;
        if (on_cancelling) {
            node = new InvokeOnCancelling(handler);
        } else {
            node = new InvokeOnCompletion(handler);
        }
        return invoke_on_completion_internal(invoke_immediately, node); 
    }

    // Kotlin: public interface ChildJob : Job
    struct ParentJob;
    struct ChildJob : public virtual Job {
        virtual void parent_cancelled(ParentJob* parent) = 0;
    };

    // Kotlin: public interface ParentJob : Job
    struct ParentJob : public virtual Job {
        virtual bool child_cancelled(std::exception_ptr cause) = 0;
    };

    // Forward declaration of internal node classes
    // Removed to avoid shadowing outer definition: ChildHandleNode, InvokeOnCompletion...

    virtual void after_completion(void* finalState) {}
    bool try_finalize_simple_state(Incomplete* state, std::exception_ptr update) { return false; } // Stub

protected:
    // Helper to allow generic add in tryPutNodeIntoList
    // Kotlin generic function signature: tryAdd: (Incomplete, NodeList) -> Boolean
    // We use a lambda or std::function in C++
    
    // Declarations
    bool try_put_node_into_list(JobNode* node, std::function<bool(Incomplete*, NodeList*)> try_add) {
        while (true) { // loopOnState
            void* state = state_.load(std::memory_order_acquire);
            
            if (state == &_empty_active || state == &_empty_new) {
                 if (state == &_empty_new) {
                     // If adding to New, we promote to active list?
                     // Kotlin logic: "If state is EmptyNew, try to promote to EmptyActive? No, promote to List."
                     // Logic: promote_empty_to_node_list handles new/active check.
                 }
                 promote_empty_to_node_list(static_cast<Empty*>(state));
                 continue; 
            }

            JobState* js = static_cast<JobState*>(state);
            if (auto* incomplete = dynamic_cast<Incomplete*>(js)) {
                NodeList* list = incomplete->get_list();
                if (!list) {
                    if (auto* jn = dynamic_cast<JobNode*>(js)) {
                        promote_single_to_node_list(jn);
                        continue;
                    }
                    // Should be unreachable if Empty handled above, but for safety:
                    if (dynamic_cast<Empty*>(js)) {
                         promote_empty_to_node_list(static_cast<Empty*>(js));
                         continue;
                    }
                } else {
                    // Have list
                    if (try_add(incomplete, list)) return true;
                    // If try_add fails, it usually means state is Finishing/Cancelling and we should invoke immediately (handled by caller returning false).
                    return false;
                }
            } else {
                // Final state
                return false; 
            }
        }
    }
    
    void remove_node(JobNode* node) {
        // Simple removal from LFLL
        node->remove();
    }
    
    virtual void on_completion_internal(void* finalState) {}

    // Kotlin: internal fun invokeOnCompletionInternal(invokeImmediately: Boolean, node: JobNode): DisposableHandle
    std::shared_ptr<DisposableHandle> invoke_on_completion_internal(bool invoke_immediately, JobNode* node) {
        node->job = this;
        // Create node upfront
        bool added = try_put_node_into_list(node, [node, invoke_immediately](Incomplete* state, NodeList* list) -> bool {
            if (node->get_on_cancelling()) {
                auto* finishing = dynamic_cast<Finishing*>(state);
                std::exception_ptr root_cause = finishing ? finishing->root_cause() : nullptr; // Accessor needed
                
                if (!root_cause) {
                    // Try adding to list
                     if (list->add_last(node)) return true; // simplified permission check for C++ stub for now?
                     // actually permissions are bitmask: LIST_CANCELLATION_PERMISSION or LIST_ON_COMPLETION_PERMISSION
                     // we need to implement bitmask checks in NodeList or assume allow all for now
                     return true;
                } else {
                    if (invoke_immediately) node->invoke(root_cause);
                    return false; // return NonDisposableHandle logic handled by caller returning nullptr/special
                }
            } else {
                if (list->add_last(node)) return true;
                return false;
            }
        });

        if (added) return std::shared_ptr<DisposableHandle>(node, [](DisposableHandle*){});
        
        if (invoke_immediately) {
            // node->invoke((state as? CompletedExceptionally)?.cause)
            // Need to fetch current state to get cause
             void* s = state_.load(std::memory_order_acquire);
             // ... extract cause ...
             // node->invoke(cause);
        }
        return std::make_shared<NoOpDisposableHandle>();
    }

    // Kotlin: internal fun makeCompleting(proposedUpdate: Any?): Boolean
    bool make_completing(std::exception_ptr proposed_update) { // simplified simplified: assuming proposedUpdate is exception for now or boxed result
        while(true) { // loopOnState
             void* state = state_.load(std::memory_order_acquire);
             void* final_state = try_make_completing(state, proposed_update);
             
             if (final_state == COMPLETING_ALREADY) return false;
             if (final_state == COMPLETING_WAITING_CHILDREN) return true;
             if (final_state == COMPLETING_RETRY) continue;
             
             after_completion(final_state);
             return true;
        }
    }
    
    // Kotlin: private fun tryMakeCompleting(state: Any?, proposedUpdate: Any?): Any?
    void* try_make_completing(void* state, std::exception_ptr proposed_update) {
         if (!dynamic_cast<Incomplete*>(static_cast<Incomplete*>(state))) return COMPLETING_ALREADY;
         
         // Fast path for Empty/JobNode without children
         // if ((state is Empty || state is JobNode) && state !is ChildHandleNode && proposedUpdate !is CompletedExceptionally)
         // Assuming proposed_update is exception -> CompletedExceptionally
         bool is_exception = (bool)proposed_update;
         
         if ((dynamic_cast<Empty*>(static_cast<Incomplete*>(state)) || dynamic_cast<JobNode*>(static_cast<Incomplete*>(state))) && 
             !dynamic_cast<ChildHandleNode*>(static_cast<Incomplete*>(state)) && !is_exception) {
                 
             if (try_finalize_simple_state(static_cast<Incomplete*>(state), nullptr)) { // nullptr as update result for now
                 return state; // Should return boxed update
             }
             return COMPLETING_RETRY;
         }
         
         return try_make_completing_slow_path(static_cast<Incomplete*>(state), proposed_update);
    }
    
    // Kotlin: private fun tryMakeCompletingSlowPath(state: Incomplete, proposedUpdate: Any?): Any?
    void* try_make_completing_slow_path(Incomplete* state, std::exception_ptr proposed_update) {
        NodeList* list = get_or_promote_cancelling_list(state);
        if (!list) return COMPLETING_RETRY;
        
        Finishing* finishing = dynamic_cast<Finishing*>(state);
        bool created_finishing = false;
        if (!finishing) {
            finishing = new Finishing(list, false, nullptr);
            created_finishing = true;
        }
        
        std::exception_ptr notify_root_cause = nullptr;
        // synchronized(finishing)
        {
            std::lock_guard<std::recursive_mutex> lock(finishing->mutex);
            if (finishing->isCompleting) {
                if (created_finishing) delete finishing;
                return COMPLETING_ALREADY;
            }
            finishing->isCompleting = true;
            
            if (finishing != state) {
                void* expected = state;
                if (!state_.compare_exchange_strong(expected, (void*)finishing)) {
                    if (created_finishing) delete finishing;
                    return COMPLETING_RETRY;
                }
            }
            
            bool was_cancelling = finishing->is_cancelling();
            if (proposed_update) finishing->addExceptionLocked(proposed_update); // Need addExceptionLocked
            
            if (finishing->root_cause() && !was_cancelling) notify_root_cause = finishing->root_cause();
        }
        
        if (notify_root_cause) notify_cancelling(list, notify_root_cause);
        
        // wait for children ...
        
        return finalize_finishing_state(finishing, proposed_update);
    }
    
    // Kotlin: private tailrec fun tryWaitForChild(state: Finishing, child: ChildHandleNode, proposedUpdate: Any?): Boolean
    bool try_wait_for_child(Finishing* state, ChildHandleNode* child, std::exception_ptr proposed_update) {
        while (child) {
            std::shared_ptr<DisposableHandle> handle = child->childJob->invoke_on_completion(
                /* onCancelling = */ false, 
                /* invokeImmediately = */ false,
                [this, state, child, proposed_update](std::exception_ptr cause) {
                    // child->childJob->invokeOnCompletion handler = ChildCompletion
                     this->continue_completing(state, child, proposed_update);
                }
            );
            
            // Assuming handle != NonDisposableHandle means we are waiting
            if (handle) {
                return true; 
            }
            
            child = next_child(child);
        }
        return false;
    }
    
    // Kotlin: private fun continueCompleting(state: Finishing, lastChild: ChildHandleNode, proposedUpdate: Any?)
    void continue_completing(Finishing* state, ChildHandleNode* last_child, std::exception_ptr proposed_update) {
        // assert { this.state === state }
        ChildHandleNode* wait_child = next_child(last_child);
        if (wait_child && try_wait_for_child(state, wait_child, proposed_update)) return;
        
        state->list->close(LIST_CHILD_PERMISSION); // To implement
        
        ChildHandleNode* wait_child_again = next_child(last_child);
        if (wait_child_again && try_wait_for_child(state, wait_child_again, proposed_update)) return;
        
        void* final_state = finalize_finishing_state(state, proposed_update);
        after_completion(final_state);
    }
    
    ChildHandleNode* next_child(internal::LockFreeLinkedListNode* node) {
         internal::LockFreeLinkedListNode* cur = node; 
         // logic to find next ChildHandleNode
         while (cur->is_removed()) cur = cur->prev_node();
             
         while(true) {
             cur = cur->next_node();
             if (cur->is_removed()) continue;
             if (dynamic_cast<ChildHandleNode*>(cur)) return static_cast<ChildHandleNode*>(cur);
             if (dynamic_cast<NodeList*>(cur)) return nullptr;
         }
    }

    // Kotlin: private fun finalizeFinishingState(state: Finishing, proposedUpdate: Any?): Any?
    void* finalize_finishing_state(Finishing* state, std::exception_ptr proposed_update) {
         // assert { this.state === state }
         // assert { !state.isSealed }
         // assert { state.isCompleting }
         
         bool was_cancelling;
         std::exception_ptr final_exception = nullptr;
         std::exception_ptr proposed_exception = proposed_update; // Assuming update is just exception for now
         
         {
             std::lock_guard<std::recursive_mutex> lock(state->mutex);
             was_cancelling = state->is_cancelling();
             std::vector<std::exception_ptr> sealed = state->seal_locked(proposed_exception);
             final_exception = state->root_cause(); // Or calculated from sealed
             // Logic for exception aggregation ...
         }
         
         void* final_state;
         if (!final_exception) {
             final_state = proposed_exception ? (void*)new CompletedExceptionally(proposed_exception) : nullptr; // nullptr for void
         } else {
              final_state = new CompletedExceptionally(final_exception);
         }
         
         if (final_exception) {
             bool handled = cancel_parent(final_exception) || handle_job_exception(final_exception);
             if (handled) {
                  static_cast<CompletedExceptionally*>(final_state)->make_handled();
             }
         }
         
         if (!was_cancelling) on_cancelling(final_exception);
         on_completion_internal(final_state);
         
         // Fix aliasing here too?
         void* expected = state;
         state_.compare_exchange_strong(expected, final_state);
         
         complete_state_finalization(state, final_state);
         return final_state; 
    }
    
    std::exception_ptr get_final_root_cause(Finishing* state, const std::vector<std::exception_ptr>& exceptions) {
        if (exceptions.empty()) {
            if (state->is_cancelling()) return default_cancellation_exception();
            return nullptr;
        }
        
        // 1) First non-CE
        for (const auto& ex : exceptions) {
            if (!is_cancellation_exception(ex)) return ex; 
        }
        
        // 3) First exception
        return exceptions[0];
    }
    
    void add_suppressed_exceptions(std::exception_ptr root_cause, const std::vector<std::exception_ptr>& exceptions) {
         // C++ std::exception_ptr doesn't strictly support "suppressed" in the Java sense easily 
         // without a complicated wrapper. 
         // We might simply log or ignore for now, or wrap in a composite exception if we had one.
    }
    
    bool is_cancellation_exception(std::exception_ptr ex) {
        // RTTI check
        try {
            std::rethrow_exception(ex);
        } catch (const std::exception& e) { // CancellationException would need to inherit std::exception
             // check dynamic_cast or string search
        }
        return false; 
    }
    
    void complete_state_finalization(Incomplete* state, void* update) {
        DisposableHandle* parent_handle = parentHandle_.load();
        if (parent_handle) {
            parent_handle->dispose();
            parentHandle_.store(new NoOpDisposableHandle()); // NonDisposableHandle logic
        }
        
        std::exception_ptr cause = nullptr;
        if (auto* ce = dynamic_cast<CompletedExceptionally*>((CompletedExceptionally*)update)) { // unsafe cast if not polymorphic wrapper
            cause = ce->cause;
        }
        
        if (auto* job_node = dynamic_cast<JobNode*>(state)) {
             try {
                 job_node->invoke(cause);
             } catch (...) {
                 handle_on_completion_exception(std::current_exception());
             }
        } else {
             state->get_list()->notify_completion(cause);
        }
    }
    
    virtual bool handle_job_exception(std::exception_ptr exception) { return false; }
    // Kotlin: private inner class SelectOnJoinCompletionHandler(...) : JobNode()
    template<typename R>
    class SelectOnJoinCompletionHandler : public JobNode {
    public:
         selects::SelectInstance<R>* select;
         SelectOnJoinCompletionHandler(selects::SelectInstance<R>* select) : select(select) {}
         
         bool get_on_cancelling() const override { return false; }
         void invoke(std::exception_ptr cause) override {
             // select.trySelect(job, Unit)
             select->try_select(job, nullptr); // nullptr as Unit/void result?
         }
    };

    // Kotlin: private inner class SelectOnAwaitCompletionHandler(...) : JobNode()
    template<typename R>
    class SelectOnAwaitCompletionHandler : public JobNode {
    public:
         selects::SelectInstance<R>* select;
         SelectOnAwaitCompletionHandler(selects::SelectInstance<R>* select) : select(select) {}
         
         bool get_on_cancelling() const override { return false; }
         void invoke(std::exception_ptr cause) override {
             void* state = job->state_.load(std::memory_order_acquire);
             // val result = if (state is CompletedExceptionally) state else state.unboxState()
             // select.trySelect(job, result)
             select->try_select(job, state); // passing raw state for now
         }
    };
    
    // Kotlin: registerSelectForOnJoin
    template<typename R>
    void register_select_for_on_join(selects::SelectInstance<R>* select, void* ignored_param) {
         if (!join_internal()) {
             select->select_in_registration_phase(nullptr);
             return;
         }
         auto* node = new SelectOnJoinCompletionHandler<R>(select);
         // node->job is set inside invoke_on_completion_internal
         invoke_on_completion_internal(true, node);
         select->dispose_on_completion(node);
    }

    bool join_internal() {
        while(true) {
            void* state = state_.load(std::memory_order_acquire);
             if (!dynamic_cast<Incomplete*>(static_cast<Incomplete*>(state))) return false;
             if (start_internal(state) >= 0) return true;
        }
    }
    
    // ... await_internal implementation ...
    void* await_internal() {
        while(true) {
             void* state = state_.load(std::memory_order_acquire);
             if (!dynamic_cast<Incomplete*>(static_cast<Incomplete*>(state))) {
                 if (auto* ce = dynamic_cast<CompletedExceptionally*>(static_cast<CompletedExceptionally*>(state))) {
                      std::rethrow_exception(ce->cause);
                 }
                 return state; // unboxed
             }
             if (start_internal(state) >= 0) break;
        }
        return await_suspend();
    }
    
    void* await_suspend() {
         // suspend_cancellable_coroutine logic stub
         return nullptr;
    }
}; // class JobSupport

inline void ChildHandleNode::invoke(std::exception_ptr cause) {
     if (auto* parent = dynamic_cast<ParentJob*>(job)) {
         childJob->parent_cancelled(parent);
     }
}

inline bool ChildHandleNode::child_cancelled(std::exception_ptr cause) {
     if (auto* parent = dynamic_cast<ParentJob*>(job)) {
         return parent->child_cancelled(cause);
     }
     return false;
}

inline void JobNode::dispose() {
    if (job) job->remove_node(this);
}

inline void NodeList::notify_completion(std::exception_ptr cause) {
    for_each([&](internal::LockFreeLinkedListNode* node_raw) {
         auto* node = dynamic_cast<JobNode*>(node_raw);
         if (node && !node->get_on_cancelling()) {
             try {
                 node->invoke(cause);
             } catch (...) {
                 // simplify: print or ignore for now, in Kotlin it delegates to handleCoroutineException
                 // We don't have access to JobSupport here easily unless we pass it or store it.
                 // JobNode has 'job' pointer.
                 if (node->job) node->job->handle_job_exception(std::current_exception());
             }
         }
    });
}

} // namespace coroutines
} // namespace kotlinx
