/**
 * @file Select.hpp
 * @brief Select expression for coroutines
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/selects/Select.kt
 *
 * Waits for the result of multiple suspending functions simultaneously.
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
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

namespace kotlinx {
namespace coroutines {
namespace selects {

// Forward declarations
template<typename R> class SelectInstance;
template<typename R> class SelectImplementation;
template<typename R> class SelectBuilder;

// =============================================================================
// Line 874-878: trySelectInternal(..) results.
// =============================================================================
constexpr int TRY_SELECT_SUCCESSFUL = 0;
constexpr int TRY_SELECT_REREGISTER = 1;
constexpr int TRY_SELECT_CANCELLED = 2;
constexpr int TRY_SELECT_ALREADY_SELECTED = 3;

// =============================================================================
// Line 880-890: trySelectDetailed(..) results.
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
// Line 892-903: Markers for states and special values.
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

// =============================================================================
// Line 147: typealias RegistrationFunction
// (clauseObject: Any, select: SelectInstance<*>, param: Any?) -> Unit
// Note: SelectInstance<*> becomes void* in C++ since we don't know R at clause level
// =============================================================================
using RegistrationFunction = std::function<void(void* clause_object, void* select, void* param)>;

// =============================================================================
// Line 158: typealias ProcessResultFunction
// (clauseObject: Any, param: Any?, clauseResult: Any?) -> Any?
// =============================================================================
using ProcessResultFunction = std::function<void*(void* clause_object, void* param, void* clause_result)>;

// =============================================================================
// Line 169-170: typealias OnCancellationConstructor
// (select: SelectInstance<*>, param: Any?, internalResult: Any?) ->
//     (Throwable, Any?, CoroutineContext) -> Unit
// =============================================================================
using OnCancellationAction = std::function<void(std::exception_ptr, void*, std::shared_ptr<CoroutineContext>)>;

using OnCancellationConstructor = std::function<
    OnCancellationAction(void* select, void* param, void* internal_result)
>;

// =============================================================================
// Line 185: private val DUMMY_PROCESS_RESULT_FUNCTION
// =============================================================================
inline ProcessResultFunction dummy_process_result_function() {
    return [](void*, void*, void*) -> void* { return nullptr; };
}

// =============================================================================
// Line 131-136: public sealed interface SelectClause
// =============================================================================
class SelectClause {
public:
    virtual ~SelectClause() = default;

    // Line 132: public val clauseObject: Any
    virtual void* get_clause_object() const = 0;

    // Line 133: public val regFunc: RegistrationFunction
    virtual RegistrationFunction get_reg_func() const = 0;

    // Line 134: public val processResFunc: ProcessResultFunction
    virtual ProcessResultFunction get_process_res_func() const = 0;

    // Line 135: public val onCancellationConstructor: OnCancellationConstructor?
    virtual OnCancellationConstructor get_on_cancellation_constructor() const = 0;
    virtual bool has_on_cancellation_constructor() const = 0;
};

// =============================================================================
// Line 175: public sealed interface SelectClause0 : SelectClause
// =============================================================================
class SelectClause0 : public SelectClause {};

// =============================================================================
// Line 177-183: internal class SelectClause0Impl
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
    // Line 182: override val processResFunc = DUMMY_PROCESS_RESULT_FUNCTION
    ProcessResultFunction get_process_res_func() const override { return dummy_process_result_function(); }
    OnCancellationConstructor get_on_cancellation_constructor() const override { return on_cancellation_constructor_; }
    bool has_on_cancellation_constructor() const override { return has_on_cancellation_; }
};

// =============================================================================
// Line 190: public sealed interface SelectClause1<out Q> : SelectClause
// =============================================================================
template<typename Q>
class SelectClause1 : public SelectClause {};

// =============================================================================
// Line 192-197: internal class SelectClause1Impl<Q>
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
// Line 202: public sealed interface SelectClause2<in P, out Q> : SelectClause
// =============================================================================
template<typename P, typename Q>
class SelectClause2 : public SelectClause {};

// =============================================================================
// Line 204-209: internal class SelectClause2Impl<P, Q>
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

// =============================================================================
// Line 217-247: public sealed interface SelectInstance<in R>
// =============================================================================
template<typename R>
class SelectInstance {
public:
    virtual ~SelectInstance() = default;

    // Line 221: public val context: CoroutineContext
    virtual std::shared_ptr<CoroutineContext> get_context() const = 0;

    // Line 233: public fun trySelect(clauseObject: Any, result: Any?): Boolean
    virtual bool try_select(void* clause_object, void* result) = 0;

    // Line 239: public fun disposeOnCompletion(disposableHandle: DisposableHandle)
    virtual void dispose_on_completion(std::shared_ptr<DisposableHandle> handle) = 0;

    // Line 246: public fun selectInRegistrationPhase(internalResult: Any?)
    virtual void select_in_registration_phase(void* internal_result) = 0;
};

// =============================================================================
// Line 249: internal interface SelectInstanceInternal<R> : SelectInstance<R>, Waiter
// =============================================================================
template<typename R>
class SelectInstanceInternal : public SelectInstance<R>, public Waiter {
public:
    // Waiter interface for segment-based cancellation
    // SegmentBase is the non-templated base for Segment<S>
    virtual void invoke_on_cancellation(internal::SegmentBase* segment, int index) = 0;
};

// =============================================================================
// Line 73-112: public sealed interface SelectBuilder<in R>
// =============================================================================
template<typename R>
class SelectBuilder {
public:
    virtual ~SelectBuilder() = default;

    // Line 77: public operator fun SelectClause0.invoke(block: suspend () -> R)
    virtual void invoke(SelectClause0& clause, std::function<void*(Continuation<void*>*)> block) = 0;

    // Line 82: public operator fun <Q> SelectClause1<Q>.invoke(block: suspend (Q) -> R)
    template<typename Q>
    void invoke(SelectClause1<Q>& clause, std::function<void*(Q, Continuation<void*>*)> block);

    // Line 87: public operator fun <P, Q> SelectClause2<P, Q>.invoke(param: P, block: suspend (Q) -> R)
    template<typename P, typename Q>
    void invoke(SelectClause2<P, Q>& clause, P param, std::function<void*(Q, Continuation<void*>*)> block);

    // Line 96-111: fun onTimeout(timeMillis: Long, block: suspend () -> R)
    virtual void on_timeout(long long time_millis, std::function<void*(Continuation<void*>*)> block) = 0;
};

// =============================================================================
// Line 251-863: SelectImplementation class
// =============================================================================
template<typename R>
class SelectImplementation : public CancelHandler,
                             public SelectBuilder<R>,
                             public SelectInstanceInternal<R>,
                             public std::enable_shared_from_this<SelectImplementation<R>> {
public:
    // ==========================================================================
    // Line 783-862: internal inner class ClauseData
    // ==========================================================================
    class ClauseData {
    public:
        // Line 784: @JvmField val clauseObject: Any
        void* const clause_object;

    private:
        // Line 785: private val regFunc: RegistrationFunction
        RegistrationFunction reg_func_;
        // Line 786: private val processResFunc: ProcessResultFunction
        ProcessResultFunction process_res_func_;
        // Line 787: private val param: Any?
        void* param_;
        // Line 788: private val block: Any (suspend function)
        std::function<void*(void*, Continuation<void*>*)> block_;
        // Line 789: @JvmField val onCancellationConstructor: OnCancellationConstructor?
        OnCancellationConstructor on_cancellation_constructor_;
        bool has_on_cancellation_;

    public:
        // Line 792: var disposableHandleOrSegment: Any? = null
        void* disposable_handle_or_segment = nullptr;
        // Line 794: var indexInSegment: Int = -1
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

        // Line 807-812: fun tryRegisterAsWaiter(select: SelectImplementation<R>): Boolean
        bool try_register_as_waiter(SelectImplementation<R>* select) {
            assert(select->in_registration_phase() || select->is_cancelled());
            assert(select->internal_result_ == NO_RESULT());
            reg_func_(clause_object, static_cast<void*>(select), param_);
            return select->internal_result_ == NO_RESULT();
        }

        // Line 824: fun processResult(result: Any?)
        void* process_result(void* result) {
            return process_res_func_(clause_object, param_, result);
        }

        // Line 831-848: suspend fun invokeBlock(argument: Any?): R
        void* invoke_block(void* argument, Continuation<void*>* completion) {
            return block_(argument, completion);
        }

        // Line 850-858: fun dispose()
        void dispose(std::shared_ptr<CoroutineContext> context) {
            // TODO(port): Segment-based cancellation when Segment is fully implemented
            if (auto* handle = static_cast<DisposableHandle*>(disposable_handle_or_segment)) {
                if (handle) handle->dispose();
            }
        }

        // Line 860-861: fun createOnCancellationAction(...)
        OnCancellationAction create_on_cancellation_action(
            SelectInstance<R>* select, void* internal_result
        ) {
            if (!has_on_cancellation_) return nullptr;
            return on_cancellation_constructor_(static_cast<void*>(select), param_, internal_result);
        }

        void* get_param() const { return param_; }
    };

private:
    // Line 253: override val context: CoroutineContext
    std::shared_ptr<CoroutineContext> context_;

    // Line 360: private val state = atomic<Any>(STATE_REG)
    // State can be: STATE_REG, reregister_list_ set, continuation stored, ClauseData*, STATE_COMPLETED, STATE_CANCELLED
    std::atomic<void*> state_{STATE_REG()};

    // Line 392: private var clauses: MutableList<ClauseData>? = ArrayList(2)
    std::vector<std::unique_ptr<ClauseData>>* clauses_;

    // Line 404: private var disposableHandleOrSegment: Any? = null
    void* disposable_handle_or_segment_ = nullptr;

    // Line 412: private var indexInSegment: Int = -1
    int index_in_segment_ = -1;

    // Line 433: private var internalResult: Any? = NO_RESULT
    void* internal_result_ = NO_RESULT();

    // Continuation stored during WAITING phase
    CancellableContinuation<void>* waiting_continuation_ = nullptr;

    // Reregister list when in registration phase with pending reregistrations
    // Line 590: curState is List<*> - stored as vector of clause objects to re-register
    std::unique_ptr<std::vector<void*>> reregister_list_;

    // Selected clause (when state transitions to selected)
    ClauseData* selected_clause_ = nullptr;

public:
    // Line 252-254: constructor
    explicit SelectImplementation(std::shared_ptr<CoroutineContext> context)
        : context_(std::move(context)),
          clauses_(new std::vector<std::unique_ptr<ClauseData>>()) {
        clauses_->reserve(2);  // Line 392: ArrayList(2)
    }

    ~SelectImplementation() {
        delete clauses_;
    }

    // Line 221: public val context: CoroutineContext
    std::shared_ptr<CoroutineContext> get_context() const override {
        return context_;
    }

    // ==========================================================================
    // Line 366-369: private val inRegistrationPhase
    // ==========================================================================
    bool in_registration_phase() const {
        void* s = state_.load(std::memory_order_acquire);
        return s == STATE_REG() || reregister_list_ != nullptr;
    }

    // ==========================================================================
    // Line 375-376: private val isSelected
    // ==========================================================================
    bool is_selected() const {
        return selected_clause_ != nullptr;
    }

    // ==========================================================================
    // Line 381-382: private val isCancelled
    // ==========================================================================
    bool is_cancelled() const {
        return state_.load(std::memory_order_acquire) == STATE_CANCELLED();
    }

    // ==========================================================================
    // Line 443-457: internal open suspend fun doSelect(): R
    // ==========================================================================
    void* do_select(Continuation<void*>* completion) {
        // Line 444: if (isSelected) complete() else doSelectSuspend()
        if (is_selected()) {
            return complete(completion);  // Fast path
        }
        return do_select_suspend(completion);  // Slow path
    }

private:
    // ==========================================================================
    // Line 450-457: private suspend fun doSelectSuspend(): R
    // ==========================================================================
    void* do_select_suspend(Continuation<void*>* completion) {
        // Line 453: waitUntilSelected()
        void* wait_result = wait_until_selected(completion);
        if (intrinsics::is_coroutine_suspended(wait_result)) {
            return wait_result;
        }
        // Line 456: return complete()
        return complete(completion);
    }

    // ==========================================================================
    // Line 569-604: private suspend fun waitUntilSelected()
    // ==========================================================================
    void* wait_until_selected(Continuation<void*>* completion) {
        // Use suspend_cancellable_coroutine pattern
        return suspend_cancellable_coroutine<void>(
            [this](CancellableContinuation<void>& cont) {
                // Line 571: state.loop { curState -> ... }
                while (true) {
                    void* cur_state = state_.load(std::memory_order_acquire);

                    // Line 575: curState === STATE_REG
                    if (cur_state == STATE_REG() && reregister_list_ == nullptr) {
                        // Transition to WAITING phase by storing continuation
                        waiting_continuation_ = &cont;
                        void* expected = STATE_REG();
                        if (state_.compare_exchange_strong(expected, &cont)) {
                            // Line 585: cont.invokeOnCancellation(this)
                            // TODO(port): proper cancellation handler integration
                            return;  // Suspend
                        }
                        waiting_continuation_ = nullptr;
                        continue;
                    }

                    // Line 590: curState is List<*>
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

                    // Line 596: curState is ClauseData
                    if (is_selected()) {
                        // Line 597: cont.resume(Unit, ...)
                        auto on_cancel = selected_clause_->create_on_cancellation_action(
                            this, internal_result_);
                        cont.resume(on_cancel);
                        return;  // Don't suspend
                    }

                    // Line 601: else -> error
                    throw std::runtime_error("unexpected state in waitUntilSelected");
                }
            },
            completion
        );
    }

    // ==========================================================================
    // Line 612-617: private fun reregisterClause(clauseObject: Any)
    // ==========================================================================
    void reregister_clause(void* clause_object) {
        // Line 613: findClause(clauseObject)!! - guaranteed non-null
        ClauseData* clause = find_clause(clause_object);
        if (!clause) throw std::logic_error("reregisterClause: clause must exist");
        clause->disposable_handle_or_segment = nullptr;
        clause->index_in_segment = -1;
        register_clause_impl(clause, true);
    }

public:
    // ==========================================================================
    // Line 623-624: override fun trySelect(clauseObject: Any, result: Any?): Boolean
    // ==========================================================================
    bool try_select(void* clause_object, void* result) override {
        return try_select_internal(clause_object, result) == TRY_SELECT_SUCCESSFUL;
    }

    // ==========================================================================
    // Line 631-632: fun trySelectDetailed(...)
    // ==========================================================================
    TrySelectDetailedResult try_select_detailed(void* clause_object, void* result) {
        return to_try_select_detailed_result(try_select_internal(clause_object, result));
    }

private:
    // ==========================================================================
    // Line 634-669: private fun trySelectInternal(...)
    // ==========================================================================
    int try_select_internal(void* clause_object, void* internal_result) {
        while (true) {
            void* cur_state = state_.load(std::memory_order_acquire);

            // Line 638: is CancellableContinuation<*>
            if (waiting_continuation_ != nullptr && cur_state == waiting_continuation_) {
                ClauseData* clause = find_clause(clause_object);
                if (!clause) continue;  // retry if clauses already null

                auto on_cancellation = clause->create_on_cancellation_action(this, internal_result);

                // Line 641: if (state.compareAndSet(curState, clause))
                void* expected = cur_state;
                if (state_.compare_exchange_strong(expected, clause)) {
                    // Line 646: this.internalResult = internalResult
                    internal_result_ = internal_result;
                    selected_clause_ = clause;

                    // Line 647: if (cont.tryResume(onCancellation))
                    auto* cont = waiting_continuation_;
                    waiting_continuation_ = nullptr;
                    if (try_resume_with_on_cancellation(cont, on_cancellation)) {
                        return TRY_SELECT_SUCCESSFUL;
                    }
                    // Line 649: resumption failed, clean up
                    internal_result_ = NO_RESULT();
                    selected_clause_ = nullptr;
                    return TRY_SELECT_CANCELLED;
                }
                continue;
            }

            // Line 654: STATE_COMPLETED, is ClauseData
            if (cur_state == STATE_COMPLETED() || is_selected()) {
                return TRY_SELECT_ALREADY_SELECTED;
            }

            // Line 656: STATE_CANCELLED
            if (cur_state == STATE_CANCELLED()) {
                return TRY_SELECT_CANCELLED;
            }

            // Line 660: STATE_REG (still in registration)
            if (cur_state == STATE_REG() && reregister_list_ == nullptr) {
                reregister_list_ = std::make_unique<std::vector<void*>>();
                reregister_list_->push_back(clause_object);
                return TRY_SELECT_REREGISTER;
            }

            // Line 664: is List<*> (add to reregister list)
            if (reregister_list_ != nullptr) {
                reregister_list_->push_back(clause_object);
                return TRY_SELECT_REREGISTER;
            }

            throw std::runtime_error("Unexpected state in trySelectInternal");
        }
    }

    // ==========================================================================
    // Line 676-683: private fun findClause(clauseObject: Any): ClauseData?
    // ==========================================================================
    ClauseData* find_clause(void* clause_object) {
        // Line 679: val clauses = this.clauses ?: return null
        if (!clauses_) return nullptr;
        // Line 680-682: find clause or throw
        for (auto& clause : *clauses_) {
            if (clause->clause_object == clause_object) {
                return clause.get();
            }
        }
        // Line 682: ?: error("Clause with object $clauseObject is not found")
        throw std::runtime_error("Clause with object is not found");
    }

    // ==========================================================================
    // Line 700-724: private suspend fun complete(): R
    // ==========================================================================
    void* complete(Continuation<void*>* completion) {
        assert(is_selected());

        // Line 704: val selectedClause = state.value as ClauseData
        ClauseData* selected = selected_clause_;

        // Line 708: val internalResult = this.internalResult
        void* result = internal_result_;

        // Line 709: cleanup(selectedClause)
        cleanup(selected);

        // Line 714-715: process result and invoke block
        void* block_argument = selected->process_result(result);
        return selected->invoke_block(block_argument, completion);
    }

    // ==========================================================================
    // Line 742-756: private fun cleanup(selectedClause: ClauseData)
    // ==========================================================================
    void cleanup(ClauseData* selected_clause) {
        assert(is_selected());

        // Line 746: val clauses = this.clauses ?: return
        if (!clauses_) return;

        // Line 749-751: dispose all except selected
        for (auto& clause : *clauses_) {
            if (clause.get() != selected_clause) {
                clause->dispose(context_);
            }
        }

        // Line 753-755: clean up to avoid memory leaks
        state_.store(STATE_COMPLETED(), std::memory_order_release);
        internal_result_ = NO_RESULT();
        delete clauses_;
        clauses_ = nullptr;
    }

public:
    // ==========================================================================
    // Line 759-778: override fun invoke(cause: Throwable?) [CancelHandler]
    // ==========================================================================
    void invoke(std::exception_ptr cause) override {
        // Line 761-768: update state
        void* cur = state_.load(std::memory_order_acquire);
        while (true) {
            if (cur == STATE_COMPLETED()) return;
            if (state_.compare_exchange_weak(cur, STATE_CANCELLED())) break;
        }

        // Line 772: val clauses = this.clauses ?: return
        if (!clauses_) return;

        // Line 774: dispose all clauses
        for (auto& clause : *clauses_) {
            clause->dispose(context_);
        }

        // Line 776-777: clean up
        internal_result_ = NO_RESULT();
        delete clauses_;
        clauses_ = nullptr;
    }

    // ==========================================================================
    // Line 534-536: override fun disposeOnCompletion(disposableHandle: DisposableHandle)
    // ==========================================================================
    void dispose_on_completion(std::shared_ptr<DisposableHandle> handle) override {
        disposable_handle_or_segment_ = handle.get();
    }

    // ==========================================================================
    // Line 549-552: override fun invokeOnCancellation(segment: Segment<*>, index: Int)
    // ==========================================================================
    void invoke_on_cancellation(internal::SegmentBase* segment, int index) override {
        disposable_handle_or_segment_ = segment;
        index_in_segment_ = index;
    }

    // ==========================================================================
    // Line 554-556: override fun selectInRegistrationPhase(internalResult: Any?)
    // ==========================================================================
    void select_in_registration_phase(void* internal_result) override {
        internal_result_ = internal_result;
    }

    // ==========================================================================
    // SelectBuilder implementation
    // Line 463-470: clause registration operators
    // ==========================================================================

    // Line 463-464: operator fun SelectClause0.invoke(block: suspend () -> R)
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

    // Line 466-467: operator fun <Q> SelectClause1<Q>.invoke(block: suspend (Q) -> R)
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

    // Line 469-470: operator fun <P, Q> SelectClause2<P, Q>.invoke(param: P, block: suspend (Q) -> R)
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

    // Line 96-111: fun onTimeout(timeMillis: Long, block: suspend () -> R)
    void on_timeout(long long time_millis, std::function<void*(Continuation<void*>*)> block) override {
        // TODO(port): Implement onTimeout using delay infrastructure
        // IMPORTANT: This is NOT implemented - will cause runtime error if called
        (void)time_millis;
        (void)block;
        throw std::runtime_error("select onTimeout is not yet implemented - use external timeout wrapper");
    }

private:
    // ==========================================================================
    // Line 488-518: internal fun ClauseData.register(reregister: Boolean = false)
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
        // Line 489: assert { state.value !== STATE_CANCELLED }
        assert(state_.load() != STATE_CANCELLED());

        // Line 491: if already selected, return
        if (is_selected()) return;

        // Line 494: check no duplicate clause objects
        if (!reregister) {
            check_clause_object(clause->clause_object);
        }

        // Line 496: try to register in the corresponding object
        if (clause->try_register_as_waiter(this)) {
            // Line 509-512: store disposal info
            clause->disposable_handle_or_segment = disposable_handle_or_segment_;
            clause->index_in_segment = index_in_segment_;
            disposable_handle_or_segment_ = nullptr;
            index_in_segment_ = -1;
        }
        // else: clause was selected, state already updated via select_in_registration_phase
    }

    // ==========================================================================
    // Line 523-532: private fun checkClauseObject(clauseObject: Any)
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
// Line 50-60: public suspend inline fun <R> select(...)
// =============================================================================
template<typename R, typename BuilderFunc>
void* select(BuilderFunc&& builder, Continuation<void*>* continuation) {
    // Line 54: SelectImplementation<R>(coroutineContext)
    auto context = continuation->get_context();
    auto impl = std::make_shared<SelectImplementation<R>>(context);

    // Line 55: builder(this)
    builder(*impl);

    // Line 58: doSelect()
    return impl->do_select(continuation);
}

} // namespace selects
} // namespace coroutines
} // namespace kotlinx
