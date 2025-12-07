// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/selects/Select.kt
//
// TODO: This is a mechanical syntax transliteration. The following Kotlin constructs need proper C++ implementation:
// - suspend functions (marked but not implemented as C++20 coroutines)
// - inline functions with crossinline parameters
// - Kotlin contracts (contract { ... })
// - sealed interface (converted to abstract class with pure virtual methods)
// - operator overloading (invoke converted to regular methods)
// - typealias (converted to using declarations)
// - inner class (need explicit parent reference)
// - @JvmField, @JvmName annotations (kept as comments)
// - Kotlin atomicfu (atomic<T> needs C++ std::atomic or equivalent)
// - Smart casts and type checks (is, as operators)
// - Default parameters (need overloads or std::optional)
// - Extension functions (converted to free functions or methods)
// - when expressions (converted to if-else chains or switch)
// - Nullable types (T? -> T* or std::optional<T>)
// - object (singleton pattern)
// - data class properties
// - Property delegation
// - Lambda types and closures
// - Kotlin Symbol type
// - kotlin.internal.InlineOnly, LowPriorityInOverloadResolution
// - @Deprecated annotations
// - @OptIn, @InternalCoroutinesApi, @ExperimentalCoroutinesApi, @PublishedApi annotations (kept as comments)
// - @BenignDataRace annotation (data race annotation)

namespace kotlinx {
namespace coroutines {
namespace selects {

// import kotlinx.atomicfu.*
// import kotlinx.coroutines.*
// import kotlinx.coroutines.channels.*
// import kotlinx.coroutines.internal.*
// import kotlinx.coroutines.selects.TrySelectDetailedResult.*
// import kotlin.contracts.*
// import kotlin.coroutines.*
// import kotlin.internal.*
// import kotlin.jvm.*

/**
 * Waits for the result of multiple suspending functions simultaneously, which are specified using _clauses_
 * in the [builder] scope of this select invocation. The caller is suspended until one of the clauses
 * is either _selected_ or _fails_.
 *
 * At most one clause is *atomically* selected and its block is executed. The result of the selected clause
 * becomes the result of the select. If any clause _fails_, then the select invocation produces the
 * corresponding exception. No clause is selected in this case.
 *
 * This select function is _biased_ to the first clause. When multiple clauses can be selected at the same time,
 * the first one of them gets priority. Use [selectUnbiased] for an unbiased (randomized) selection among
 * the clauses.

 * There is no `default` clause for select expression. Instead, each selectable suspending function has the
 * corresponding non-suspending version that can be used with a regular `when` expression to select one
 * of the alternatives or to perform the default (`else`) action if none of them can be immediately selected.
 *
 * ### List of supported select methods
 *
 * | **Receiver**     | **Suspending function**                           | **Select clause**
 * | ---------------- | ---------------------------------------------     | -----------------------------------------------------
 * | [Job]            | [join][Job.join]                                  | [onJoin][Job.onJoin]
 * | [Deferred]       | [await][Deferred.await]                           | [onAwait][Deferred.onAwait]
 * | [SendChannel]    | [send][SendChannel.send]                          | [onSend][SendChannel.onSend]
 * | [ReceiveChannel] | [receive][ReceiveChannel.receive]                 | [onReceive][ReceiveChannel.onReceive]
 * | [ReceiveChannel] | [receiveCatching][ReceiveChannel.receiveCatching] | [onReceiveCatching][ReceiveChannel.onReceiveCatching]
 * | none             | [delay]                                           | [onTimeout][SelectBuilder.onTimeout]
 *
 * This suspending function is cancellable: if the [Job] of the current coroutine is cancelled while this
 * suspending function is waiting, this function immediately resumes with [CancellationException].
 * There is a **prompt cancellation guarantee**: even if this function is ready to return the result, but was cancelled
 * while suspended, [CancellationException] will be thrown. See [suspendCancellableCoroutine] for low-level details.
 *
 * Note that this function does not check for cancellation when it is not suspended.
 * Use [yield] or [CoroutineScope.isActive] to periodically check for cancellation in tight loops if needed.
 */
// @OptIn(ExperimentalContracts::class)
template<typename R, typename BuilderFunc>
R select(BuilderFunc&& builder) {
    // TODO: suspend function semantics not implemented
    // TODO: Kotlin contract { callsInPlace(builder, InvocationKind.EXACTLY_ONCE) } not applicable in C++
    // contract {
    //     callsInPlace(builder, InvocationKind.EXACTLY_ONCE)
    // }

    // TODO: coroutineContext not available in C++ without coroutine infrastructure
    SelectImplementation<R> select_impl(/* coroutineContext */ CoroutineContext{});
    builder(select_impl);
    // TAIL-CALL OPTIMIZATION: the only
    // suspend call is at the last position.
    return select_impl.do_select();
}

/**
 * Scope for [select] invocation.
 *
 * An instance of [SelectBuilder] can only be retrieved as a receiver of a [select] block call,
 * and it is only valid during the registration phase of the select builder.
 * Any uses outside it lead to unspecified behaviour and are prohibited.
 *
 * The general rule of thumb is that instances of this type should always be used
 * implicitly and there shouldn't be any signatures mentioning this type,
 * whether explicitly (e.g. function signature) or implicitly (e.g. inferred `val` type).
 */
// sealed interface -> abstract class
template<typename R>
class SelectBuilder {
public:
    virtual ~SelectBuilder() = default;

