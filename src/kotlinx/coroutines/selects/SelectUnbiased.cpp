// port-lint: source selects/SelectUnbiased.kt
/**
 * @file SelectUnbiased.cpp
 * @brief Unbiased select implementation
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/selects/SelectUnbiased.kt
 *
 * Waits for the result of multiple suspending functions simultaneously like select, but in an _unbiased_
 * way when multiple clauses are selectable at the same time.
 *
 * This unbiased implementation of select expression randomly shuffles the clauses before checking
 * if they are selectable, thus ensuring that there is no statistical bias to the selection of the first
 * clauses.
 */

#include "kotlinx/coroutines/selects/Select.hpp"
#include <vector>
#include <algorithm>
#include <random>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace selects {

// ============================================================================
// Line 37-64: UnbiasedSelectImplementation
// ============================================================================

/**
 * Sentinel value for SelectClause0 param (no-argument clause).
 *
 * Transliterated from: private object PARAM_CLAUSE_0
 */
struct PARAM_CLAUSE_0_t {};
inline constexpr PARAM_CLAUSE_0_t PARAM_CLAUSE_0{};

/**
 * Data structure to hold clause registration information.
 *
 * Transliterated from:
 * ClauseData(clauseObject, regFunc, processResFunc, param, block, onCancellationConstructor)
 */
struct ClauseData {
    void* clause_object;
    std::function<void()> reg_func;
    std::function<void*()> process_res_func;
    void* param;
    std::function<void*()> block;
    std::function<void()> on_cancellation_constructor;

    /**
     * Registers this clause with the select.
     */
    void register_clause() {
        if (reg_func) {
            reg_func();
        }
    }
};

/**
 * The unbiased `select` inherits the standard SelectImplementation,
 * but does not register clauses immediately. Instead, it stores all of them
 * in clausesToRegister list, shuffles and registers them in the beginning of doSelect
 * (see shuffleAndRegisterClauses), and then delegates the rest
 * to the parent's doSelect implementation.
 *
 * Transliterated from:
 * internal open class UnbiasedSelectImplementation<R>(context: CoroutineContext) : SelectImplementation<R>(context)
 */
template <typename R>
class UnbiasedSelectImplementation : public SelectImplementation<R> {
public:
    /**
     * Constructor.
     *
     * @param context The coroutine context for this select
     */
    explicit UnbiasedSelectImplementation(std::shared_ptr<CoroutineContext> context)
        : SelectImplementation<R>(context)
        , rng_(std::random_device{}())
    {}

    /**
     * Line 40-42: SelectClause0 invoke override
     *
     * Instead of registering immediately, stores the clause data for later shuffled registration.
     */
    void invoke(SelectClause0& clause, std::function<R()> block) {
        clauses_to_register_.push_back(ClauseData{
            &clause,
            [&clause, this]() { SelectImplementation<R>::invoke(clause, {}); },
            nullptr,
            const_cast<PARAM_CLAUSE_0_t*>(&PARAM_CLAUSE_0),
            [block]() -> void* { return new R(block()); },
            nullptr
        });
    }

    /**
     * Line 44-46: SelectClause1 invoke override
     *
     * Stores clause data for deferred shuffled registration.
     */
    template <typename Q>
    void invoke(SelectClause1<Q>& clause, std::function<R(Q)> block) {
        clauses_to_register_.push_back(ClauseData{
            &clause,
            [&clause, this]() { SelectImplementation<R>::template invoke_clause1(clause); },
            nullptr,
            nullptr,
            nullptr,
            nullptr
        });
    }

    /**
     * Line 48-50: SelectClause2 invoke override
     *
     * Stores clause data with param for deferred shuffled registration.
     */
    template <typename P, typename Q>
    void invoke(SelectClause2<P, Q>& clause, P param, std::function<R(Q)> block) {
        clauses_to_register_.push_back(ClauseData{
            &clause,
            [&clause, param, this]() { SelectImplementation<R>::template invoke_clause2(clause, param); },
            nullptr,
            reinterpret_cast<void*>(new P(param)),
            nullptr,
            nullptr
        });
    }

    /**
     * Line 52-56: doSelect override
     *
     * Shuffles and registers all clauses before delegating to parent's doSelect.
     *
     * Transliterated from:
     * override suspend fun doSelect(): R {
     *     shuffleAndRegisterClauses()
     *     return super.doSelect()
     * }
     */
    R do_select() {
        shuffle_and_register_clauses();
        return SelectImplementation<R>::do_select();
    }

private:
    std::vector<ClauseData> clauses_to_register_;
    std::mt19937 rng_;

    /**
     * Line 58-63: shuffleAndRegisterClauses
     *
     * Shuffles the clauses randomly and registers them, ensuring unbiased selection.
     *
     * Transliterated from:
     * private fun shuffleAndRegisterClauses() = try {
     *     clausesToRegister.shuffle()
     *     clausesToRegister.forEach { it.register() }
     * } finally {
     *     clausesToRegister.clear()
     * }
     */
    void shuffle_and_register_clauses() {
        try {
            // Shuffle clauses for unbiased selection
            std::shuffle(clauses_to_register_.begin(), clauses_to_register_.end(), rng_);
            
            // Register all clauses in shuffled order
            for (auto& clause_data : clauses_to_register_) {
                clause_data.register_clause();
            }
        } catch (...) {
            clauses_to_register_.clear();
            throw;
        }
        clauses_to_register_.clear();
    }
};

// ============================================================================
// Line 18-27: selectUnbiased function
// ============================================================================

/**
 * Waits for the result of multiple suspending functions simultaneously like select, but in an _unbiased_
 * way when multiple clauses are selectable at the same time.
 *
 * This unbiased implementation of select expression randomly shuffles the clauses before checking
 * if they are selectable, thus ensuring that there is no statistical bias to the selection of the first
 * clauses.
 *
 * See select function description for all the other details.
 *
 * Transliterated from:
 * public suspend inline fun <R> selectUnbiased(crossinline builder: SelectBuilder<R>.() -> Unit): R
 *
 * @param context The coroutine context
 * @param builder Function to configure the select builder
 * @return The result of the selected clause
 */
template <typename R>
R select_unbiased(std::shared_ptr<CoroutineContext> context, std::function<void(SelectBuilder<R>&)> builder) {
    UnbiasedSelectImplementation<R> impl(context);
    builder(impl);
    return impl.do_select();
}

} // namespace selects
} // namespace coroutines
} // namespace kotlinx