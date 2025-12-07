#pragma once
#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <exception>
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"

namespace kotlinx {
namespace coroutines {
namespace selects {

// Forward declarations
template<typename R> class SelectBuilder;
class SelectClause;
class SelectClause0;
template<typename Q> class SelectClause1;
template<typename P, typename Q> class SelectClause2;
template<typename R> class SelectInstance;
class DisposableHandle;

using RegistrationFunction = std::function<void(void* /* clauseObject */, SelectInstance<void*>* /* select */, void* /* param */)>;
using ProcessResultFunction = std::function<void*(void* /* clauseObject */, void* /* param */, void* /* clauseResult */)>;
using OnCancellationHandler = std::function<void(std::exception_ptr /*throwable*/, void* /*value*/, CoroutineContext /*context*/)>;
using OnCancellationConstructor = std::function<OnCancellationHandler(SelectInstance<void*>* /*select*/, void* /*param*/, void* /*internalResult*/)>;

/**
 * Scope for [select] invocation.
 */
template<typename R>
class SelectBuilder {
public:
    virtual ~SelectBuilder() = default;
    virtual void invoke(SelectClause0& clause, std::function<R()> block) = 0;
    template<typename Q>
    virtual void invoke(SelectClause1<Q>& clause, std::function<R(Q)> block) = 0; // implementation in derived class
    template<typename P, typename Q>
    virtual void invoke(SelectClause2<P, Q>& clause, P param, std::function<R(Q)> block) = 0; // implementation in derived class
    
    void on_timeout(long time_millis, std::function<R()> block);
};

class SelectClause {
public:
    virtual ~SelectClause() = default;
    virtual void* get_clause_object() const = 0;
    virtual RegistrationFunction get_reg_func() const = 0;
    virtual ProcessResultFunction get_process_res_func() const = 0;
    virtual OnCancellationConstructor* get_on_cancellation_constructor() const = 0;
};

class SelectClause0 : public SelectClause {
public:
    virtual ~SelectClause0() = default;
};

template<typename Q>
class SelectClause1 : public SelectClause {
public:
    virtual ~SelectClause1() = default;
};

template<typename P, typename Q>
class SelectClause2 : public SelectClause {
public:
    virtual ~SelectClause2() = default;
};

template<typename R>
class SelectInstance {
public:
    virtual ~SelectInstance() = default;
    virtual CoroutineContext get_context() const = 0;
    virtual bool try_select(void* clause_object, void* result) = 0;
    virtual void dispose_on_completion(DisposableHandle* disposable_handle) = 0;
    virtual void select_in_registration_phase(void* internal_result) = 0;
};

/**
 * Waits for the result of multiple suspending functions simultaneously.
 */
template<typename R, typename BuilderFunc>
R select(BuilderFunc&& builder) {
    // Stub implementation
    return R{};
}

} // namespace selects
} // namespace coroutines
} // namespace kotlinx