    /**
     * Registers a clause in this [select] expression without additional parameters that does not select any value.
     */
    virtual void invoke(SelectClause0& clause, std::function<R()> block) = 0;

    /**
     * Registers clause in this [select] expression without additional parameters that selects value of type [Q].
     */
    template<typename Q>
    virtual void invoke(SelectClause1<Q>& clause, std::function<R(Q)> block) = 0;

    /**
     * Registers clause in this [select] expression with additional parameter of type [P] that selects value of type [Q].
     */
    template<typename P, typename Q>
    virtual void invoke(SelectClause2<P, Q>& clause, P param, std::function<R(Q)> block) = 0;

    /**
     * Registers clause in this [select] expression with additional nullable parameter of type [P]
     * with the `null` value for this parameter that selects value of type [Q].
     */
    template<typename P, typename Q>
    void invoke(SelectClause2<P*, Q>& clause, std::function<R(Q)> block) {
        invoke(clause, nullptr, block);
    }

    /**
     * Clause that selects the given [block] after a specified timeout passes.
     * If timeout is negative or zero, [block] is selected immediately.
     *
     * **Note: This is an experimental api.** It may be replaced with light-weight timer/timeout channels in the future.
     *
     * @param timeMillis timeout time in milliseconds.
     */
    // @ExperimentalCoroutinesApi
    // @Suppress("INVISIBLE_REFERENCE", "INVISIBLE_MEMBER")
    // @LowPriorityInOverloadResolution
    // @Deprecated(
    //     message = "Replaced with the same extension function",
    //     level = DeprecationLevel.ERROR,
    //     replaceWith = ReplaceWith(expression = "onTimeout", imports = ["kotlinx.coroutines.selects.onTimeout"])
    // ) // Since 1.7.0, was experimental
    void on_timeout(long time_millis, std::function<R()> block) {
        on_timeout(time_millis, block);
    }
};

/**
 * Each [select] clause is specified with:
 * 1) the [object of this clause][clauseObject],
 *    such as the channel instance for [SendChannel.onSend];
 * 2) the function that specifies how this clause
 *    should be registered in the object above;
 * 3) the function that modifies the internal result
 *    (passed via [SelectInstance.trySelect] or
 *    [SelectInstance.selectInRegistrationPhase])
 *    to the argument of the user-specified block.
 * 4) the function that specifies how the internal result provided via
 *    [SelectInstance.trySelect] or [SelectInstance.selectInRegistrationPhase]
 *    should be processed in case of this `select` cancellation while dispatching.
 *
 * @suppress **This is unstable API, and it is subject to change.**
 */
// @InternalCoroutinesApi
// sealed interface -> abstract class
class SelectClause {
public:
    virtual ~SelectClause() = default;
    virtual void* get_clause_object() const = 0;
    virtual RegistrationFunction get_reg_func() const = 0;
    virtual ProcessResultFunction get_process_res_func() const = 0;
    virtual OnCancellationConstructor* get_on_cancellation_constructor() const = 0;
};

/**
 * The registration function specifies how the `select` instance should be registered into
 * the specified clause object. In case of channels, the registration logic
 * coincides with the plain `send/receive` operation with the only difference that
 * the `select` instance is stored as a waiter instead of continuation.
 *
 * @suppress **This is unstable API, and it is subject to change.**
 */
// @InternalCoroutinesApi
// typealias -> using
using RegistrationFunction = std::function<void(void* /* clauseObject */, SelectInstance<void*>* /* select */, void* /* param */)>;

/**
 * This function specifies how the _internal_ result, provided via [SelectInstance.selectInRegistrationPhase]
 * or [SelectInstance.trySelect] should be processed. For example, both [ReceiveChannel.onReceive] and
 * [ReceiveChannel.onReceiveCatching] clauses perform exactly the same synchronization logic,
 * but differ when the channel has been discovered in the closed or cancelled state.
 *
 * @suppress **This is unstable API, and it is subject to change.**
 */
// @InternalCoroutinesApi
// typealias -> using
using ProcessResultFunction = std::function<void*(void* /* clauseObject */, void* /* param */, void* /* clauseResult */)>;

/**
 * This function specifies how the internal result, provided via [SelectInstance.trySelect]
 * or [SelectInstance.selectInRegistrationPhase], should be processed in case of this `select`
 * cancellation while dispatching. Unfortunately, we cannot pass this function only in [SelectInstance.trySelect],
 * as [SelectInstance.selectInRegistrationPhase] can be called when the coroutine is already cancelled.
 *
 * @suppress **This is unstable API, and it is subject to change.**
 */
// @InternalCoroutinesApi
// typealias -> using
using OnCancellationConstructor = std::function<
    std::function<void(std::exception_ptr /* Throwable */, void* /* value */, CoroutineContext /* context */)>(
        SelectInstance<void*>* /* select */,
        void* /* param */,
        void* /* internalResult */
    )
>;

/**
 * Clause for [select] expression without additional parameters that does not select any value.
 */
// sealed interface -> abstract class
class SelectClause0 : public SelectClause {
public:
    virtual ~SelectClause0() = default;
};

class SelectClause0Impl : public SelectClause0 {
private:
    void* clause_object_;
    RegistrationFunction reg_func_;
    OnCancellationConstructor* on_cancellation_constructor_;

public:
    SelectClause0Impl(
        void* clause_object,
        RegistrationFunction reg_func,
        OnCancellationConstructor* on_cancellation_constructor = nullptr
    ) : clause_object_(clause_object),
        reg_func_(reg_func),
        on_cancellation_constructor_(on_cancellation_constructor) {}

