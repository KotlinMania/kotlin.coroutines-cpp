#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/selects/SelectUnbiased.kt
 *
 * Kotlin file header (translated):
 *   @file:OptIn(ExperimentalContracts::class)
 *   package kotlinx.coroutines.selects
 */

#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <random>
#include <vector>

namespace kotlinx::coroutines::selects {

/** Sentinel object matching upstream `private val PARAM_CLAUSE_0 = Any()`. */
struct ParamClause0 {};
inline constexpr ParamClause0 PARAM_CLAUSE_0{};

/**
 * Data structure to hold clause registration information.
 *
 * Mirrors upstream's internal `ClauseData(clauseObject, regFunc, processResFunc, param,
 * block, onCancellationConstructor)`.
 */
struct ClauseData {
    void* clause_object = nullptr;
    std::function<void()> reg_func;
    std::function<void*()> process_res_func;
    void* param = nullptr;
    std::function<void*()> block;
    std::function<void()> on_cancellation_constructor;

    /** Upstream: `it.register()` inside `forEach { it.register() }`. */
    void register_clause() {
        if (reg_func) reg_func();
    }
};

/**
 * The unbiased `select` inherits the [standard one][SelectImplementation],
 * but does not register clauses immediately. Instead, it stores all of them
 * in [clauses_to_register] lists, shuffles and registers them at the start of [do_select]
 * (see [shuffle_and_register_clauses]), and then delegates the rest to the parent's
 * [do_select] implementation.
 *
 * Upstream:
 *   @PublishedApi
 *   internal open class UnbiasedSelectImplementation<R>(context: CoroutineContext)
 *       : SelectImplementation<R>(context)
 */
template <typename R>
class UnbiasedSelectImplementation : public SelectImplementation<R> {
public:
    explicit UnbiasedSelectImplementation(std::shared_ptr<CoroutineContext> context)
        : SelectImplementation<R>(std::move(context)),
          rng_(std::random_device{}()) {}

    /**
     * Upstream:
     *   override fun SelectClause0.invoke(block: suspend () -> R) {
     *       clausesToRegister += ClauseData(clauseObject, regFunc, processResFunc, PARAM_CLAUSE_0, block, onCancellationConstructor)
     *   }
     */
    void invoke(SelectClause0& clause, std::function<R()> block) override {
        clauses_to_register_.push_back(ClauseData{
            clause.clause_object(),
            [&clause, this]() { SelectImplementation<R>::invoke(clause, {}); },
            nullptr,
            const_cast<ParamClause0*>(&PARAM_CLAUSE_0),
            [block = std::move(block)]() -> void* { return new R(block()); },
            nullptr,
        });
    }

    /**
     * Upstream:
     *   override fun <Q> SelectClause1<Q>.invoke(block: suspend (Q) -> R) { clausesToRegister += ... }
     */
    template <typename Q>
    void invoke(SelectClause1<Q>& clause, std::function<R(Q)> block) {
        clauses_to_register_.push_back(ClauseData{
            clause.clause_object(),
            [&clause, this]() { SelectImplementation<R>::template invoke_clause1(clause); },
            nullptr,
            nullptr,
            nullptr,
            nullptr,
        });
        (void)block;
    }

    /**
     * Upstream:
     *   override fun <P, Q> SelectClause2<P, Q>.invoke(param: P, block: suspend (Q) -> R) { ... }
     */
    template <typename P, typename Q>
    void invoke(SelectClause2<P, Q>& clause, P param, std::function<R(Q)> block) {
        clauses_to_register_.push_back(ClauseData{
            clause.clause_object(),
            [&clause, param, this]() {
                SelectImplementation<R>::template invoke_clause2(clause, param);
            },
            nullptr,
            reinterpret_cast<void*>(new P(param)),
            nullptr,
            nullptr,
        });
        (void)block;
    }

    /**
     * Upstream:
     *   @PublishedApi
     *   override suspend fun doSelect(): R {
     *       shuffleAndRegisterClauses()
     *       return super.doSelect()
     *   }
     */
    R do_select() override {
        shuffle_and_register_clauses();
        return SelectImplementation<R>::do_select();
    }

private:
    std::vector<ClauseData> clauses_to_register_;
    std::mt19937 rng_;

    /**
     * Upstream:
     *   private fun shuffleAndRegisterClauses() = try {
     *       clausesToRegister.shuffle()
     *       clausesToRegister.forEach { it.register() }
     *   } finally {
     *       clausesToRegister.clear()
     *   }
     */
    void shuffle_and_register_clauses() {
        try {
            std::shuffle(clauses_to_register_.begin(), clauses_to_register_.end(), rng_);
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

/**
 * Waits for the result of multiple suspending functions simultaneously like [select], but in an
 * _unbiased_ way when multiple clauses are selectable at the same time.
 *
 * This unbiased implementation of `select` expression randomly shuffles the clauses before
 * checking if they are selectable, thus ensuring that there is no statistical bias to the
 * selection of the first clauses.
 *
 * Upstream:
 *   @OptIn(ExperimentalContracts::class)
 *   public suspend inline fun <R> selectUnbiased(crossinline builder: SelectBuilder<R>.() -> Unit): R {
 *       contract { callsInPlace(builder, InvocationKind.EXACTLY_ONCE) }
 *       return UnbiasedSelectImplementation<R>(coroutineContext).run {
 *           builder(this)
 *           doSelect()
 *       }
 *   }
 */
template <typename R>
inline R select_unbiased(
    std::shared_ptr<CoroutineContext> context,
    std::function<void(SelectBuilder<R>&)> builder) {
    UnbiasedSelectImplementation<R> impl(std::move(context));
    builder(impl);
    return impl.do_select();
}

} // namespace kotlinx::coroutines::selects
