#pragma once
#include <functional>
#include <exception>
#include <vector>
#include <atomic>
#include <mutex>
#include <memory>
#include <algorithm>
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/DisposableHandle.hpp"

namespace kotlinx {
namespace coroutines {
namespace selects {

// Forward declarations
template <typename R> class SelectInstance;

using RegistrationFunction = std::function<void(void* clause, SelectInstance<void*>* select, void* param)>;
using ProcessResultFunction = std::function<void*(void* clause, void* param, void* clauseResult)>;
// Simplified OnCancellationConstructor: we might not support customized cancellation handlers strictly yet
using OnCancellationConstructor = std::function<void(void*)>; 

/**
 * Common interface for all select clauses.
 */
class SelectClause {
public:
    virtual ~SelectClause() = default;
    virtual void* get_clause_object() const = 0;
    virtual RegistrationFunction get_reg_func() const = 0;
    virtual ProcessResultFunction get_process_res_func() const = 0;
};

/**
 * Clause for [select] expression of [SelectBuilder] without additional parameters.
 */
class SelectClause0 : public SelectClause {
public:
    // Implementation details handles by template subclasses usually
};

/**
 * Clause for [select] expression of [SelectBuilder] with one parameter of type [P].
 */
template <typename P>
class SelectClause1 : public SelectClause {
};

/**
 * Clause for [select] expression of [SelectBuilder] with two parameters of type [P] and [Q].
 */
template <typename P, typename Q>
class SelectClause2 : public SelectClause {
};

// Implementations of clauses
class SelectClause0Impl : public SelectClause0 {
    void* clauseObject;
    RegistrationFunction regFunc;
    ProcessResultFunction processFunc;
public:
    SelectClause0Impl(void* obj, RegistrationFunction reg, ProcessResultFunction proc) 
        : clauseObject(obj), regFunc(reg), processFunc(proc) {}
    
    void* get_clause_object() const override { return clauseObject; }
    RegistrationFunction get_reg_func() const override { return regFunc; }
    ProcessResultFunction get_process_res_func() const override { return processFunc; }
};

template <typename Q>
class SelectClause1Impl : public SelectClause1<Q> {
    void* clauseObject;
    RegistrationFunction regFunc;
    ProcessResultFunction processFunc;
public:
    SelectClause1Impl(void* obj, RegistrationFunction reg, ProcessResultFunction proc)
        : clauseObject(obj), regFunc(reg), processFunc(proc) {}

    void* get_clause_object() const override { return clauseObject; }
    RegistrationFunction get_reg_func() const override { return regFunc; }
    ProcessResultFunction get_process_res_func() const override { return processFunc; }
};

template <typename P, typename Q>
class SelectClause2Impl : public SelectClause2<P, Q> {
    void* clauseObject;
    RegistrationFunction regFunc;
    ProcessResultFunction processFunc;
public:
    SelectClause2Impl(void* obj, RegistrationFunction reg, ProcessResultFunction proc)
        : clauseObject(obj), regFunc(reg), processFunc(proc) {}

    void* get_clause_object() const override { return clauseObject; }
    RegistrationFunction get_reg_func() const override { return regFunc; }
    ProcessResultFunction get_process_res_func() const override { return processFunc; }
};

/**
 * Instance of select operation.
 */
template <typename R>
class SelectInstance {
public:
    virtual ~SelectInstance() = default;

    virtual bool try_select(void* clauseObject, void* result) = 0;
    virtual void dispose_on_completion(std::shared_ptr<DisposableHandle> handle) = 0;
    virtual void select_in_registration_phase(void* internalResult) = 0;
    virtual std::shared_ptr<CancellableContinuation<R>> get_continuation() = 0;
};

// Concrete implementation
template <typename R>
class SelectImplementation : public SelectInstance<R>, public std::enable_shared_from_this<SelectImplementation<R>> {
    enum State { REGISTRATION, WAITING, SELECTED, COMPLETED, CANCELLED };
    
