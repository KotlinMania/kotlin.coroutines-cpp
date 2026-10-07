// port-lint: source compiler/fir/checkers/src/org/jetbrains/kotlin/fir/analysis/checkers/expression/FirSuspendCallChecker.kt
// Transliterated from: compiler/fir/checkers/src/org/jetbrains/kotlin/fir/analysis/checkers/expression/FirSuspendCallChecker.kt:53-73,135-180
#include "FirSuspendCallChecker.hpp"
#include "SuspendFunctionAnalyzer.hpp"
#include "clang/AST/Attr.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Basic/Diagnostic.h"
#include <algorithm>

namespace org::jetbrains::kotlin::fir::analysis::checkers::expression {
// Transliterated from: compiler/fir/checkers/src/org/jetbrains/kotlin/fir/analysis/checkers/expression/FirSuspendCallChecker.kt:53-73
// NOTE(port): This check translates default-value and non-local suspension rules.
// Ordinary explicit C++ ABI calls outside defaults are checked separately from
// Kotlin authoring contexts; the other FIR diagnostic branches remain untranslated.
void FirSuspendCallChecker::check(const clang::CallExpr* expression,
    const std::vector<const clang::Decl*>& containing_declarations,
    clang::DiagnosticsEngine& reporter) {
    if (!kotlinx::suspend::SuspendFunctionAnalyzer::is_suspend_call(expression)) return;
    const auto* enclosing_suspend_function = find_enclosing_suspend_function(containing_declarations);
    if (!enclosing_suspend_function) {
        // NOTE(port): C++ parameter defaults have a concrete declaration stack.
        // Apply the illegal-call diagnostic when the call executes in a default,
        // rather than inventing a suspend owner for an ordinary ABI constructor.
        for (auto declaration = containing_declarations.rbegin(); declaration != containing_declarations.rend(); ++declaration) {
            if (const auto* parameter = llvm::dyn_cast<clang::ParmVarDecl>(*declaration);
                parameter && parameter->hasDefaultArg()) {
                const auto id = reporter.getCustomDiagID(clang::DiagnosticsEngine::Error,
                    "suspend function can only be called from a coroutine or another suspend function");
                reporter.Report(expression->getBeginLoc(), id);
                break;
            }
            if (llvm::isa<clang::FunctionDecl>(*declaration)) break;
        }
    } else {
        if (!check_non_local_return_usage(enclosing_suspend_function, containing_declarations)) {
            const auto id = reporter.getCustomDiagID(clang::DiagnosticsEngine::Error,
                "suspension functions can only be called within coroutine body");
            reporter.Report(expression->getBeginLoc(), id);
        }
        if (is_in_scope_for_default_parameter_values(enclosing_suspend_function, containing_declarations)) {
            const auto id = reporter.getCustomDiagID(clang::DiagnosticsEngine::Error,
                "suspend function call in default parameter value is unsupported");
            reporter.Report(expression->getBeginLoc(), id);
        }
    }
}

// Transliterated from: compiler/fir/checkers/src/org/jetbrains/kotlin/fir/analysis/checkers/expression/FirSuspendCallChecker.kt:164-180
bool FirSuspendCallChecker::check_non_local_return_usage(
    const clang::FunctionDecl* enclosing_suspend_function,
    const std::vector<const clang::Decl*>& containing_declarations) {
    for (auto declaration = containing_declarations.rbegin(); declaration != containing_declarations.rend(); ++declaration) {
        // If we found the nearest suspend function, we're finished.
        if (*declaration == enclosing_suspend_function) return true;
        // Local variables are okay.
        if (const auto* property = llvm::dyn_cast<clang::VarDecl>(*declaration);
            property && !llvm::isa<clang::ParmVarDecl>(property) && property->isLocalVarDecl()) continue;
        // Inline lambdas that allow non-local returns are okay.
        if (const auto* function = llvm::dyn_cast<clang::CXXMethodDecl>(*declaration);
            function && function->getParent()->isLambda()) {
            // NOTE(port): Kotlin's inline return permission uses its IR annotation.
            // Cross-inline and non-inline states do not allow non-local returns.
            bool return_allowed = false;
            for (const auto* attribute : function->specific_attrs<clang::AnnotateAttr>())
                return_allowed |= attribute->getAnnotation() == "kotlin.ir.Inline";
            if (return_allowed) continue;
        }
        // Default parameter suspension receives its own unsupported diagnostic.
        if (llvm::isa<clang::ParmVarDecl>(*declaration)) continue;
        // Local classes, initializer blocks and non-inline lambdas are not okay.
        return false;
    }
    return false;
}

// Transliterated from: compiler/fir/checkers/src/org/jetbrains/kotlin/fir/analysis/checkers/expression/FirSuspendCallChecker.kt:135-147
const clang::FunctionDecl* FirSuspendCallChecker::find_enclosing_suspend_function(
    const std::vector<const clang::Decl*>& containing_declarations) {
    for (auto declaration = containing_declarations.rbegin(); declaration != containing_declarations.rend(); ++declaration) {
        if (const auto* function = llvm::dyn_cast<clang::FunctionDecl>(*declaration)) {
            // NOTE(port): Resolved Kotlin suspend callable types and suspend
            // modifiers are represented by the same Clang function annotation.
            for (const auto* attribute : function->specific_attrs<clang::AnnotateAttr>())
                if (attribute->getAnnotation() == "suspend") return function;
        }
    }
    return nullptr;
}

// Transliterated from: compiler/fir/checkers/src/org/jetbrains/kotlin/fir/analysis/checkers/expression/FirSuspendCallChecker.kt:149-162
bool FirSuspendCallChecker::is_in_scope_for_default_parameter_values(
    const clang::FunctionDecl* enclosing_function,
    const std::vector<const clang::Decl*>& containing_declarations) {
    const auto value_parameters = enclosing_function->parameters();
    for (auto declaration = containing_declarations.rbegin(); declaration != containing_declarations.rend(); ++declaration) {
        if (const auto* parameter = llvm::dyn_cast<clang::ParmVarDecl>(*declaration);
            parameter && std::find(value_parameters.begin(), value_parameters.end(), parameter) != value_parameters.end() &&
            parameter->hasDefaultArg()) return true;
        if (const auto* function = llvm::dyn_cast<clang::FunctionDecl>(*declaration)) {
            // NOTE(port): Kotlin inline status is an explicit IR annotation;
            // C++ inline linkage does not imply Kotlin non-local execution.
            bool is_inline = false;
            for (const auto* attribute : function->specific_attrs<clang::AnnotateAttr>())
                is_inline |= attribute->getAnnotation() == "kotlin.ir.Inline";
            if (is_inline) continue;
            return false;
        }
    }
    return false;
}
}
