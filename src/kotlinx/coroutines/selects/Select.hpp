/**
 * @file Select.hpp
 * @brief Select expression for coroutines
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/selects/Select.kt
 *
 * Waits for the result of multiple suspending functions simultaneously, which are specified
 * using _clauses_ in the builder scope of this select invocation. The caller is suspended
 * until one of the clauses is either _selected_ or _fails_.
 *
 * At most one clause is *atomically* selected and its block is executed. The result of the
 * selected clause becomes the result of the select. If any clause _fails_, then the select
 * invocation produces the corresponding exception. No clause is selected in this case.
 *
 * This select function is _biased_ to the first clause. When multiple clauses can be selected
 * at the same time, the first one of them gets priority. Use select_unbiased for an unbiased
 * (randomized) selection among the clauses.
 *
 * There is no `default` clause for select expression. Instead, each selectable suspending
 * function has the corresponding non-suspending version that can be used with a regular
 * conditional to select one of the alternatives or to perform the default action if none
 * of them can be immediately selected.
 *
 * ## List of supported select methods
 *
 * | Receiver       | Suspending function    | Select clause        |
 * |----------------|------------------------|----------------------|
 * | Job            | join                   | on_join              |
 * | Deferred<T>    | await                  | on_await             |
 * | SendChannel    | send                   | on_send              |
 * | ReceiveChannel | receive                | on_receive           |
 * | ReceiveChannel | receive_catching       | on_receive_catching  |
 * | none           | delay                  | on_timeout           |
 *
 * This suspending function is cancellable: if the Job of the current coroutine is cancelled
 * while this suspending function is waiting, this function immediately resumes with
 * CancellationException. There is a **prompt cancellation guarantee**: even if this function
 * is ready to return the result, but was cancelled while suspended, CancellationException
 * will be thrown.
 *
 * Note that this function does not check for cancellation when it is not suspended.
 * Use yield or CoroutineScope::is_active to periodically check for cancellation in tight
 * loops if needed.
 */
#pragma once

#include <atomic>
#include <vector>
#include <memory>
#include <functional>
#include <cassert>
#include <type_traits>

#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"  // CancelHandler defined here
#include "kotlinx/coroutines/DisposableHandle.hpp"
#include "kotlinx/coroutines/Waiter.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