    void* get_clause_object() const override { return clause_object_; }
    RegistrationFunction get_reg_func() const override { return reg_func_; }
    ProcessResultFunction get_process_res_func() const override { return kDummyProcessResultFunction; }
    OnCancellationConstructor* get_on_cancellation_constructor() const override { return on_cancellation_constructor_; }
};

ProcessResultFunction kDummyProcessResultFunction = [](void*, void*, void*) -> void* { return nullptr; };

/**
 * Clause for [select] expression without additional parameters that selects value of type [Q].
 */
// sealed interface -> abstract class
template<typename Q>
class SelectClause1 : public SelectClause {
public:
    virtual ~SelectClause1() = default;
};

template<typename Q>
class SelectClause1Impl : public SelectClause1<Q> {
private:
    void* clause_object_;
    RegistrationFunction reg_func_;
    ProcessResultFunction process_res_func_;
    OnCancellationConstructor* on_cancellation_constructor_;

public:
    SelectClause1Impl(
        void* clause_object,
        RegistrationFunction reg_func,
        ProcessResultFunction process_res_func,
        OnCancellationConstructor* on_cancellation_constructor = nullptr
    ) : clause_object_(clause_object),
        reg_func_(reg_func),
        process_res_func_(process_res_func),
        on_cancellation_constructor_(on_cancellation_constructor) {}

    void* get_clause_object() const override { return clause_object_; }
    RegistrationFunction get_reg_func() const override { return reg_func_; }
    ProcessResultFunction get_process_res_func() const override { return process_res_func_; }
    OnCancellationConstructor* get_on_cancellation_constructor() const override { return on_cancellation_constructor_; }
};

/**
 * Clause for [select] expression with additional parameter of type [P] that selects value of type [Q].
 */
// sealed interface -> abstract class
template<typename P, typename Q>
class SelectClause2 : public SelectClause {
public:
    virtual ~SelectClause2() = default;
};

template<typename P, typename Q>
class SelectClause2Impl : public SelectClause2<P, Q> {
private:
    void* clause_object_;
    RegistrationFunction reg_func_;
    ProcessResultFunction process_res_func_;
    OnCancellationConstructor* on_cancellation_constructor_;

public:
    SelectClause2Impl(
        void* clause_object,
        RegistrationFunction reg_func,
        ProcessResultFunction process_res_func,
        OnCancellationConstructor* on_cancellation_constructor = nullptr
    ) : clause_object_(clause_object),
        reg_func_(reg_func),
        process_res_func_(process_res_func),
        on_cancellation_constructor_(on_cancellation_constructor) {}