    struct ClauseData {
        SelectClause* clause;
        std::function<R(void*)> block; // Wrapper that takes internal result and returns R
        void* param; 
        std::shared_ptr<DisposableHandle> disposable;
    };

    std::recursive_mutex mutex;
    State state = REGISTRATION;
    std::vector<ClauseData> clauses;
    std::shared_ptr<CancellableContinuation<R>> continuation;
    ClauseData* selectedClause = nullptr;
    void* internalResult = nullptr;
    
public:
    SelectImplementation(std::shared_ptr<CancellableContinuation<R>> cont) : continuation(cont) {}

    void register_clause(SelectClause* clause, std::function<R(void*)> block, void* param = nullptr) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (state != REGISTRATION) {
             // Should not happen if builder used correctly
             return;
        }
        clauses.push_back({clause, block, param, nullptr});
        // Invoke registration immediately
        // Note: Kotlin registers all, then checks.
        // We register here. regFunc takes (clauseObj, selectInstance, param)
        // We cast 'this' to SelectInstance<void*>* because typically external APIs work with void* or generic
        clause->get_reg_func()(clause->get_clause_object(), reinterpret_cast<SelectInstance<void*>*>(this), param);
    }
    
    // Called by external object (Channel/Job) to claim selection
    bool try_select(void* clauseObject, void* result) override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (state == SELECTED || state == COMPLETED || state == CANCELLED) return false;
        
        // Find clause
        auto it = std::find_if(clauses.begin(), clauses.end(), [&](const ClauseData& c){
            return c.clause->get_clause_object() == clauseObject;
        });
        
        if (it != clauses.end()) {
            state = SELECTED;
            selectedClause = &(*it);
            internalResult = result;
            return true;
        }
        return false;
    }
    
    void dispose_on_completion(std::shared_ptr<DisposableHandle> handle) override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (!clauses.empty()) {
            clauses.back().disposable = handle;
        }
    }
    
    void select_in_registration_phase(void* res) override {
         try_select(clauses.back().clause->get_clause_object(), res);
    }
    
    std::shared_ptr<CancellableContinuation<R>> get_continuation() override {
        return continuation;
    }
    
    // Core logic called by select()
    R do_select() {
        /*
         * TODO: STUB - Select suspension not fully implemented
         *
         * Kotlin source: SelectImplementation in Select.kt
         *
         * What's missing:
         * - Full state machine for select expression:
         *   1. REGISTRATION: Register all clauses with their selectable objects
         *   2. If any clause immediately ready, transition to SELECTED
         *   3. If none ready, transition to WAITING and suspend
         *   4. When a clause becomes ready, try_select() is called
         *   5. First successful try_select() wins, resume continuation
         * - Proper integration with suspendCancellableCoroutine
         * - Cancellation handling (dispose all non-selected clauses)
         *
         * Current behavior: Throws if no clause selected during registration
         * Correct behavior: Suspend waiting for a clause to become ready
         *
         * Dependencies:
         * - Kotlin-style suspension (Continuation<void*>* parameter)
         * - Channel/Job/etc integration calling try_select() when ready
         * - Proper clause disposal on completion/cancellation
         */
        // 1. Check if already selected during registration
        {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (state == SELECTED) {
                state = COMPLETED;
                dispose_others();
                // Execute block
                return selectedClause->block(internalResult);
            }
            state = WAITING;
        }

        // Cannot suspend properly - throw error
        throw std::runtime_error("Do not call do_select directly, use select() wrapper");
    }
    
    void resume_if_waiting() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (state == SELECTED && continuation && continuation->is_active()) {
            // value is produced by block? No, block is executed after resume usually in Kotlin's optimized select.
            // Simplified: we resume with INTERNAL result, then select wrapper executes block.
            // But R might not be void*.
            // We need to resume with a lambda or execute block here?
            // If we execute block here, we need to return R to resume.
            // But resume takes R.
            // Let's assume we execute block here and resume with result.
            // Or resume with "SelectClause" index/pointer and let main routine execute block.
            // Better: Resume with R.
            
            try {
                R res = selectedClause->block(internalResult);
                state = COMPLETED;
                dispose_others();
                continuation->resume(res);
            } catch (...) {
                state = COMPLETED; // or cancelled?
                dispose_others();
                continuation->resume_with_exception(std::current_exception());
            }
        }
    }
    
    void dispose_others() {
        for(auto& c : clauses) {
            if (&c != selectedClause && c.disposable) {
                c.disposable->dispose();
            }
        }
    }
};