namespace kotlinx {
namespace coroutines {
namespace selects {

// Forward declarations
template<typename R> class SelectInstance;
template<typename R> class SelectImplementation;
template<typename R> class SelectBuilder;

// =============================================================================
// =============================================================================
constexpr int TRY_SELECT_SUCCESSFUL = 0;
constexpr int TRY_SELECT_REREGISTER = 1;
constexpr int TRY_SELECT_CANCELLED = 2;
constexpr int TRY_SELECT_ALREADY_SELECTED = 3;

// =============================================================================
// =============================================================================
enum class TrySelectDetailedResult {
    SUCCESSFUL,
    REREGISTER,
    CANCELLED,
    ALREADY_SELECTED
};

inline TrySelectDetailedResult to_try_select_detailed_result(int result) {
    switch (result) {
        case TRY_SELECT_SUCCESSFUL: return TrySelectDetailedResult::SUCCESSFUL;
        case TRY_SELECT_REREGISTER: return TrySelectDetailedResult::REREGISTER;
        case TRY_SELECT_CANCELLED: return TrySelectDetailedResult::CANCELLED;
        case TRY_SELECT_ALREADY_SELECTED: return TrySelectDetailedResult::ALREADY_SELECTED;
        default: throw std::runtime_error("Unexpected internal result");
    }
}

// =============================================================================
// =============================================================================
namespace detail {
    struct StateMarker { const char* name; };
    inline StateMarker STATE_REG_MARKER{"STATE_REG"};
    inline StateMarker STATE_COMPLETED_MARKER{"STATE_COMPLETED"};
    inline StateMarker STATE_CANCELLED_MARKER{"STATE_CANCELLED"};
    inline StateMarker NO_RESULT_MARKER{"NO_RESULT"};
    inline StateMarker PARAM_CLAUSE_0_MARKER{"PARAM_CLAUSE_0"};
}

inline void* STATE_REG() { return &detail::STATE_REG_MARKER; }
inline void* STATE_COMPLETED() { return &detail::STATE_COMPLETED_MARKER; }
inline void* STATE_CANCELLED() { return &detail::STATE_CANCELLED_MARKER; }
inline void* NO_RESULT() { return &detail::NO_RESULT_MARKER; }
inline void* PARAM_CLAUSE_0() { return &detail::PARAM_CLAUSE_0_MARKER; }

/**
 * The registration function specifies how the select instance should be registered into
 * the specified clause object. In case of channels, the registration logic coincides with
 * the plain send/receive operation with the only difference that the select instance is
 * stored as a waiter instead of continuation.
 *
 * Transliterated from:
 * public typealias RegistrationFunction = (clauseObject: Any, select: SelectInstance<*>, param: Any?) -> Unit
 *
 * @note SelectInstance<*> becomes void* in C++ since we don't know R at clause level
 */
using RegistrationFunction = std::function<void(void* clause_object, void* select, void* param)>;

/**
 * This function specifies how the _internal_ result, provided via SelectInstance::select_in_registration_phase
 * or SelectInstance::try_select should be processed. For example, both ReceiveChannel::on_receive and
 * ReceiveChannel::on_receive_catching clauses perform exactly the same synchronization logic,
 * but differ when the channel has been discovered in the closed or cancelled state.
 *
 * Transliterated from:
 * public typealias ProcessResultFunction = (clauseObject: Any, param: Any?, clauseResult: Any?) -> Any?
 */
using ProcessResultFunction = std::function<void*(void* clause_object, void* param, void* clause_result)>;

/**
 * Action to execute on cancellation.
 */
using OnCancellationAction = std::function<void(std::exception_ptr, void*, std::shared_ptr<CoroutineContext>)>;

/**
 * This function specifies how the internal result, provided via SelectInstance::try_select
 * or SelectInstance::select_in_registration_phase, should be processed in case of this select
 * cancellation while dispatching. Unfortunately, we cannot pass this function only in try_select,
 * as select_in_registration_phase can be called when the coroutine is already cancelled.
 *
 * Transliterated from:
 * public typealias OnCancellationConstructor = (select: SelectInstance<*>, param: Any?, internalResult: Any?) ->
 *     (Throwable, Any?, CoroutineContext) -> Unit
 */
using OnCancellationConstructor = std::function<
    OnCancellationAction(void* select, void* param, void* internal_result)
>;

inline ProcessResultFunction dummy_process_result_function() {
    return [](void*, void*, void*) -> void* { return nullptr; };
}

/**
 * Each select clause is specified with:
 * 1) the object of this clause (clauseObject), such as the channel instance for SendChannel::on_send;
 * 2) the function that specifies how this clause should be registered in the object above (regFunc);
 * 3) the function that modifies the internal result (passed via SelectInstance::try_select or
 *    SelectInstance::select_in_registration_phase) to the argument of the user-specified block (processResFunc);
 * 4) the function that specifies how the internal result provided via SelectInstance::try_select or
 *    SelectInstance::select_in_registration_phase should be processed in case of this select cancellation
 *    while dispatching (onCancellationConstructor).
 *
 * @note This is unstable API, and it is subject to change.
 *
 * Transliterated from:
 * public sealed interface SelectClause
 */
class SelectClause {
public:
    virtual ~SelectClause() = default;

    virtual void* get_clause_object() const = 0;

    virtual RegistrationFunction get_reg_func() const = 0;

    virtual ProcessResultFunction get_process_res_func() const = 0;

    virtual OnCancellationConstructor get_on_cancellation_constructor() const = 0;
    virtual bool has_on_cancellation_constructor() const = 0;
};

/**
 * Clause for select expression without additional parameters that does not select any value.
 *
 * Transliterated from:
 * public sealed interface SelectClause0 : SelectClause
 */
class SelectClause0 : public SelectClause {};

// =============================================================================
// =============================================================================
class SelectClause0Impl : public SelectClause0 {
    void* clause_object_;
    RegistrationFunction reg_func_;
    OnCancellationConstructor on_cancellation_constructor_;
    bool has_on_cancellation_;

public:
    SelectClause0Impl(
        void* clause_object,
        RegistrationFunction reg_func,
        OnCancellationConstructor on_cancellation_constructor = nullptr
    ) : clause_object_(clause_object),
        reg_func_(std::move(reg_func)),
        on_cancellation_constructor_(std::move(on_cancellation_constructor)),
        has_on_cancellation_(on_cancellation_constructor_ != nullptr) {}