    void* get_clause_object() const override { return clause_object_; }
    RegistrationFunction get_reg_func() const override { return reg_func_; }
    ProcessResultFunction get_process_res_func() const override { return process_res_func_; }
    OnCancellationConstructor* get_on_cancellation_constructor() const override { return on_cancellation_constructor_; }
};

/**
 * Internal representation of `select` instance.
 *
 * @suppress **This is unstable API, and it is subject to change.**
 */
// @InternalCoroutinesApi
// sealed interface -> abstract class
template<typename R>
class SelectInstance {
public:
    virtual ~SelectInstance() = default;

    /**
     * The context of the coroutine that is performing this `select` operation.
     */
    virtual CoroutineContext get_context() const = 0;

    /**
     * This function should be called by other operations,
     * which are trying to perform a rendezvous with this `select`.
     * Returns `true` if the rendezvous succeeds, `false` otherwise.
     *
     * Note that according to the current implementation, a rendezvous attempt can fail
     * when either another clause is already selected or this `select` is still in
     * REGISTRATION phase. To distinguish the reasons, [SelectImplementation.trySelectDetailed]
     * function can be used instead.
     */
    virtual bool try_select(void* clause_object, void* result) = 0;

    /**
     * When this `select` instance is stored as a waiter, the specified [handle][disposableHandle]
     * defines how the stored `select` should be removed in case of cancellation or another clause selection.
     */
    virtual void dispose_on_completion(DisposableHandle* disposable_handle) = 0;

    /**
     * When a clause becomes selected during registration, the corresponding internal result
     * (which is further passed to the clause's [ProcessResultFunction]) should be provided
     * via this function. After that, other clause registrations are ignored and [trySelect] fails.
     */
    virtual void select_in_registration_phase(void* internal_result) = 0;
};

template<typename R>
class SelectInstanceInternal : public SelectInstance<R>, public Waiter {
public:
    virtual ~SelectInstanceInternal() = default;
};

// @PublishedApi
template<typename R>
class SelectImplementation : public CancelHandler, public SelectBuilder<R>, public SelectInstanceInternal<R> {
public:
    /**
     * Essentially, the `select` operation is split into three phases: REGISTRATION, WAITING, and COMPLETION.
     *
     * == Phase 1: REGISTRATION ==
     * (detailed description from original Kotlin code preserved in comments)
     *
     * == Phase 2: WAITING ==
     * (detailed description from original Kotlin code preserved in comments)
     *
     * == Phase 3: COMPLETION ==
     * (detailed description from original Kotlin code preserved in comments)
     *
     * (full state machine diagram preserved in comments from original)
     */

    CoroutineContext context;

private:
    /**
     * The state of this `select` operation. See the description above for details.
     */
    std::atomic<void*> state;

    /**
     * Returns `true` if this `select` instance is in the REGISTRATION phase;
     * otherwise, returns `false`.
     */
    bool in_registration_phase() const {
        void* val = state.load();
        return val == kStateReg || /* TODO: check if List<*> */ false;
    }

    /**
     * Returns `true` if this `select` is already selected;
     * thus, other parties are bound to fail when making a rendezvous with it.
     */
    bool is_selected() const {
        // TODO: instanceof check for ClauseData
        return false;
    }

    /**
     * Returns `true` if this `select` is cancelled.
     */
    bool is_cancelled() const {
        return state.load() == kStateCancelled;
    }

    /**
     * List of clauses waiting on this `select` instance.
     *
     * This property is the subject to benign data race: concurrent cancellation might null-out this property
     * while [trySelect] operation reads it and iterates over its content.
     * A logical race is resolved by the consensus on [state] property.
     */
    // @BenignDataRace
    std::vector<ClauseData>* clauses;

