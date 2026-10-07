// port-lint: source compiler/fir/checkers/src/org/jetbrains/kotlin/fir/analysis/checkers/expression/FirSuspendCallChecker.kt
// Transliterated from: compiler/fir/checkers/src/org/jetbrains/kotlin/fir/analysis/checkers/expression/FirSuspendCallChecker.kt:33-78,135-180
#ifndef KOTLINX_FIR_SUSPEND_CALL_CHECKER_HPP
#define KOTLINX_FIR_SUSPEND_CALL_CHECKER_HPP
#include <vector>
namespace clang {
class CallExpr;
class Decl;
class FunctionDecl;
class DiagnosticsEngine;
}
namespace org::jetbrains::kotlin::fir::analysis::checkers::expression {
// NOTE(port): Clang declarations supply the containing-declaration context.
// Transliterated from: compiler/fir/checkers/src/org/jetbrains/kotlin/fir/analysis/checkers/expression/FirSuspendCallChecker.kt:33-78
class FirSuspendCallChecker {
public:
    static void check(const clang::CallExpr* expression,
                      const std::vector<const clang::Decl*>& containing_declarations,
                      clang::DiagnosticsEngine& reporter);
private:
    static const clang::FunctionDecl* find_enclosing_suspend_function(
        const std::vector<const clang::Decl*>& containing_declarations);
    static bool check_non_local_return_usage(
        const clang::FunctionDecl* enclosing_suspend_function,
        const std::vector<const clang::Decl*>& containing_declarations);
    static bool is_in_scope_for_default_parameter_values(
        const clang::FunctionDecl* enclosing_function,
        const std::vector<const clang::Decl*>& containing_declarations);
};
}
#endif