    void* get_clause_object() const override { return clause_object_; }
    RegistrationFunction get_reg_func() const override { return reg_func_; }
    ProcessResultFunction get_process_res_func() const override { return dummy_process_result_function(); }
    OnCancellationConstructor get_on_cancellation_constructor() const override { return on_cancellation_constructor_; }
    bool has_on_cancellation_constructor() const override { return has_on_cancellation_; }
};

/**
 * Clause for select expression without additional parameters that selects value of type Q.
 *
 * Transliterated from:
 * public sealed interface SelectClause1<out Q> : SelectClause
 */
template<typename Q>
class SelectClause1 : public SelectClause {};

// =============================================================================
// =============================================================================
template<typename Q>
class SelectClause1Impl : public SelectClause1<Q> {
    void* clause_object_;
    RegistrationFunction reg_func_;
    ProcessResultFunction process_res_func_;
    OnCancellationConstructor on_cancellation_constructor_;
    bool has_on_cancellation_;

public:
    SelectClause1Impl(
        void* clause_object,
        RegistrationFunction reg_func,
        ProcessResultFunction process_res_func,
        OnCancellationConstructor on_cancellation_constructor = nullptr
    ) : clause_object_(clause_object),
        reg_func_(std::move(reg_func)),
        process_res_func_(std::move(process_res_func)),
        on_cancellation_constructor_(std::move(on_cancellation_constructor)),
        has_on_cancellation_(on_cancellation_constructor_ != nullptr) {}

    void* get_clause_object() const override { return clause_object_; }
    RegistrationFunction get_reg_func() const override { return reg_func_; }
    ProcessResultFunction get_process_res_func() const override { return process_res_func_; }
    OnCancellationConstructor get_on_cancellation_constructor() const override { return on_cancellation_constructor_; }
    bool has_on_cancellation_constructor() const override { return has_on_cancellation_; }
};

// =============================================================================
// =============================================================================
template<typename P, typename Q>
class SelectClause2 : public SelectClause {};

// =============================================================================
// =============================================================================
template<typename P, typename Q>
class SelectClause2Impl : public SelectClause2<P, Q> {
    void* clause_object_;
    RegistrationFunction reg_func_;
    ProcessResultFunction process_res_func_;
    OnCancellationConstructor on_cancellation_constructor_;
    bool has_on_cancellation_;

public:
    SelectClause2Impl(
        void* clause_object,
        RegistrationFunction reg_func,
        ProcessResultFunction process_res_func,
        OnCancellationConstructor on_cancellation_constructor = nullptr
    ) : clause_object_(clause_object),
        reg_func_(std::move(reg_func)),
        process_res_func_(std::move(process_res_func)),
        on_cancellation_constructor_(std::move(on_cancellation_constructor)),
        has_on_cancellation_(on_cancellation_constructor_ != nullptr) {}

    void* get_clause_object() const override { return clause_object_; }
    RegistrationFunction get_reg_func() const override { return reg_func_; }
    ProcessResultFunction get_process_res_func() const override { return process_res_func_; }
    OnCancellationConstructor get_on_cancellation_constructor() const override { return on_cancellation_constructor_; }
    bool has_on_cancellation_constructor() const override { return has_on_cancellation_; }
};

/**
 * Internal representation of select instance.
 *
 * @note This is unstable API, and it is subject to change.
 *
 * Transliterated from:
 * public sealed interface SelectInstance<in R>
 */
template<typename R>
class SelectInstance {
public:
    virtual ~SelectInstance() = default;

    /**
     * The context of the coroutine that is performing this select operation.
     *
     * Transliterated from:
     * public val context: CoroutineContext
     */
    virtual std::shared_ptr<CoroutineContext> get_context() const = 0;

    /**
     * This function should be called by other operations, which are trying to perform a rendezvous
     * with this select. Returns true if the rendezvous succeeds, false otherwise.
     *
     * Note that according to the current implementation, a rendezvous attempt can fail when either
     * another clause is already selected or this select is still in REGISTRATION phase. To distinguish
     * the reasons, SelectImplementation::try_select_detailed function can be used instead.
     *
     * Transliterated from:
     * public fun trySelect(clauseObject: Any, result: Any?): Boolean
     */
    virtual bool try_select(void* clause_object, void* result) = 0;

    /**
     * When this select instance is stored as a waiter, the specified handle defines how the stored
     * select should be removed in case of cancellation or another clause selection.
     *
     * Transliterated from:
     * public fun disposeOnCompletion(disposableHandle: DisposableHandle)
     */
    virtual void dispose_on_completion(std::shared_ptr<DisposableHandle> handle) = 0;

