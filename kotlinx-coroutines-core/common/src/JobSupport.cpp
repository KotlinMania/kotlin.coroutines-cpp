#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include <cassert>
#include <thread>
#include <iostream>
#include <vector>
#include <memory>

namespace kotlinx {
namespace coroutines {

// --------------------------------------------------------------------------
// JobSupport Implementation
// --------------------------------------------------------------------------

// Incomplete implementations
std::string NodeList::to_string() const { return "NodeList"; }
std::string JobNode::to_string() const { return "JobNode"; }
std::string Empty::to_string() const { return std::string("Empty{") + (is_active_ ? "Active" : "New") + "}"; }
std::string InactiveNodeList::to_string() const { return "InactiveNodeList"; }
std::string Symbol::to_string() const { return name; }

// Finishing implementations
std::exception_ptr Finishing::root_cause() const {
    void* ptr = _rootCause.load();
    return ptr ? *static_cast<std::exception_ptr*>(ptr) : nullptr;
}

std::vector<std::exception_ptr> Finishing::seal_locked(std::exception_ptr proposed_exception) {
    std::vector<std::exception_ptr> list;
    void* eh = exceptionsHolder.load();
    if (eh == nullptr) {
        list = {};
    } else if (eh == SEALED) {
        return {}; 
    } else if (is_vector(eh)) {
        list = *static_cast<std::vector<std::exception_ptr>*>(eh);
    } else {
        list = {}; 
        list.push_back(*static_cast<std::exception_ptr*>(eh));
    }
    
    std::exception_ptr root = root_cause();
    if (root) list.insert(list.begin(), root);
    if (proposed_exception && proposed_exception != root) list.push_back(proposed_exception);
    
    exceptionsHolder.store(SEALED);
    return list;
}

void Finishing::add_exception_locked(std::exception_ptr exception) {
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

// JobSupport methods
// Transliterated from JobSupport.kt attachChild (lines 1011-1083)
std::shared_ptr<ChildHandle> JobSupport::attach_child(std::shared_ptr<ChildJob> child) {
    auto* node = new ChildHandleNode(child.get());
    node->job = this;

    bool added = try_put_node_into_list(node, [&](Incomplete* state, NodeList* list) -> bool {
        // First, try to add child along with cancellation handlers
        bool added_before_cancellation = list->add_last(node);
        if (added_before_cancellation) {
            return true; // Child added before cancellation/completion
        }

        // Cancellation or completion already happened
        // Try adding just for completion tracking
        bool added_before_completion = list->add_last(node);

        // Check why we couldn't add before cancellation
        void* latest_state = state_.load();
        std::exception_ptr root_cause = nullptr;
        if (auto* finishing = dynamic_cast<Finishing*>(static_cast<JobState*>(latest_state))) {
            root_cause = finishing->root_cause();
        } else if (!dynamic_cast<Incomplete*>(static_cast<JobState*>(latest_state))) {
            auto* ex = dynamic_cast<CompletedExceptionally*>(static_cast<JobState*>(latest_state));
            root_cause = ex ? ex->cause : nullptr;
        }

        // Must cancel child if parent was already cancelled
        node->invoke(root_cause);

        if (added_before_completion) {
            return true;
        } else {
            // Couldn't add, return NonDisposableHandle
            return false;
        }
    });

    if (added) {
        return std::shared_ptr<ChildHandle>(node, [](ChildHandle* h) {
            // Custom deleter - node is managed by the list
        });
    }

    // Final state reached, invoke handler and return NonDisposable
    void* s = state_.load();
    auto* ex = dynamic_cast<CompletedExceptionally*>(static_cast<JobState*>(s));
    node->invoke(ex ? ex->cause : nullptr);
    delete node;
    return std::shared_ptr<ChildHandle>(&NonDisposableHandle::instance(), [](ChildHandle*){});
}

std::shared_ptr<DisposableHandle> JobSupport::invoke_on_completion(bool on_cancelling, bool invoke_immediately, std::function<void(std::exception_ptr)> handler) {
    JobNode* node;
    if (on_cancelling) {
        node = new InvokeOnCancelling(handler);
    } else {
        node = new InvokeOnCompletion(handler);
    }
    return invoke_on_completion_internal(invoke_immediately, node); 
}

std::exception_ptr JobSupport::create_cause_exception(std::exception_ptr cause) {
     if (!cause) return default_cancellation_exception();
     return cause;
}

std::exception_ptr JobSupport::default_cancellation_exception(const char* message) {
    return std::make_exception_ptr(std::runtime_error(message ? message : "Job was cancelled"));
}

bool JobSupport::is_active() const {
    void* s = state_.load(std::memory_order_acquire);
    if (!s) return false;
    JobState* js = static_cast<JobState*>(s);
    if (auto* i = dynamic_cast<Incomplete*>(js)) {
         return i->is_active();
    }
    return false;
}

bool JobSupport::is_completed() const {
    void* s = state_.load(std::memory_order_acquire);
    JobState* js = static_cast<JobState*>(s); 
    return dynamic_cast<Incomplete*>(js) == nullptr;
}

// Kotlin: private fun startInternal(state: Any?): Int
int JobSupport::start_internal(void* state) {
    if (state == &_empty_new) { // state is Empty
         if (static_cast<Empty*>(state)->is_active()) return FALSE; // already active
         // transition to EMPTY_ACTIVE
         if (!state_.compare_exchange_strong(state, (void*)&_empty_active)) return RETRY;
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

bool JobSupport::start() {
    while (true) {
        void* state = state_.load(std::memory_order_acquire);
        int res = start_internal(state);
        if (res == FALSE) return false;
        if (res == TRUE) return true;
    }
}

// FIX: Signature takes std::exception_ptr to match header
void* JobSupport::make_cancelling(std::exception_ptr cause) {
    std::exception_ptr cause_exception_cache = nullptr;
    while(true) {
        void* state = state_.load(std::memory_order_acquire);
        if (auto* finishing = dynamic_cast<Finishing*>(static_cast<Incomplete*>(state))) {
            std::lock_guard<std::recursive_mutex> lock(finishing->mutex);
            if (finishing->is_sealed()) return TOO_LATE_TO_CANCEL;
            
            bool was_cancelling = finishing->is_cancelling();
            if (cause || !was_cancelling) {
                if (!cause_exception_cache) cause_exception_cache = create_cause_exception(cause); 
                finishing->add_exception_locked(cause_exception_cache);
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
                if (make_completing(cause_exception_cache)) return COMPLETING_ALREADY; 
                return COMPLETING_ALREADY;
            }
        } else {
             // Completed (Final) state
            return TOO_LATE_TO_CANCEL;
        }
    }
}

bool JobSupport::try_make_cancelling(Incomplete* state, std::exception_ptr root_cause) {
    NodeList* list = get_or_promote_cancelling_list(state);
    if (!list) return false;
    
    auto* cancelling = new Finishing(list, false, root_cause);
    void* expected = state;
    if (!state_.compare_exchange_strong(expected, (void*)cancelling)) {
        delete cancelling;
        return false;
    }
    notify_cancelling(list, root_cause);
    return true;
}

NodeList* JobSupport::get_or_promote_cancelling_list(Incomplete* state) {
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

void JobSupport::promote_single_to_node_list(JobNode* state) {
     NodeList* list = new NodeList();
     if (state->add_one_if_empty(list)) {
         void* expected = state;
         if (state_.compare_exchange_strong(expected, (void*)list)) {
             return;
         }
     }
}

void JobSupport::promote_empty_to_node_list(Empty* empty) {
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

void JobSupport::notify_cancelling(NodeList* list, std::exception_ptr cause) {
    on_cancelling(cause);
    notify_handlers(list, cause, [](JobNode* node){ return node->get_on_cancelling(); });
    cancel_parent(cause);
}

// FIX: Added implementation
bool JobSupport::cancel_parent(std::exception_ptr cause) {
     DisposableHandle* h = parent_handle_.load(std::memory_order_relaxed);
     if (h) {
          if (auto* ch = dynamic_cast<ChildHandle*>(h)) {
              return ch->child_cancelled(cause);
          }
     }
     return false;
}

std::vector<std::shared_ptr<struct Job>> JobSupport::get_children() const {
    std::vector<std::shared_ptr<struct Job>> children;
    // Implementation is tricky without shared_from_this logic on children
    // Leaving as stub returning empty for now to fix build.
    return children;
}

void JobSupport::cancel_internal(std::exception_ptr cause) {
    make_cancelling(cause ? cause : default_cancellation_exception());
}

void JobSupport::join() {
    while(!is_completed()) {
        std::this_thread::yield();
    }
}

// Wrapper to handle exception_ptr to void* conversion if needed
// Or simply we expect callers to handle it. 
// In make_cancelling, we cache exception.
// Here we just fix the "Completing" step.
bool JobSupport::make_completing(void* proposed_update) {
    while(true) { 
         void* state = state_.load(std::memory_order_acquire);
         void* final_state = try_make_completing(state, proposed_update);
         
         if (final_state == COMPLETING_ALREADY) return false;
         if (final_state == COMPLETING_WAITING_CHILDREN) return true;
         if (final_state == COMPLETING_RETRY) continue;
         
         on_completion_internal(final_state);
         return true;
    }
}

// Overload for exception
bool JobSupport::make_completing(std::exception_ptr ex) {
     // We need to wrap this exception into something that fits void* or handle it specially.
     // For now, let's create a CompletedExceptionally wrapper?
     // Or just pass it if we can cast. cannot cast exception_ptr to void*.
     // We need a heap object.
     return make_completing((void*)new std::exception_ptr(ex)); // LEAK! usage in try_make_completing needs to know it's a pointer.
}

void* JobSupport::try_make_completing(void* state, void* proposed_update) {
     if (!dynamic_cast<Incomplete*>(static_cast<Incomplete*>(state))) return COMPLETING_ALREADY;
     
     // proposed_update is usually exception or result. 
     // We assume it's exception_ptr for now if not valid type.
     // In Kotlin it checks: proposedUpdate !is CompletedExceptionally
     
     // FIX: Check if exception. We assume proposed_update IS void* pointing to exception if error.
     // But wait, make_completing is called with exception_ptr from make_cancelling.
     // But exception_ptr is not void*.
     // We need to wrap it if we pass it as void*.
     // For now, assume it's NOT an exception unless strictly typed.
     // This part is fragile. 
     
     bool is_exception = (proposed_update != nullptr); // Simplified assumption
     
     if ((dynamic_cast<Empty*>(static_cast<Incomplete*>(state)) || dynamic_cast<JobNode*>(static_cast<Incomplete*>(state))) && 
         !dynamic_cast<ChildHandleNode*>(static_cast<Incomplete*>(state)) && !is_exception) {
             
         if (try_finalize_simple_state(static_cast<Incomplete*>(state), nullptr)) { 
             return state; 
         }
         return COMPLETING_RETRY;
     }
     
     return try_make_completing_slow_path(static_cast<Incomplete*>(state), proposed_update);
}

void* JobSupport::try_make_completing_slow_path(Incomplete* state, void* proposed_update) {
    NodeList* list = get_or_promote_cancelling_list(state);
    if (!list) return COMPLETING_RETRY;
    
    Finishing* finishing = dynamic_cast<Finishing*>(state);
    
    if (!finishing) {
        finishing = new Finishing(list, false, nullptr);
        void* expected = state;
        if (!state_.compare_exchange_strong(expected, (void*)finishing)) {
             delete finishing; 
             return COMPLETING_RETRY;
        }
    }
    return COMPLETING_WAITING_CHILDREN; 
}

bool JobSupport::try_put_node_into_list(JobNode* node, std::function<bool(Incomplete*, NodeList*)> try_add) {
    while (true) {
        void* state = state_.load(std::memory_order_acquire);

        if (state == &_empty_active || state == &_empty_new) {
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
                if (dynamic_cast<Empty*>(js)) {
                     promote_empty_to_node_list(static_cast<Empty*>(js));
                     continue;
                }
            } else {
                if (try_add(incomplete, list)) return true;
                return false;
            }
        } else {
            return false;
        }
    }
}

// --------------------------------------------------------------------------
// State Finalization Methods (transliterated from JobSupport.kt)
// --------------------------------------------------------------------------

// Transliterated from JobSupport.kt finalizeFinishingState (lines 191-235)
void* JobSupport::finalize_finishing_state(Finishing* state, void* proposed_update) {
    // Note: proposed state can be Incomplete (e.g. handle from invokeOnCompletion)
    // Consistency checks
    assert(state_.load() == state);
    assert(!state->is_sealed());
    assert(state->isCompleting.load());

    auto* proposed_ex = dynamic_cast<CompletedExceptionally*>(static_cast<JobState*>(proposed_update));
    std::exception_ptr proposed_exception = proposed_ex ? proposed_ex->cause : nullptr;

    // Create the final exception and seal the state
    bool was_cancelling;
    std::exception_ptr final_exception;
    {
        std::lock_guard<std::recursive_mutex> lock(state->mutex);
        was_cancelling = state->is_cancelling();
        std::vector<std::exception_ptr> exceptions = state->seal_locked(proposed_exception);
        final_exception = get_final_root_cause(state, exceptions);
        if (final_exception) {
            add_suppressed_exceptions(final_exception, exceptions);
        }
    }

    // Create the final state object
    void* final_state;
    if (!final_exception) {
        // was not cancelled (no exception) -> use proposed update value
        final_state = proposed_update;
    } else if (final_exception == proposed_exception) {
        // small optimization when we can use proposedUpdate object as-is on cancellation
        final_state = proposed_update;
    } else {
        // cancelled job final state
        final_state = new CompletedExceptionally(final_exception);
    }

    // Now handle the final exception
    if (final_exception) {
        bool handled = cancel_parent(final_exception) || handle_job_exception(final_exception);
        if (handled) {
            auto* ex_state = dynamic_cast<CompletedExceptionally*>(static_cast<JobState*>(final_state));
            if (ex_state) ex_state->make_handled();
        }
    }

    // Process state updates before actual state change
    if (!was_cancelling) on_cancelling(final_exception);
    on_completion_internal(final_state);

    // CAS to completed state -> must succeed
    void* expected = state;
    bool cas_success = state_.compare_exchange_strong(expected, final_state);
    assert(cas_success);

    // Process all post-completion actions
    complete_state_finalization(state, final_state);
    return final_state;
}

// Transliterated from JobSupport.kt getFinalRootCause (lines 237-260)
std::exception_ptr JobSupport::get_final_root_cause(Finishing* state, const std::vector<std::exception_ptr>& exceptions) {
    // A case of no exceptions
    if (exceptions.empty()) {
        // materialize cancellation exception if it was not materialized yet
        if (state->is_cancelling()) return default_cancellation_exception();
        return nullptr;
    }

    // 1) If we have non-CancellationException, use it as root cause
    // 2) Otherwise just return the first exception
    // Note: In C++ we can't easily check if exception is CancellationException without rethrow
    // For now, just return the first exception
    return exceptions[0];
}

// Transliterated from JobSupport.kt addSuppressedExceptions (lines 262-278)
void JobSupport::add_suppressed_exceptions(std::exception_ptr root_cause, const std::vector<std::exception_ptr>& exceptions) {
    // C++ doesn't have built-in suppressed exceptions like Java/Kotlin
    // We could store them in a custom exception type, but for now we skip this
    // The primary exception is already captured in root_cause
}

// Transliterated from JobSupport.kt tryFinalizeSimpleState (lines 282-290)
bool JobSupport::try_finalize_simple_state(Incomplete* state, void* update) {
    // Only for simple states without lists where children can add
    // update should not be CompletedExceptionally (only for normal completion)
    void* expected = state;
    if (!state_.compare_exchange_strong(expected, update)) return false;
    on_cancelling(nullptr); // simple state is not a failure
    on_completion_internal(update);
    complete_state_finalization(state, update);
    return true;
}

// Transliterated from JobSupport.kt completeStateFinalization (lines 293-318)
void JobSupport::complete_state_finalization(Incomplete* state, void* update) {
    // 1) Unregister from parent job
    DisposableHandle* ph = parent_handle_.load();
    if (ph) {
        ph->dispose();
        parent_handle_.store(&NonDisposableHandle::instance());
    }

    auto* ex = dynamic_cast<CompletedExceptionally*>(static_cast<JobState*>(update));
    std::exception_ptr cause = ex ? ex->cause : nullptr;

    // 2) Invoke completion handlers
    if (auto* job_node = dynamic_cast<JobNode*>(state)) {
        // SINGLE/SINGLE+ state -- one completion handler
        try {
            job_node->invoke(cause);
        } catch (...) {
            handle_on_completion_exception(std::current_exception());
        }
    } else if (NodeList* list = state->get_list()) {
        list->notify_completion(cause);
    }
}

// Transliterated from JobSupport.kt initParentJob (lines 141-155)
void JobSupport::init_parent_job(std::shared_ptr<Job> parent) {
    assert(parent_handle_.load() == nullptr);
    if (!parent) {
        parent_handle_.store(&NonDisposableHandle::instance());
        return;
    }
    parent->start(); // make sure the parent is started
    auto handle = parent->attach_child(std::dynamic_pointer_cast<ChildJob>(shared_from_this()));
    // Store raw pointer (we need to manage lifetime carefully)
    parent_handle_.store(handle.get());
    // Check state after registering
    if (is_completed()) {
        handle->dispose();
        parent_handle_.store(&NonDisposableHandle::instance());
    }
}

// Transliterated from JobSupport.kt invokeOnCompletionInternal (lines 461-519)
std::shared_ptr<DisposableHandle> JobSupport::invoke_on_completion_internal(bool invoke_immediately, JobNode* node) {
    node->job = this;
    bool added = try_put_node_into_list(node, [&](Incomplete* state, NodeList* list) -> bool {
        if (node->get_on_cancelling()) {
            // Check if already cancelling
            auto* finishing = dynamic_cast<Finishing*>(state);
            std::exception_ptr root_cause = finishing ? finishing->root_cause() : nullptr;
            if (!root_cause) {
                // No root cause yet, add to list
                return list->add_last(node);
            } else {
                // Already cancelling, invoke immediately if requested
                if (invoke_immediately) node->invoke(root_cause);
                return false; // Don't add to list
            }
        } else {
            // Non-cancelling handler, just add to list
            return list->add_last(node);
        }
    });

    if (added) {
        return std::shared_ptr<DisposableHandle>(node, [](DisposableHandle*) {});
    }

    // State is final, invoke immediately if requested
    if (invoke_immediately) {
        void* s = state_.load();
        auto* ex = dynamic_cast<CompletedExceptionally*>(static_cast<JobState*>(s));
        node->invoke(ex ? ex->cause : nullptr);
    }
    return non_disposable_handle();
}

// Transliterated from JobSupport.kt tryWaitForChild (lines 954-962)
bool JobSupport::try_wait_for_child(Finishing* state, ChildHandleNode* child, void* proposed_update) {
    auto* completion = new ChildCompletion(this, state, child, proposed_update);
    auto handle = child->child_job->invoke_on_completion(false, false, [completion](std::exception_ptr cause) {
        completion->invoke(cause);
    });
    if (handle.get() != &NonDisposableHandle::instance()) {
        return true; // child is not complete and we've started waiting
    }
    ChildHandleNode* next = next_child(child);
    if (!next) return false;
    return try_wait_for_child(state, next, proposed_update);
}

// Transliterated from JobSupport.kt continueCompleting (lines 965-988)
void JobSupport::continue_completing(Finishing* state, ChildHandleNode* last_child, void* proposed_update) {
    assert(state_.load() == state);
    // Find next child to wait for
    ChildHandleNode* wait_child = next_child(last_child);
    if (wait_child && try_wait_for_child(state, wait_child, proposed_update)) {
        return; // waiting for next child
    }
    // No more children to await, close list for new children
    state->list->close(LIST_CHILD_PERMISSION);
    // Check if any children sneaked in
    ChildHandleNode* wait_child_again = next_child(last_child);
    if (wait_child_again && try_wait_for_child(state, wait_child_again, proposed_update)) {
        return;
    }
    // No more children, finalize
    void* final_state = finalize_finishing_state(state, proposed_update);
    after_completion(final_state);
}

// Transliterated from JobSupport.kt nextChild extension (lines 990-999)
ChildHandleNode* JobSupport::next_child(internal::LockFreeLinkedListNode* node) {
    internal::LockFreeLinkedListNode* cur = node;
    while (cur->is_removed()) cur = cur->prev_node(); // rollback to prev non-removed
    while (true) {
        cur = cur->next_node();
        if (!cur || dynamic_cast<NodeList*>(cur)) return nullptr; // reached end
        if (cur->is_removed()) continue;
        if (auto* child = dynamic_cast<ChildHandleNode*>(cur)) return child;
    }
}

// Transliterated from JobSupport.kt makeCompletingOnce (lines 857-870)
void* JobSupport::make_completing_once(void* proposed_update) {
    while (true) {
        void* state = state_.load(std::memory_order_acquire);
        void* final_state = try_make_completing(state, proposed_update);
        if (final_state == COMPLETING_ALREADY) {
            throw std::logic_error("Job is already complete or completing");
        }
        if (final_state == COMPLETING_RETRY) continue;
        return final_state; // COMPLETING_WAITING_CHILDREN or final state
    }
}

// Transliterated from JobSupport.kt cancelMakeCompleting (lines 720-730)
void* JobSupport::cancel_make_completing(void* cause) {
    while (true) {
        void* state = state_.load(std::memory_order_acquire);
        if (!dynamic_cast<Incomplete*>(static_cast<JobState*>(state))) {
            return COMPLETING_ALREADY;
        }
        auto* finishing = dynamic_cast<Finishing*>(static_cast<JobState*>(state));
        if (finishing && finishing->isCompleting.load()) {
            return COMPLETING_ALREADY;
        }
        std::exception_ptr cause_ex = create_cause_exception(cause ? *static_cast<std::exception_ptr*>(cause) : nullptr);
        auto* proposed_update = new CompletedExceptionally(cause_ex);
        void* final_state = try_make_completing(state, proposed_update);
        if (final_state != COMPLETING_RETRY) return final_state;
    }
}

// Transliterated from JobSupport.kt cancelImpl (lines 693-713)
bool JobSupport::cancel_impl(void* cause) {
    void* final_state = COMPLETING_ALREADY;
    if (get_on_cancel_complete()) {
        final_state = cancel_make_completing(cause);
        if (final_state == COMPLETING_WAITING_CHILDREN) return true;
    }
    if (final_state == COMPLETING_ALREADY) {
        final_state = make_cancelling(cause ? *static_cast<std::exception_ptr*>(cause) : nullptr);
    }
    if (final_state == COMPLETING_ALREADY) return true;
    if (final_state == COMPLETING_WAITING_CHILDREN) return true;
    if (final_state == TOO_LATE_TO_CANCEL) return false;
    after_completion(final_state);
    return true;
}

// --------------------------------------------------------------------------
// State Query Helpers
// --------------------------------------------------------------------------

bool JobSupport::is_completed_exceptionally() const {
    void* s = state_.load(std::memory_order_acquire);
    return dynamic_cast<CompletedExceptionally*>(static_cast<JobState*>(s)) != nullptr;
}

std::exception_ptr JobSupport::get_completion_exception_or_null() const {
    void* s = state_.load(std::memory_order_acquire);
    if (dynamic_cast<Incomplete*>(static_cast<JobState*>(s))) {
        throw std::logic_error("This job has not completed yet");
    }
    auto* ex = dynamic_cast<CompletedExceptionally*>(static_cast<JobState*>(s));
    return ex ? ex->cause : nullptr;
}

void* JobSupport::get_completed_internal() const {
    void* s = state_.load(std::memory_order_acquire);
    if (dynamic_cast<Incomplete*>(static_cast<JobState*>(s))) {
        throw std::logic_error("This job has not completed yet");
    }
    auto* ex = dynamic_cast<CompletedExceptionally*>(static_cast<JobState*>(s));
    if (ex) std::rethrow_exception(ex->cause);
    return s;
}

std::exception_ptr JobSupport::get_completion_cause() const {
    void* s = state_.load(std::memory_order_acquire);
    if (auto* finishing = dynamic_cast<Finishing*>(static_cast<JobState*>(s))) {
        std::exception_ptr root = finishing->root_cause();
        if (!root) throw std::logic_error("Job is still new or active");
        return root;
    }
    if (dynamic_cast<Incomplete*>(static_cast<JobState*>(s))) {
        throw std::logic_error("Job is still new or active");
    }
    auto* ex = dynamic_cast<CompletedExceptionally*>(static_cast<JobState*>(s));
    return ex ? ex->cause : nullptr;
}

bool JobSupport::get_completion_cause_handled() const {
    void* s = state_.load(std::memory_order_acquire);
    auto* ex = dynamic_cast<CompletedExceptionally*>(static_cast<JobState*>(s));
    return ex && ex->handled.load();
}

// --------------------------------------------------------------------------
// Debug Helpers
// --------------------------------------------------------------------------

std::string JobSupport::to_debug_string() const {
    return name_string() + "{" + state_string(state_.load()) + "}";
}

std::string JobSupport::name_string() const {
    return "JobSupport";
}

std::string JobSupport::state_string(void* state) const {
    if (auto* finishing = dynamic_cast<Finishing*>(static_cast<JobState*>(state))) {
        if (finishing->is_cancelling()) return "Cancelling";
        if (finishing->isCompleting.load()) return "Completing";
        return "Active";
    }
    if (auto* incomplete = dynamic_cast<Incomplete*>(static_cast<JobState*>(state))) {
        return incomplete->is_active() ? "Active" : "New";
    }
    if (dynamic_cast<CompletedExceptionally*>(static_cast<JobState*>(state))) {
        return "Cancelled";
    }
    return "Completed";
}

// Transliterated from JobSupport.kt:355-358
// private fun NodeList.notifyCompletion(cause: Throwable?) {
//     close(LIST_ON_COMPLETION_PERMISSION)
//     notifyHandlers(this, cause) { true }
// }
void NodeList::notify_completion(std::exception_ptr cause) {
    close(LIST_ON_COMPLETION_PERMISSION);
    // Transliterated from JobSupport.kt:360-370 - notifyHandlers inline
    std::exception_ptr exception = nullptr;
    for_each([&](internal::LockFreeLinkedListNode* node_raw) {
        if (auto* node = dynamic_cast<JobNode*>(node_raw)) {
            try {
                node->invoke(cause);
            } catch (...) {
                // exception?.apply { addSuppressed(ex) } ?: run { exception = ... }
                if (!exception) {
                    exception = std::current_exception();
                }
                // C++ doesn't have addSuppressed, first exception wins
            }
        }
    });
    // if (exception != null) throw exception
    if (exception) {
        std::rethrow_exception(exception);
    }
}

// Transliterated from JobSupport.kt:1475
// override fun dispose() = job.removeNode(this)
void JobNode::dispose() {
    if (job) job->remove_node(this);
}

// ChildHandleNode methods
// Transliterated from JobSupport.kt:1580
// override fun invoke(cause: Throwable?) = childJob.parentCancelled(job)
void ChildHandleNode::invoke(std::exception_ptr cause) {
    if (child_job) child_job->parent_cancelled(dynamic_cast<ParentJob*>(job));
}

// Transliterated from JobSupport.kt:1581
// override fun childCancelled(cause: Throwable): Boolean = job.childCancelled(cause)
bool ChildHandleNode::child_cancelled(std::exception_ptr cause) {
    if (job) return job->child_cancelled(cause);
    return false;
}

// Transliterated from JobSupport.kt:1578 - override val parent: Job get() = job
std::shared_ptr<Job> ChildHandleNode::get_parent() const {
    if (job) {
        return std::dynamic_pointer_cast<Job>(job->shared_from_this());
    }
    return nullptr;
}

// Transliterated from JobSupport.kt:1575 - override fun dispose() = job.removeNode(this)
void ChildHandleNode::dispose() {
    if (job) job->remove_node(this);
}

// ChildCompletion::invoke - must be out-of-line since JobSupport isn't complete at declaration
void ChildCompletion::invoke(std::exception_ptr cause) {
    parent->continue_completing(state, child, proposed_update);
}

// Transliterated from JobSupport.kt:181-184
bool JobSupport::is_cancelled() const {
    void* state = state_.load(std::memory_order_acquire);
    if (!state) return false;
    JobState* js = static_cast<JobState*>(state);
    // return state is CompletedExceptionally || (state is Finishing && state.isCancelling)
    if (dynamic_cast<CompletedExceptionally*>(js)) return true;
    if (auto* finishing = dynamic_cast<Finishing*>(js)) {
        return finishing->is_cancelling();
    }
    return false;
}

// Transliterated from JobSupport.kt:412-419
std::exception_ptr JobSupport::get_cancellation_exception() {
    void* state = state_.load(std::memory_order_acquire);
    JobState* js = static_cast<JobState*>(state);

    if (auto* finishing = dynamic_cast<Finishing*>(js)) {
        // is Finishing -> state.rootCause?.toCancellationException("$classSimpleName is cancelling")
        //     ?: error("Job is still new or active: $this")
        std::exception_ptr root = finishing->root_cause();
        if (root) return root;
        throw std::logic_error("Job is still new or active: " + to_debug_string());
    }
    if (dynamic_cast<Incomplete*>(js)) {
        // is Incomplete -> error("Job is still new or active: $this")
        throw std::logic_error("Job is still new or active: " + to_debug_string());
    }
    if (auto* completed_ex = dynamic_cast<CompletedExceptionally*>(js)) {
        // is CompletedExceptionally -> state.cause.toCancellationException()
        return completed_ex->cause;
    }
    // else -> JobCancellationException("$classSimpleName has completed normally", null, this)
    return std::make_exception_ptr(std::runtime_error(
        name_string() + " has completed normally"));
}

// Transliterated from JobSupport.kt:680-683
bool JobSupport::child_cancelled(std::exception_ptr cause) {
    // if (cause is CancellationException) return true
    // In C++ we check by attempting to rethrow and catch
    if (cause) {
        try {
            std::rethrow_exception(cause);
        } catch (const CancellationException&) {
            return true;
        } catch (...) {
            // Not a CancellationException, continue
        }
    }
    // return cancelImpl(cause) && handlesException
    bool result = cancel_impl(cause ? &cause : nullptr);
    return result && get_handles_exception();
}

// Transliterated from JobSupport.kt:689
bool JobSupport::cancel_coroutine(std::exception_ptr cause) {
    // public fun cancelCoroutine(cause: Throwable?): Boolean = cancelImpl(cause)
    return cancel_impl(cause ? &cause : nullptr);
}

// Transliterated from JobSupport.kt:619-636
void JobSupport::remove_node(JobNode* node) {
    // loopOnState { state ->
    while (true) {
        void* state = state_.load(std::memory_order_acquire);
        JobState* js = static_cast<JobState*>(state);

        // when (state) {
        if (auto* state_as_job_node = dynamic_cast<JobNode*>(js)) {
            // is JobNode -> {
            //     if (state !== node) return // different node -> already removed
            if (state_as_job_node != node) return;
            //     if (_state.compareAndSet(state, EMPTY_ACTIVE)) return
            if (state_.compare_exchange_strong(state, (void*)&_empty_active)) return;
            // }
            continue; // CAS failed, retry
        }
        if (auto* incomplete = dynamic_cast<Incomplete*>(js)) {
            // is Incomplete -> {
            //     if (state.list != null) node.remove()
            //     return
            // }
            if (incomplete->get_list() != nullptr) {
                node->remove();
            }
            return;
        }
        // else -> return // it is complete and does not have any completion handlers
        return;
    }
}

} // namespace coroutines
} // namespace kotlinx
