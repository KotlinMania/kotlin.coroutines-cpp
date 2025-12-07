#include <functional>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/selects/SelectUnbiased.kt
//
// TODO: This is a mechanical syntax transliteration. The following Kotlin constructs need proper C++ implementation:
// - @file:OptIn annotation (kept as comment)
// - suspend functions (marked but not implemented as C++20 coroutines)
// - inline functions with crossinline parameters
// - Kotlin contracts (contract { ... })
// - Lambda types and closures
// - @ExperimentalContracts, @OptIn annotations (kept as comments)

// @file:OptIn(ExperimentalContracts::class)

namespace kotlinx {
namespace coroutines {
namespace selects {

// import kotlin.contracts.*
// import kotlin.coroutines.*

/**
 * Waits for the result of multiple suspending functions simultaneously like [select], but in an _unbiased_
 * way when multiple clauses are selectable at the same time.
 *
 * This unbiased implementation of `select` expression randomly shuffles the clauses before checking
 * if they are selectable, thus ensuring that there is no statistical bias to the selection of the first
 * clauses.
 *
 * See [select] function description for all the other details.
 */
// @OptIn(ExperimentalContracts::class)
template<typename R, typename BuilderFunc>
R select_unbiased(BuilderFunc&& builder) {
    // TODO: suspend function semantics not implemented
    // TODO: Kotlin contract { callsInPlace(builder, InvocationKind.EXACTLY_ONCE) } not applicable in C++
    // contract {
    //     callsInPlace(builder, InvocationKind.EXACTLY_ONCE)
    // }

    // TODO: coroutineContext not available in C++ without coroutine infrastructure
    UnbiasedSelectImplementation<R> select_impl(/* coroutineContext */ CoroutineContext{});
    builder(select_impl);
    // TAIL-CALL OPTIMIZATION: the only
    // suspend call is at the last position.
    return select_impl.do_select();
}

/**
 * The unbiased `select` inherits the [standard one][SelectImplementation],
 * but does not register clauses immediately. Instead, it stores all of them
 * in [clausesToRegister] lists, shuffles and registers them in the beginning of [doSelect]
 * (see [shuffleAndRegisterClauses]), and then delegates the rest
 * to the parent's [doSelect] implementation.
 */
// @PublishedApi
template<typename R>
class UnbiasedSelectImplementation : SelectImplementation<R> {
private:
    std::vector<typename SelectImplementation<R>::ClauseData> clauses_to_register;

public:
    explicit UnbiasedSelectImplementation(CoroutineContext context)
        : SelectImplementation<R>(context) {}

    void invoke(SelectClause0& clause, std::function<R()> block) override {
        clauses_to_register.push_back(
            typename SelectImplementation<R>::ClauseData(
                clause.clause_object,
                clause.reg_func,
                clause.process_res_func,
                kParamClause0,
                block,
                clause.on_cancellation_constructor
            )
        );
    }

    template<typename Q>
    void invoke(SelectClause1<Q>& clause, std::function<R(Q)> block) override {
        clauses_to_register.push_back(
            typename SelectImplementation<R>::ClauseData(
                clause.clause_object,
                clause.reg_func,
                clause.process_res_func,
                nullptr,
                block,
                clause.on_cancellation_constructor
            )
        );
    }

    template<typename P, typename Q>
    void invoke(SelectClause2<P, Q>& clause, P param, std::function<R(Q)> block) override {
        clauses_to_register.push_back(
            typename SelectImplementation<R>::ClauseData(
                clause.clause_object,
                clause.reg_func,
                clause.process_res_func,
                param,
                block,
                clause.on_cancellation_constructor
            )
        );
    }

    // @PublishedApi
    R do_select() override {
        // TODO: suspend function semantics not implemented
        shuffle_and_register_clauses();
        return SelectImplementation<R>::do_select();
    }

private:
    void shuffle_and_register_clauses() {
        try {
            // TODO: std::shuffle or equivalent for random shuffle
            std::random_shuffle(clauses_to_register.begin(), clauses_to_register.end());
            for (auto& clause : clauses_to_register) {
                clause.register_clause();
            }
        } catch (...) {
            clauses_to_register.clear();
            throw;
        }
        clauses_to_register.clear();
    }
};

} // namespace selects
} // namespace coroutines
} // namespace kotlinx