    /**
     * When a clause becomes selected during registration, the corresponding internal result
     * (which is further passed to the clause's ProcessResultFunction) should be provided via this
     * function. After that, other clause registrations are ignored and try_select fails.
     *
     * Transliterated from:
     * public fun selectInRegistrationPhase(internalResult: Any?)
     */
    virtual void select_in_registration_phase(void* internal_result) = 0;
};

// =============================================================================
// =============================================================================
template<typename R>
class SelectInstanceInternal : public SelectInstance<R>, public Waiter {
public:
    // Waiter interface for segment-based cancellation
    // SegmentBase is the non-templated base for Segment<S>
    virtual void invoke_on_cancellation(internal::SegmentBase* segment, int index) = 0;
};

/**
 * Scope for select invocation.
 *
 * An instance of SelectBuilder can only be retrieved as a receiver of a select block call,
 * and it is only valid during the registration phase of the select builder.
 * Any uses outside it lead to unspecified behaviour and are prohibited.
 *
 * The general rule of thumb is that instances of this type should always be used
 * implicitly and there shouldn't be any signatures mentioning this type,
 * whether explicitly (e.g. function signature) or implicitly (e.g. inferred variable type).
 *
 * Transliterated from:
 * public sealed interface SelectBuilder<in R>
 */
template<typename R>
class SelectBuilder {
public:
    virtual ~SelectBuilder() = default;

    /**
     * Registers a clause in this select expression without additional parameters that does not select any value.
     *
     * Transliterated from:
     * public operator fun SelectClause0.invoke(block: suspend () -> R)
     */
    virtual void invoke(SelectClause0& clause, std::function<void*(Continuation<void*>*)> block) = 0;

    /**
     * Registers clause in this select expression without additional parameters that selects value of type Q.
     *
     * Transliterated from:
     * public operator fun <Q> SelectClause1<Q>.invoke(block: suspend (Q) -> R)
     */
    template<typename Q>
    void invoke(SelectClause1<Q>& clause, std::function<void*(Q, Continuation<void*>*)> block);