    /**
     * Stores the completion action provided through [disposeOnCompletion] or [invokeOnCancellation]
     * during clause registration. After that, if the clause is successfully registered
     * (so, it has not completed immediately), this handler is stored into
     * the corresponding [ClauseData] instance.
     *
     * Note that either [DisposableHandle] is provided, or a [Segment] instance with
     * the index in it, which specify the location of storing this `select`.
     * In the latter case, [Segment.onCancellation] should be called on completion/cancellation.
     */
    void* disposable_handle_or_segment;

    /**
     * In case the disposable handle is specified via [Segment]
     * and index in it, implying calling [Segment.onCancellation],
     * the corresponding index is stored in this field.
     * The segment is stored in [disposableHandleOrSegment].
     */
    int index_in_segment;

    /**
     * Stores the result passed via [selectInRegistrationPhase] during clause registration
     * or [trySelect], which is called by another coroutine trying to make a rendezvous
     * with this `select` instance. Further, this result is processed via the
     * [ProcessResultFunction] of the selected clause.
     *
     * This property is the subject to benign data race.
     */
    // @BenignDataRace
    void* internal_result;

public:
    explicit SelectImplementation(CoroutineContext context_)
        : context(context_),
          state(kStateReg),
          clauses(new std::vector<ClauseData>(/* initial capacity */ 2)),
          disposable_handle_or_segment(nullptr),
          index_in_segment(-1),
          internal_result(kNoResult) {}

    /**
     * This function is called after the [SelectBuilder] is applied. In case one of the clauses is already selected,
     * the algorithm applies the corresponding [ProcessResultFunction] and invokes the user-specified [block][ClauseData.block].
     * Otherwise, it moves this `select` to WAITING phase (re-registering clauses if needed), suspends until a rendezvous
     * is happened, and then completes the operation by applying the corresponding [ProcessResultFunction] and
     * invoking the user-specified [block][ClauseData.block].
     */
    // @PublishedApi
    virtual R do_select() {
        // TODO: suspend function semantics not implemented
        if (is_selected()) {
            return complete();  // Fast path
        } else {
            return do_select_suspend();  // Slow path
        }
    }

private:
    // We separate the following logic as it has two suspension points
    // and, therefore, breaks the tail-call optimization if it were
    // inlined in [doSelect]
    R do_select_suspend() {
        // TODO: suspend function semantics not implemented
        // In case no clause has been selected during registration,
        // the `select` operation suspends and waits for a rendezvous.
        wait_until_selected(); // <-- suspend call => no tail-call optimization here
        // There is a selected clause! Apply the corresponding
        // [ProcessResultFunction] and invoke the user-specified block.
        return complete(); // <-- one more suspend call
    }

    // ========================
    // = CLAUSES REGISTRATION =
    // ========================

public:
    void invoke(SelectClause0& clause, std::function<R()> block) override {
        ClauseData clause_data(
            clause.get_clause_object(),
            clause.get_reg_func(),
            clause.get_process_res_func(),
            kParamClause0,
            block,
            clause.get_on_cancellation_constructor()
        );
        clause_data.register_clause();
    }

    template<typename Q>
    void invoke(SelectClause1<Q>& clause, std::function<R(Q)> block) override {
        ClauseData clause_data(
            clause.get_clause_object(),
            clause.get_reg_func(),
            clause.get_process_res_func(),
            nullptr,
            block,
            clause.get_on_cancellation_constructor()
        );
        clause_data.register_clause();
    }

    template<typename P, typename Q>
    void invoke(SelectClause2<P, Q>& clause, P param, std::function<R(Q)> block) override {
        ClauseData clause_data(
            clause.get_clause_object(),
            clause.get_reg_func(),
            clause.get_process_res_func(),
            param,
            block,
            clause.get_on_cancellation_constructor()
        );
        clause_data.register_clause();
    }

