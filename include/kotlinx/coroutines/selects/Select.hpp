#pragma once
#include <functional>
#include <exception>
#include <vector>
#include <atomic>
#include "kotlinx/coroutines/CoroutineContext.hpp"

namespace kotlinx {
namespace coroutines {
namespace selects {

// Forward declare SelectInstance for use in aliases
template <typename R> class SelectInstance;

using OnCancellationHandler = std::function<void(
    std::exception_ptr throwable,
    void* value,
    CoroutineContext context
)>;

using OnCancellationConstructor = std::function<OnCancellationHandler(
    SelectInstance<void*>* select,
    void* param,
    void* internalResult
)>;

using RegistrationFunction = std::function<void(void* clause, SelectInstance<void*>* select, void* param)>;

using ProcessResultFunction = std::function<void*(void* clause, void* param, void* clauseResult)>;

/**
 * Common interface for all select clauses.
 */
class SelectClause {
public:
    virtual ~SelectClause() = default;
    virtual void* get_clause_object() const = 0;
    virtual RegistrationFunction get_reg_func() const = 0;
    virtual ProcessResultFunction get_process_res_func() const = 0;
    virtual const OnCancellationConstructor* get_on_cancellation_constructor() const = 0;
};

/**
 * Clause for [select] expression of [SelectBuilder] without additional parameters.
 */
class SelectClause0 : public SelectClause {
public:
    // Implementation details would go here
};

/**
 * Clause for [select] expression of [SelectBuilder] with one parameter of type [P].
 */
template <typename P>
class SelectClause1 : public SelectClause {
public:
    // Implementation details...
};

/**
 * Clause for [select] expression of [SelectBuilder] with two parameters of type [P] and [Q].
 */
template <typename P, typename Q>
class SelectClause2 : public SelectClause {
public:
    // Implementation details...
};

/**
 * Instance of select operation.
 */
template <typename R>
class SelectInstance {
public:
    virtual ~SelectInstance() = default;

    /**
     * Registers a clause in this select instance.
     */
    virtual void register_clause(SelectClause* clause, std::function<R()> block) = 0;
    virtual void register_clause(SelectClause* clause, std::function<R(void*)> block) = 0; 
    
    /**
     * Tries to select this instance.
     */
    virtual bool try_select(void* clauseObject, void* result) = 0;
    
    /**
     * Completes this select instance with the result.
     */
    virtual void complete_with(R result) = 0;
    
    /**
     * Disposes the specified disposable handle on completion of this select instance.
     */
    virtual void dispose_on_completion(void* disposable) = 0;
    
    /**
     * Returns the context of this select instance.
     */
    virtual CoroutineContext get_context() const = 0;
    
    /**
     * Selects this instance during registration phase.
     */
    virtual void select_in_registration_phase(void* internalResult) = 0;
};

/**
 * Builder for the [select] expression.
 */
template <typename R>
class SelectBuilder {
    SelectInstance<R>* impl_;
public:
    explicit SelectBuilder(SelectInstance<R>* impl) : impl_(impl) {}

    /**
     * Registers a clause with no parameters.
     */
    void invoke(SelectClause0& clause, std::function<R()> block) {
        impl_->register_clause(&clause, block);
    }

    /**
     * Registers a clause with one parameter.
     */
    template<typename Q>
    void invoke(SelectClause1<Q>& clause, std::function<R(Q)> block) {
       auto wrapped_block = [block](void* val) -> R {
           return block(*static_cast<Q*>(val)); 
       };
       impl_->register_clause(&clause, wrapped_block);
    }

    /**
     * Registers a clause with two parameters.
     */
    template<typename P, typename Q>
    void invoke(SelectClause2<P, Q>& clause, P param, std::function<R(Q)> block) {
        auto wrapped_block = [block](void* val) -> R {
            return block(*static_cast<Q*>(val));
        };
        // Note: Implementation of 2-param registration logic is simplified here
    }
    
    /**
     * Registers a clause that selects on timeout.
     */
    void on_timeout(long long timeMillis, std::function<R()> block) {
        // Register timeout
    }
};

/**
 * Waits for the result of multiple suspending functions simultaneously, which are specified using [builder]
 * in the [select] expression.
 *
 * The [builder] is a lambda that registers clauses for the select expression.
 *
 * This function is suspending and cancellable.
 * If the [Job] of the current coroutine is cancelled or completed while this function is suspended, this function
 * throws [CancellationException].
 */
template<typename R, typename BuilderFunc>
R select(BuilderFunc&& builder) {
    throw std::runtime_error("select() is not fully implemented in C++ runtime yet");
}

} // namespace selects
} // namespace coroutines
} // namespace kotlinx