    /**
     * Registers clause in this select expression with additional parameter of type P that selects value of type Q.
     *
     * Transliterated from:
     * public operator fun <P, Q> SelectClause2<P, Q>.invoke(param: P, block: suspend (Q) -> R)
     */
    template<typename P, typename Q>
    void invoke(SelectClause2<P, Q>& clause, P param, std::function<void*(Q, Continuation<void*>*)> block);


};

/**
 * Internal implementation of the select expression.
 *
 * Essentially, the `select` operation is split into three phases: REGISTRATION, WAITING, and COMPLETION.
 *
 * == Phase 1: REGISTRATION ==
 * In the first REGISTRATION phase, the user-specified SelectBuilder is applied, and all the listed clauses
 * are registered via the provided registration functions. Intuitively, select clause registration is similar
 * to the plain blocking operation, with the only difference that this SelectInstance is stored as a waiter
 * instead of continuation, and SelectInstance::try_select is used to make a rendezvous. Also, when registering,
 * it is possible for the operation to complete immediately, without waiting. In this case,
 * SelectInstance::select_in_registration_phase should be used. Otherwise, when no rendezvous happens and this
 * select instance is stored as a waiter, a completion handler for the registering clause should be specified
 * via SelectInstance::dispose_on_completion; this handler specifies how to remove this select instance from
 * the clause object when another clause becomes selected or the operation cancels.
 *
 * After a clause registration is completed, another coroutine can attempt to make a rendezvous with this select.
 * However, to resolve a race between clauses registration and SelectInstance::try_select, the latter fails when
 * this select is still in REGISTRATION phase. Thus, the corresponding clause has to be re-registered.
 *
 * In this phase, the state field stores either a special STATE_REG marker or a list of clauses to be
 * re-registered due to failed rendezvous attempts.
 *
 * == Phase 2: WAITING ==
 * If no rendezvous happens in REGISTRATION phase, the select operation moves to WAITING phase and suspends
 * until SelectInstance::try_select is called. Also, when waiting, this select can be cancelled. In the latter
 * case, further SelectInstance::try_select attempts fail, and all the completion handlers, specified via
 * SelectInstance::dispose_on_completion, are invoked to remove this select instance from the corresponding
 * clause objects.
 *
 * In this phase, the state field stores either the continuation to be later resumed or a special Cancelled
 * object (with the cancellation cause inside) when this select becomes cancelled.
 *
 * == Phase 3: COMPLETION ==
 * Once a rendezvous happens either in REGISTRATION phase (via SelectInstance::select_in_registration_phase) or
 * in WAITING phase (via SelectInstance::try_select), this select moves to the final COMPLETION phase.
 * First, the provided internal result is processed via the ProcessResultFunction of the selected clause;
 * it returns the argument for the user-specified block or throws an exception (see SendChannel::on_send as
 * an example). After that, this select should be removed from all other clause objects by calling the
 * corresponding DisposableHandles, provided via SelectInstance::dispose_on_completion during registration.
 * At the end, the user-specified block is called and this select finishes.
 *
 * In this phase, once a rendezvous has happened, the state field stores the corresponding clause.
 * After that, it moves to STATE_COMPLETED to avoid memory leaks.
 *
 * The state machine is listed below:
 * ```
 *            REGISTRATION PHASE                   WAITING PHASE             COMPLETION PHASE
 *       ⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢             ⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢         ⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢⌢
 *
 *                                                 +-----------+                 +-----------+
 *                                                 | CANCELLED |                 | COMPLETED |
 *                                                 +-----------+                 +-----------+
 *                                                       ^                             ^
 *     INITIAL STATE                                     |                             | this select
 *     ------------+                                     |  cancelled                  | is completed
 *                  \                                    |                             |
 *                   +=============+     move to     +------+    successful   +------------+
 *                +--|  STATE_REG  |---------------> | cont |-----------------| ClauseData |
 *                |  +=============+  WAITING phase  +------+  try_select(..) +------------+
 *                |    ^     |                                                       ^
 *                |    |     |    some clause has been selected during registration  |
 *         add a  |    |     +-------------------------------------------------------+
 *   clause to be |    |                                                             |
 *  re-registered |    | re-register                   some clause has been selected |
 *                |    | clauses                     during registration while there |
 *                v    |                            are clauses to be re-registered; |
 *          +------------------+                                   ignore the latter |
 *       +--| List<ClauseData> |-----------------------------------------------------+
 *       |  +------------------+
 *       |            ^
 *       |            |  add one more clause
 *       |            |  for re-registration
 *       +------------+
 * ```
 *
 * One of the most valuable benefits of this select design is that it allows processing clauses in a way
 * similar to plain operations, such as send or receive on channels. The only difference is that instead
 * of continuation, the operation should store the provided select instance object. Thus, this design makes
 * it possible to support the select expression for any blocking data structure in Kotlin Coroutines.
 *
 * It is worth mentioning that the algorithm above provides "obstruction-freedom" non-blocking guarantee
 * instead of the standard "lock-freedom" to avoid using heavy descriptors. In practice, this relaxation
 * does not make significant difference. However, it is vital for Kotlin Coroutines to provide some
 * non-blocking guarantee, as users may add blocking code in SelectBuilder, and this blocking code should
 * not cause blocking behaviour in other places, such as an attempt to make a rendezvous with the select
 * that is hung in REGISTRATION phase.
 *
 * Also, this implementation is NOT linearizable under some circumstances. The reason is that a rendezvous
 * attempt with select (via SelectInstance::try_select) may fail when this select operation is still in
 * REGISTRATION phase. Consider the following situation on two empty rendezvous channels c1 and c2 and the
 * select operation that tries to send an element to one of these channels. First, this select instance is
 * registered as a waiter in c1. After that, another thread can observe that c1 is no longer empty and try
 * to receive an element from c1 -- this receive attempt fails due to the select operation being in
 * REGISTRATION phase. We, however, find such a non-linearizable behaviour not so important in practice and
 * leverage the correctness relaxation for the algorithm simplicity and the non-blocking progress guarantee.
 *
 * Transliterated from:
 * internal open class SelectImplementation<R>(override val context: CoroutineContext)
 *     : CancelHandler, SelectBuilder<R>, SelectInstanceInternal<R>
 */
template<typename R>
class SelectImplementation : public CancelHandler,
                             public SelectBuilder<R>,
                             public SelectInstanceInternal<R>,
                             public std::enable_shared_from_this<SelectImplementation<R>> {
public:
    /**
     * Each select clause is internally represented with a ClauseData instance.
     *
     * Transliterated from:
     * internal inner class ClauseData(...)
     */
    class ClauseData {
    public:
        void* const clause_object;

    private:
        RegistrationFunction reg_func_;
        ProcessResultFunction process_res_func_;
        void* param_;
        std::function<void*(void*, Continuation<void*>*)> block_;
        OnCancellationConstructor on_cancellation_constructor_;
        bool has_on_cancellation_;

    public:
        void* disposable_handle_or_segment = nullptr;
        int index_in_segment = -1;

        ClauseData(
            void* clause_obj,
            RegistrationFunction reg_func,
            ProcessResultFunction process_res_func,
            void* param,
            std::function<void*(void*, Continuation<void*>*)> block,
            OnCancellationConstructor on_cancellation_constructor
        ) : clause_object(clause_obj),
            reg_func_(std::move(reg_func)),
            process_res_func_(std::move(process_res_func)),
            param_(param),
            block_(std::move(block)),
            on_cancellation_constructor_(std::move(on_cancellation_constructor)),
            has_on_cancellation_(on_cancellation_constructor_ != nullptr) {}

        bool try_register_as_waiter(SelectImplementation<R>* select) {
            assert(select->in_registration_phase() || select->is_cancelled());
            assert(select->internal_result_ == NO_RESULT());
            reg_func_(clause_object, static_cast<void*>(select), param_);
            return select->internal_result_ == NO_RESULT();
        }

        void* process_result(void* result) {
            return process_res_func_(clause_object, param_, result);
        }

        void* invoke_block(void* argument, Continuation<void*>* completion) {
            return block_(argument, completion);
        }

        void dispose(std::shared_ptr<CoroutineContext> context) {
            // TODO(port): Segment-based cancellation when Segment is fully implemented
            if (auto* handle = static_cast<DisposableHandle*>(disposable_handle_or_segment)) {
                if (handle) handle->dispose();
            }
        }

        OnCancellationAction create_on_cancellation_action(
            SelectInstance<R>* select, void* internal_result
        ) {
            if (!has_on_cancellation_) return nullptr;
            return on_cancellation_constructor_(static_cast<void*>(select), param_, internal_result);
        }

        void* get_param() const { return param_; }
    };

private:
    std::shared_ptr<CoroutineContext> context_;

    // State can be: STATE_REG, reregister_list_ set, continuation stored, ClauseData*, STATE_COMPLETED, STATE_CANCELLED
    std::atomic<void*> state_{STATE_REG()};

    std::vector<std::unique_ptr<ClauseData>>* clauses_;

    void* disposable_handle_or_segment_ = nullptr;

    int index_in_segment_ = -1;

    void* internal_result_ = NO_RESULT();

    // Continuation stored during WAITING phase
    CancellableContinuation<void>* waiting_continuation_ = nullptr;

    // Reregister list when in registration phase with pending reregistrations
    std::unique_ptr<std::vector<void*>> reregister_list_;

    // Selected clause (when state transitions to selected)
    ClauseData* selected_clause_ = nullptr;

public:
    explicit SelectImplementation(std::shared_ptr<CoroutineContext> context)
        : context_(std::move(context)),
          clauses_(new std::vector<std::unique_ptr<ClauseData>>()) {
        clauses_->reserve(2);
    }

    ~SelectImplementation() {
        delete clauses_;
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        return context_;
    }

    // ==========================================================================
    // ==========================================================================
    bool in_registration_phase() const {
        void* s = state_.load(std::memory_order_acquire);
        return s == STATE_REG() || reregister_list_ != nullptr;
    }

    // ==========================================================================
    // ==========================================================================
    bool is_selected() const {
        return selected_clause_ != nullptr;
    }

    // ==========================================================================
    // ==========================================================================
    bool is_cancelled() const {
        return state_.load(std::memory_order_acquire) == STATE_CANCELLED();
    }

    // ==========================================================================
    // ==========================================================================
    void* do_select(Continuation<void*>* completion) {
        if (is_selected()) {
            return complete(completion);  // Fast path
        }
        return do_select_suspend(completion);  // Slow path
    }

private:
    // ==========================================================================
    // ==========================================================================
    void* do_select_suspend(Continuation<void*>* completion) {
        void* wait_result = wait_until_selected(completion);
        if (intrinsics::is_coroutine_suspended(wait_result)) {
            return wait_result;
        }
        return complete(completion);
    }

    // ==========================================================================
    // ==========================================================================
    void* wait_until_selected(Continuation<void*>* completion) {
        // Use suspend_cancellable_coroutine pattern
        return suspend_cancellable_coroutine<void>(
            [this](CancellableContinuation<void>& cont) {
                while (true) {
                    void* cur_state = state_.load(std::memory_order_acquire);

                    if (cur_state == STATE_REG() && reregister_list_ == nullptr) {
                        // Transition to WAITING phase by storing continuation
                        waiting_continuation_ = &cont;
                        void* expected = STATE_REG();
                        if (state_.compare_exchange_strong(expected, &cont)) {
                            // TODO(port): proper cancellation handler integration
                            return;  // Suspend
                        }
                        waiting_continuation_ = nullptr;
                        continue;
                    }

                    if (reregister_list_ != nullptr) {
                        // Re-register clauses
                        auto list = std::move(reregister_list_);
                        void* expected = cur_state;
                        if (state_.compare_exchange_strong(expected, STATE_REG())) {
                            for (void* clause_object : *list) {
                                reregister_clause(clause_object);
                            }
                        }
                        continue;
                    }

                    if (is_selected()) {
                        auto on_cancel = selected_clause_->create_on_cancellation_action(
                            this, internal_result_);
                        cont.resume(on_cancel);
                        return;  // Don't suspend
                    }

                    throw std::runtime_error("unexpected state in waitUntilSelected");
                }
            },
            completion
        );
    }

    // ==========================================================================
    // ==========================================================================
    void reregister_clause(void* clause_object) {
        ClauseData* clause = find_clause(clause_object);
        if (!clause) throw std::logic_error("reregisterClause: clause must exist");
        clause->disposable_handle_or_segment = nullptr;
        clause->index_in_segment = -1;
        register_clause_impl(clause, true);
    }

public:
    // ==========================================================================
    // ==========================================================================
    bool try_select(void* clause_object, void* result) override {
        return try_select_internal(clause_object, result) == TRY_SELECT_SUCCESSFUL;
    }

    // ==========================================================================
    // ==========================================================================
    TrySelectDetailedResult try_select_detailed(void* clause_object, void* result) {
        return to_try_select_detailed_result(try_select_internal(clause_object, result));
    }

private:
    // ==========================================================================
    // ==========================================================================
    int try_select_internal(void* clause_object, void* internal_result) {
        while (true) {
            void* cur_state = state_.load(std::memory_order_acquire);

            if (waiting_continuation_ != nullptr && cur_state == waiting_continuation_) {
                ClauseData* clause = find_clause(clause_object);
                if (!clause) continue;  // retry if clauses already null

                auto on_cancellation = clause->create_on_cancellation_action(this, internal_result);

                void* expected = cur_state;
                if (state_.compare_exchange_strong(expected, clause)) {
                    internal_result_ = internal_result;
                    selected_clause_ = clause;

                    auto* cont = waiting_continuation_;
                    waiting_continuation_ = nullptr;
                    if (try_resume_with_on_cancellation(cont, on_cancellation)) {
                        return TRY_SELECT_SUCCESSFUL;
                    }
                    internal_result_ = NO_RESULT();
                    selected_clause_ = nullptr;
                    return TRY_SELECT_CANCELLED;
                }
                continue;
            }

            if (cur_state == STATE_COMPLETED() || is_selected()) {
                return TRY_SELECT_ALREADY_SELECTED;
            }

            if (cur_state == STATE_CANCELLED()) {
                return TRY_SELECT_CANCELLED;
            }

            if (cur_state == STATE_REG() && reregister_list_ == nullptr) {
                reregister_list_ = std::make_unique<std::vector<void*>>();
                reregister_list_->push_back(clause_object);
                return TRY_SELECT_REREGISTER;
            }

            if (reregister_list_ != nullptr) {
                reregister_list_->push_back(clause_object);
                return TRY_SELECT_REREGISTER;
            }

            throw std::runtime_error("Unexpected state in trySelectInternal");
        }
    }

    // ==========================================================================
    // ==========================================================================
    ClauseData* find_clause(void* clause_object) {
        if (!clauses_) return nullptr;
        for (auto& clause : *clauses_) {
            if (clause->clause_object == clause_object) {
                return clause.get();
            }
        }
        throw std::runtime_error("Clause with object is not found");
    }

    // ==========================================================================
    // ==========================================================================
    void* complete(Continuation<void*>* completion) {
        assert(is_selected());

        ClauseData* selected = selected_clause_;

        void* result = internal_result_;

        cleanup(selected);

        void* block_argument = selected->process_result(result);
        return selected->invoke_block(block_argument, completion);
    }

    // ==========================================================================
    // ==========================================================================
    void cleanup(ClauseData* selected_clause) {
        assert(is_selected());

        if (!clauses_) return;

        for (auto& clause : *clauses_) {
            if (clause.get() != selected_clause) {
                clause->dispose(context_);
            }
        }

        state_.store(STATE_COMPLETED(), std::memory_order_release);
        internal_result_ = NO_RESULT();
        delete clauses_;
        clauses_ = nullptr;
    }

public:
    // ==========================================================================
    // ==========================================================================
    void invoke(std::exception_ptr cause) override {
        void* cur = state_.load(std::memory_order_acquire);
        while (true) {
            if (cur == STATE_COMPLETED()) return;
            if (state_.compare_exchange_weak(cur, STATE_CANCELLED())) break;
        }

        if (!clauses_) return;

        for (auto& clause : *clauses_) {
            clause->dispose(context_);
        }

        internal_result_ = NO_RESULT();
        delete clauses_;
        clauses_ = nullptr;
    }

    // ==========================================================================
    // ==========================================================================
    void dispose_on_completion(std::shared_ptr<DisposableHandle> handle) override {
        disposable_handle_or_segment_ = handle.get();
    }

    // ==========================================================================
    // ==========================================================================
    void invoke_on_cancellation(internal::SegmentBase* segment, int index) override {
        disposable_handle_or_segment_ = segment;
        index_in_segment_ = index;
    }

    // ==========================================================================
    // ==========================================================================
    void select_in_registration_phase(void* internal_result) override {
        internal_result_ = internal_result;
    }

    // ==========================================================================
    // SelectBuilder implementation
    // ==========================================================================

    void invoke(SelectClause0& clause, std::function<void*(Continuation<void*>*)> block) override {
        auto wrapped = [block](void*, Continuation<void*>* c) { return block(c); };
        register_clause(
            clause.get_clause_object(),
            clause.get_reg_func(),
            clause.get_process_res_func(),
            PARAM_CLAUSE_0(),
            std::move(wrapped),
            clause.get_on_cancellation_constructor()
        );
    }

    template<typename Q>
    void invoke(SelectClause1<Q>& clause, std::function<void*(Q, Continuation<void*>*)> block) {
        auto wrapped = [block](void* arg, Continuation<void*>* c) {
            return block(static_cast<Q>(reinterpret_cast<std::uintptr_t>(arg)), c);
        };
        register_clause(
            clause.get_clause_object(),
            clause.get_reg_func(),
            clause.get_process_res_func(),
            nullptr,
            std::move(wrapped),
            clause.get_on_cancellation_constructor()
        );
    }

    template<typename P, typename Q>
    void invoke(SelectClause2<P, Q>& clause, P param, std::function<void*(Q, Continuation<void*>*)> block) {
        auto wrapped = [block](void* arg, Continuation<void*>* c) {
            return block(static_cast<Q>(reinterpret_cast<std::uintptr_t>(arg)), c);
        };
        register_clause(
            clause.get_clause_object(),
            clause.get_reg_func(),
            clause.get_process_res_func(),
            reinterpret_cast<void*>(param),
            std::move(wrapped),
            clause.get_on_cancellation_constructor()
        );
    }



    // Helper class for lambda-based Runnable
    class LambdaRunnable : public Runnable {
        std::function<void()> func_;
    public:
        explicit LambdaRunnable(std::function<void()> f) : func_(std::move(f)) {}
        void run() override { func_(); }
    };

private:
    // ==========================================================================
    // ==========================================================================
    void register_clause(
        void* clause_object,
        RegistrationFunction reg_func,
        ProcessResultFunction process_res_func,
        void* param,
        std::function<void*(void*, Continuation<void*>*)> block,
        OnCancellationConstructor on_cancellation_constructor
    ) {
        auto clause = std::make_unique<ClauseData>(
            clause_object,
            std::move(reg_func),
            std::move(process_res_func),
            param,
            std::move(block),
            std::move(on_cancellation_constructor)
        );

        ClauseData* clause_ptr = clause.get();
        clauses_->push_back(std::move(clause));

        register_clause_impl(clause_ptr, false);

        // If clause was selected during registration, update selected_clause_
        if (internal_result_ != NO_RESULT()) {
            selected_clause_ = clause_ptr;
        }
    }

    void register_clause_impl(ClauseData* clause, bool reregister) {
        assert(state_.load() != STATE_CANCELLED());

        if (is_selected()) return;

        if (!reregister) {
            check_clause_object(clause->clause_object);
        }

        if (clause->try_register_as_waiter(this)) {
            clause->disposable_handle_or_segment = disposable_handle_or_segment_;
            clause->index_in_segment = index_in_segment_;
            disposable_handle_or_segment_ = nullptr;
            index_in_segment_ = -1;
        }
        // else: clause was selected, state already updated via select_in_registration_phase
    }

    // ==========================================================================
    // ==========================================================================
    void check_clause_object(void* clause_object) {
        if (!clauses_) return;
        for (size_t i = 0; i < clauses_->size() - 1; ++i) {  // -1 because current clause already added
            if ((*clauses_)[i]->clause_object == clause_object) {
                throw std::runtime_error("Cannot use select clauses on the same object");
            }
        }
    }

    // Grant ClauseData access to internal_result_
    friend class ClauseData;
};

// =============================================================================
// =============================================================================
template<typename R, typename BuilderFunc>
void* select(BuilderFunc&& builder, Continuation<void*>* continuation) {
    auto context = continuation->get_context();
    auto impl = std::make_shared<SelectImplementation<R>>(context);

    builder(*impl);

    return impl->do_select(continuation);
}

} // namespace selects
} // namespace coroutines
} // namespace kotlinx