    /**
     * Attempts to register this `select` clause. If another clause is already selected,
     * this function does nothing and completes immediately.
     * Otherwise, it registers this `select` instance in
     * the [clause object][ClauseData.clauseObject]
     * according to the provided [registration function][ClauseData.regFunc].
     * On success, this `select` instance is stored as a waiter
     * in the clause object -- the algorithm also stores
     * the provided via [disposeOnCompletion] completion action
     * and adds the clause to the list of registered one.
     * In case of registration failure, the internal result
     * (not processed by [ProcessResultFunction] yet) must be
     * provided via [selectInRegistrationPhase] -- the algorithm
     * updates the state to this clause reference.
     */
    // @JvmName("register")
    void register_clause(ClauseData& clause_data, bool reregister = false) {
        // TODO: assert implementation
        // assert { state.value !== STATE_CANCELLED }
        // Is there already selected clause?
        // TODO: type check for ClauseData
        if (/* state.value is ClauseData */ false) return;
        // For new clauses, check that there does not exist
        // another clause with the same object.
        if (!reregister) check_clause_object(clause_data.clause_object);
        // Try to register in the corresponding object.
        if (try_register_as_waiter(clause_data, this)) {
            // Successfully registered, and this `select` instance
            // is stored as a waiter. Add this clause to the list
            // of registered clauses and store the provided via
            // [invokeOnCompletion] completion action into the clause.
            if (!reregister) clauses->push_back(clause_data);
            clause_data.disposable_handle_or_segment = this->disposable_handle_or_segment;
            clause_data.index_in_segment = this->index_in_segment;
            this->disposable_handle_or_segment = nullptr;
            this->index_in_segment = -1;
        } else {
            // This clause has been selected!
            // Update the state correspondingly.
            state.store(&clause_data);
        }
    }

private:
    /**
     * Checks that there does not exist another clause with the same object.
     */
    void check_clause_object(void* clause_object) {
        // Read the list of clauses, it is guaranteed that it is non-null.
        auto* clauses_list = clauses;
        // Check that there does not exist another clause with the same object.
        for (auto& c : *clauses_list) {
            if (c.clause_object == clause_object) {
                // TODO: proper error handling
                throw std::runtime_error("Cannot use select clauses on the same object");
            }
        }
    }

public:
    void dispose_on_completion(DisposableHandle* disposable_handle) override {
        this->disposable_handle_or_segment = disposable_handle;
    }

    /**
     * An optimized version for the code below that does not allocate
     * a cancellation handler object and efficiently stores the specified
     * [segment] and [index].
     */
    void invoke_on_cancellation(Segment<void*>* segment, int index) {
        this->disposable_handle_or_segment = segment;
        this->index_in_segment = index;
    }

    void select_in_registration_phase(void* internal_result_) override {
        this->internal_result = internal_result_;
    }

    // =========================
    // = WAITING FOR SELECTION =
    // =========================

private:
    /**
     * Suspends and waits until some clause is selected.
     */
    void wait_until_selected() {
        // TODO: suspend function semantics not implemented
        // TODO: suspendCancellableCoroutine implementation
    }

    /**
     * Re-registers the clause with the specified
     * [clause object][clauseObject] after unsuccessful
     * [trySelect] of this clause while the `select`
     * was still in REGISTRATION phase.
     */
    void reregister_clause(void* clause_object) {
        // TODO: find clause and re-register
    }

    // ==============
    // = RENDEZVOUS =
    // ==============

public:
    bool try_select(void* clause_object, void* result) override {
        return try_select_internal(clause_object, result) == kTrySelectSuccessful;
    }

    /**
     * Similar to [trySelect] but provides a failure reason
     * if this rendezvous is unsuccessful. We need this function
     * in the channel implementation.
     */
    TrySelectDetailedResult try_select_detailed(void* clause_object, void* result) {
        return static_cast<TrySelectDetailedResult>(try_select_internal(clause_object, result));
    }

private:
    int try_select_internal(void* clause_object, void* internal_result_) {
        // TODO: complex CAS loop implementation
        return kTrySelectSuccessful;
    }

    /**
     * Finds the clause with the corresponding [clause object][SelectClause.clauseObject].
     * If the reference to the list of clauses is already cleared due to completion/cancellation,
     * this function returns `null`
     */
    ClauseData* find_clause(void* clause_object) {
        // TODO: find clause in list
        return nullptr;
    }

    // ==============
    // = COMPLETION =
    // ==============

    /**
     * Completes this `select` operation after the internal result is provided
     * via [SelectInstance.trySelect] or [SelectInstance.selectInRegistrationPhase].
     */
    R complete() {
        // TODO: suspend function semantics not implemented
        // TODO: full completion logic
        return R{};
    }

    /**
     * Invokes all [DisposableHandle]-s provided via
     * [SelectInstance.disposeOnCompletion] during
     * clause registrations.
     */
    void cleanup(ClauseData& selected_clause) {
        // TODO: cleanup implementation
    }

