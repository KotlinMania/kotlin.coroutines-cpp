// port-lint: source compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:17-123
#include "TailSuspendCallsCollector.hpp"
#include "SuspendFunctionAnalyzer.hpp"
#include "clang/AST/AST.h"
#include <stdexcept>

namespace org::jetbrains::kotlin::backend::common {
namespace {
using namespace clang;
using kotlinx::suspend::SuspendFunctionAnalyzer;
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:27
struct VisitorState { bool inside_try_block; bool is_tail_expression; };

// NOTE(port): Clang returns target their current function; nested function bodies
// are not children of this walk. Clang has no IrReturnableBlock symbols.
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:29-119
class Visitor {
public:
    explicit Visitor(bool is_unit_return) : is_unit_return_(is_unit_return) {}
    TailSuspendCalls result{{}, false};

    // NOTE(port): Clang node dispatch supplies Kotlin IrElement.accept visitor selection.
    void accept(const Stmt* element, VisitorState data) {
        if (!element) return;
        if (SuspendFunctionAnalyzer::is_unevaluated_expression(element)) return;
        if (const auto* returned = dyn_cast<ReturnStmt>(element)) return visit_return(returned, data);
        if (const auto* body = dyn_cast<CompoundStmt>(element)) return visit_statement_container(body, data);
        if (const auto* region = dyn_cast<CXXTryStmt>(element)) return visit_try(region, data);
        if (const auto* conditional = dyn_cast<ConditionalOperator>(element)) return visit_when(conditional, data);
        if (const auto* branch = dyn_cast<IfStmt>(element)) return visit_when(branch, data);
        // NOTE(port): Parentheses/cleanup nodes add no Kotlin expression operation.
        if (const auto* parentheses = dyn_cast<ParenExpr>(element)) return accept(parentheses->getSubExpr(), data);
        if (const auto* cleanup = dyn_cast<ExprWithCleanups>(element)) return accept(cleanup->getSubExpr(), data);
        // NOTE(port): Clang materializes the DSL's forwarding-reference argument;
        // the Kotlin intrinsic has no corresponding materialization expression.
        if (const auto* temporary = dyn_cast<MaterializeTemporaryExpr>(element)) return accept(temporary->getSubExpr(), data);
        if (const auto* temporary = dyn_cast<CXXBindTemporaryExpr>(element)) return accept(temporary->getSubExpr(), data);
        if (const auto* cast = dyn_cast<CastExpr>(element)) return visit_type_operator(cast, data);
        if (const auto* call = dyn_cast<CallExpr>(element)) return visit_call(call, data);
        // NOTE(port): Local functions are moved out before Kotlin's collector.
        // In Clang, capture initializers execute here but the lambda body does not.
        if (const auto* lambda = dyn_cast<LambdaExpr>(element)) {
            for (const auto* initializer : lambda->capture_inits())
                accept(initializer, {data.inside_try_block, false});
            return;
        }
        visit_element(element, data);
    }
private:
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:35-37
    void visit_element(const Stmt* element, VisitorState data) {
        for (const auto* child : element->children()) accept(child, {data.inside_try_block, false});
    }
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:81-86
    void visit_when(const ConditionalOperator* expression, VisitorState data) {
        accept(expression->getCond(), {data.inside_try_block, false});
        accept(expression->getTrueExpr(), data);
        accept(expression->getFalseExpr(), data);
    }
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:81-86
    // NOTE(port): Clang if statements supply condition/result branches. Init
    // statements and condition declarations execute before those branches.
    void visit_when(const IfStmt* expression, VisitorState data) {
        accept(expression->getInit(), {data.inside_try_block, false});
        accept(expression->getConditionVariableDeclStmt(), {data.inside_try_block, false});
        accept(expression->getCond(), {data.inside_try_block, false});
        accept(expression->getThen(), data);
        accept(expression->getElse(), data);
    }
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:39-43
    void visit_try(const CXXTryStmt* region, VisitorState data) {
        accept(region->getTryBlock(), {true, false});
        for (unsigned i = 0; i < region->getNumHandlers(); ++i)
            accept(region->getHandler(i)->getHandlerBlock(), data);
        // Clang's CXXTryStmt has no finally clause.
    }
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:48-50
    void visit_return(const ReturnStmt* expression, VisitorState data) {
        accept(expression->getRetValue(), {data.inside_try_block, true});
    }
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:64-79
    void visit_statement_container(const CompoundStmt* expression, VisitorState data) {
        for (auto item = expression->body_begin(); item != expression->body_end(); ++item) {
            const auto next = item + 1;
            // The last statement defines the result of the container expression.
            bool is_tail_statement = next == expression->body_end() ? data.is_tail_expression : false;
            if (next != expression->body_end() && is_unit_return_) {
                // In a Unit-returning function, a statement followed by return Unit is tail.
                const auto* returned = dyn_cast<ReturnStmt>(*next);
                is_tail_statement = returned && is_unit_read(returned->getRetValue());
            }
            accept(*item, {data.inside_try_block, is_tail_statement});
        }
    }
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:90-95
    void visit_type_operator(const CastExpr* expression, VisitorState data) {
        // Until type computation removes these operators, the remaining
        // returnIfSuspended call can still be treated as a tail call.
        const auto* argument = dyn_cast<CallExpr>(expression->getSubExpr());
        // For a tail call, returnIfSuspended can be optimized away.
        // NOTE(port): Implicit Clang ABI conversions do not change a direct call's
        // result. Explicit source casts are operations and cannot inherit tail state.
        const bool is_tail_expression = data.is_tail_expression && isa<ImplicitCastExpr>(expression) &&
            (expression->getCastKind() == CK_NoOp || (argument && is_return_if_suspended_call(argument)));
        accept(expression->getSubExpr(), {data.inside_try_block, is_tail_expression});
    }
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:97-108
    void visit_call(const CallExpr* expression, VisitorState data) {
        const bool wrapper = is_return_if_suspended_call(expression);
        // NOTE(port): The DSL wrapper is Kotlin's returnIfSuspended intrinsic,
        // not a second suspend callee surrounding its argument.
        if (!wrapper && SuspendFunctionAnalyzer::is_suspend_call(expression)) {
            if (!data.inside_try_block && data.is_tail_expression) result.call_sites.insert(expression);
            else result.has_not_tail_suspend_calls = true;
        }
        // For a tail call, returnIfSuspended can be optimized away.
        const bool is_tail_expression = data.is_tail_expression && wrapper;
        for (const auto* child : expression->children())
            accept(child, {data.inside_try_block, is_tail_expression});
    }
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:110-115
    bool is_unit_read(const Expr* expression) const {
        if (const auto* operation = dyn_cast_or_null<CastExpr>(expression)) return is_unit_read(operation->getSubExpr());
        // NOTE(port): Unit-valued Continuation<void*> entries use a null result box.
        return expression && isa<CXXNullPtrLiteralExpr>(expression);
    }
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:117-118
    bool is_return_if_suspended_call(const CallExpr* expression) const {
        return SuspendFunctionAnalyzer::is_suspend_wrapper(expression);
    }
    bool is_unit_return_;
};
}
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:23-25,121-123
TailSuspendCalls collect_tail_suspend_calls(const clang::FunctionDecl* function) {
    bool is_suspend = false, is_unit_return = false;
    for (const auto* annotation : function->specific_attrs<clang::AnnotateAttr>()) {
        is_suspend |= annotation->getAnnotation() == "suspend";
        // NOTE(port): The erased void* return type cannot encode Kotlin Unit.
        is_unit_return |= annotation->getAnnotation() == "kotlin.ir.UnitReturn";
    }
    if (!is_suspend) throw std::invalid_argument("A suspend function expected");
    if (!function->hasBody()) return {{}, false};
    Visitor visitor(is_unit_return);
    visitor.accept(function->getBody(), {false, true});
    return visitor.result;
}
}