/**
 * Builder for the [select] expression.
 */
template <typename R>
class SelectBuilder {
    std::shared_ptr<SelectImplementation<R>> impl;
public:
    explicit SelectBuilder(std::shared_ptr<SelectImplementation<R>> i) : impl(i) {}

    void invoke(SelectClause0& clause, std::function<R()> block) {
        impl->register_clause(&clause, [block](void*) { return block(); });
    }

    template<typename Q>
    void invoke(SelectClause1<Q>& clause, std::function<R(Q)> block) {
       impl->register_clause(&clause, [block](void* val) -> R {
           // Provide safe cast
           return block((Q)val); // Assuming direct cast or pointer logic. Q usually T or T&.
           // In Channels, Q is often T. trySelect passes Generic.
           // Needs strict type safety ideally. For now assume void* cast.
       });
    }

    template<typename P, typename Q>
    void invoke(SelectClause2<P, Q>& clause, P param, std::function<R(Q)> block) {
        impl->register_clause(&clause, [block](void* val) -> R {
             return block((Q)val); 
        }, (void*)param); // Pass param to register if needed? SelectClause2 regFunc takes param.
        // Wait, 'register_clause' in Implementation calls regFunc passing param. 
        // Need to ensure param is passed correctly. P might not be void*.
        // CASTING WARNING.
    }
    
    void on_timeout(long long timeMillis, std::function<R()> block) {
        /*
         * TODO: STUB - Select onTimeout not implemented
         *
         * Kotlin source: SelectBuilder.onTimeout() in Select.kt
         *
         * What's missing:
         * - Should register a timeout clause that triggers after timeMillis
         * - If no other clause is selected within timeout, execute block
         * - Requires: timer/delay infrastructure integration
         * - Requires: scheduling timeout cancellation when another clause wins
         *
         * Current behavior: Does nothing - timeout clause is ignored
         * Correct behavior: Start timer, if expires before other clauses, select timeout
         *
         * Dependencies:
         * - Delay/timer infrastructure (similar to delay() suspend function)
         * - Integration with SelectImplementation state machine
         * - DisposableHandle for timer cancellation
         *
         * Workaround: Use external timeout wrapper around select{}
         */
        (void)timeMillis;
        (void)block;
    }
};

/*
 * TODO: STUB - select() expression partially implemented
 *
 * Kotlin source: select() in Select.kt
 *
 * What's missing:
 * - suspendCancellableCoroutine is not properly integrated
 * - After registration, should check if any clause was immediately selected
 * - If selected: resume immediately with result (fast path)
 * - If not selected: suspend and wait for try_select() call from clause
 * - Proper cancellation propagation to all registered clauses
 *
 * Current behavior: Calls suspendCancellableCoroutine but doesn't handle
 *   the result properly - relies on clause registration side effects
 * Correct behavior: Full select state machine with proper fast/slow paths
 *
 * Dependencies:
 * - Working suspendCancellableCoroutine<R>()
 * - Channel/Job onReceive/onJoin/onSend clause implementations
 * - Proper SelectClause registration with selectable objects
 */
template<typename R, typename BuilderFunc>
R select(BuilderFunc&& builder) {
    return suspendCancellableCoroutine<R>([&](std::shared_ptr<CancellableContinuation<R>> cont) {
        auto impl = std::make_shared<SelectImplementation<R>>(cont);
        SelectBuilder<R> b(impl);
        builder(b);

        // After registration, check if already selected (fast path)
        // If not, the coroutine will suspend until a clause calls try_select()
        impl->resume_if_waiting();
    });
}

} // namespace selects
} // namespace coroutines
} // namespace kotlinx