    // [CompletionHandler] implementation, must be invoked on cancellation.
public:
    void invoke(std::exception_ptr cause) override {
        // TODO: cancellation handling
    }

    /**
     * Each `select` clause is internally represented with a [ClauseData] instance.
     */
    class ClauseData {
    public:
        // @JvmField
        void* clause_object; // the object of this `select` clause: Channel, Mutex, Job, ...
        RegistrationFunction reg_func;
        ProcessResultFunction process_res_func;
        void* param; // the user-specified param
        void* block; // the user-specified block, which should be called if this clause becomes selected
        // @JvmField
        OnCancellationConstructor* on_cancellation_constructor;

        // @JvmField
        void* disposable_handle_or_segment;
        // @JvmField
        int index_in_segment;

        ClauseData(
            void* clause_object_,
            RegistrationFunction reg_func_,
            ProcessResultFunction process_res_func_,
            void* param_,
            void* block_,
            OnCancellationConstructor* on_cancellation_constructor_
        ) : clause_object(clause_object_),
            reg_func(reg_func_),
            process_res_func(process_res_func_),
            param(param_),
            block(block_),
            on_cancellation_constructor(on_cancellation_constructor_),
            disposable_handle_or_segment(nullptr),
            index_in_segment(-1) {}

        /**
         * Tries to register the specified [select] instance in [clauseObject] and check
         * whether the registration succeeded or a rendezvous has happened during the registration.
         */
        bool try_register_as_waiter(SelectImplementation<R>* select) {
            // TODO: registration logic
            return false;
        }

        /**
         * Processes the internal result provided via either
         * [SelectInstance.selectInRegistrationPhase] or
         * [SelectInstance.trySelect] and returns an argument
         * for the user-specified [block].
         */
        void* process_result(void* result) {
            return process_res_func(clause_object, param, result);
        }

        /**
         * Invokes the user-specified block and returns
         * the final result of this `select` clause.
         */
        R invoke_block(void* argument) {
            // TODO: suspend function semantics not implemented
            // TODO: distinguish PARAM_CLAUSE_0 vs regular params
            return R{};
        }

        void dispose() {
            // TODO: dispose implementation
        }

        auto create_on_cancellation_action(SelectInstance<R>* select, void* internal_result_) {
            if (on_cancellation_constructor != nullptr) {
                return (*on_cancellation_constructor)(select, param, internal_result_);
            }
            return decltype((*on_cancellation_constructor)(select, param, internal_result_)){};
        }
    };
};

bool try_resume(CancellableContinuation<void*>& cont,
                std::function<void(std::exception_ptr, void*, CoroutineContext)> on_cancellation) {
    // TODO: tryResume implementation
    return false;
}

// trySelectInternal(..) results.
constexpr int kTrySelectSuccessful = 0;
constexpr int kTrySelectReregister = 1;
constexpr int kTrySelectCancelled = 2;
constexpr int kTrySelectAlreadySelected = 3;

// trySelectDetailed(..) results.
enum class TrySelectDetailedResult {
    kSuccessful,
    kReregister,
    kCancelled,
    kAlreadySelected
};

TrySelectDetailedResult to_try_select_detailed_result(int try_select_internal_result) {
    switch (try_select_internal_result) {
        case kTrySelectSuccessful: return TrySelectDetailedResult::kSuccessful;
        case kTrySelectReregister: return TrySelectDetailedResult::kReregister;
        case kTrySelectCancelled: return TrySelectDetailedResult::kCancelled;
        case kTrySelectAlreadySelected: return TrySelectDetailedResult::kAlreadySelected;
        default:
            throw std::runtime_error("Unexpected internal result");
    }
}

// Markers for REGISTRATION, COMPLETED, and CANCELLED states.
// TODO: Kotlin Symbol type - using sentinel pointers or enum
void* kStateReg = reinterpret_cast<void*>(0x1);
void* kStateCompleted = reinterpret_cast<void*>(0x2);
void* kStateCancelled = reinterpret_cast<void*>(0x3);

// As the selection result is nullable, we use this special
// marker for the absence of result.
void* kNoResult = reinterpret_cast<void*>(0x4);

// We use this marker parameter objects to distinguish
// SelectClause[0,1,2] and invoke the user-specified block correctly.
void* kParamClause0 = reinterpret_cast<void*>(0x5);

} // namespace selects
} // namespace coroutines
} // namespace kotlinx
