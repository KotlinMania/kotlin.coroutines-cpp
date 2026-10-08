// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/CoroutinesLivenessAnalysis.kt:28-44
#ifndef KOTLINX_SUSPEND_FUNCTION_ANALYZER_HPP
#define KOTLINX_SUSPEND_FUNCTION_ANALYZER_HPP

#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include <set>
#include <vector>
#include <map>

namespace kotlinx {
namespace suspend {

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/CoroutinesLivenessAnalysis.kt:29-40
/// Information about a single suspension point in a suspend function.
struct SuspendPointInfo {
    const clang::Stmt* suspend_stmt;
    unsigned state_id;
    std::set<const clang::VarDecl*> live_variables;
    // NOTE(port): Clang shares the declaration expression of C++ defaults.
    // Retain its enclosing use nodes to identify this evaluated occurrence.
    std::vector<const clang::Stmt*> default_expression_path;
};

/// Dispatch mode for generated state machines.
enum class DispatchMode {
    ComputedGoto  // LLVM injector constructs blockaddress/indirectbr dispatch
};

/// Spill mode for variable saving across suspension points.
enum class SpillMode {
    All,       // Phase 1: spill all parameters
    Liveness   // Phase 2: spill only live variables (backward dataflow)
};

/// Analyzes a suspend function to determine:
/// 1. Suspension points (calls to suspend())
/// 2. Live variables at each suspension point (for spilling)
///
/// Uses the translated Kotlin IR call walk and liveness visitor, adapting
/// Clang declarations and evaluated expressions in the original source body.
class SuspendFunctionAnalyzer {
public:
    SuspendFunctionAnalyzer(clang::ASTContext& ctx, clang::FunctionDecl* fd);

    /// Run the full analysis pipeline.
    /// Returns true if analysis succeeded.
    bool analyze();

    /// Get all detected suspension points with their live variable sets.
    const std::vector<SuspendPointInfo>& get_suspend_points() const {
        return suspend_points_;
    }

    /// Get the union of all variables that need spill fields.
    const std::set<const clang::VarDecl*>& get_all_spilled_variables() const {
        return spilled_variables_;
    }

    /// Get all local variables declared in the function.
    const std::vector<const clang::VarDecl*>& get_local_variables() const {
        return local_variables_;
    }

    /// Check if a statement is a suspend call (suspend(expr) or annotated).
    static bool is_suspend_call(const clang::Stmt* stmt);
    static bool is_suspend_wrapper(const clang::CallExpr* call);
    // NOTE(port): C++ type queries have no runtime suspension operation.
    static bool is_unevaluated_expression(const clang::Stmt* statement);
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/InitializersLowering.kt:34-55
    // NOTE(port): Select Clang's resolved initialization, including implicit fields.
    static const clang::InitListExpr* evaluated_initializer_list(const clang::InitListExpr* initializer);
    static bool requires_overload_resolution(const clang::FunctionDecl* function);
    // NOTE(port): Clang adapter for the trailing lowered continuation parameter.
    static const clang::ParmVarDecl* continuation_parameter(const clang::FunctionDecl* function);
    // NOTE(port): Materialize omitted C++ arguments before the continuation.
    static std::string continuation_arguments(const clang::CallExpr* call, const std::string& continuation,
                                             const clang::PrintingPolicy& policy,
                                             const std::vector<std::string>& defaults = {});
    // NOTE(port): Shared Clang printing adapter for retained non-local symbols.
    static bool print_declaration_reference(const clang::DeclRefExpr* reference, clang::ASTContext& context,
                                            const clang::PrintingPolicy& policy, llvm::raw_ostream& output);
    static std::string default_argument(const clang::CXXDefaultArgExpr* argument, const clang::PrintingPolicy& policy);

private:
    /// Traverse the function to collect all local variable declarations.
    void collect_local_variables();

    /// Find suspension points in evaluated source expressions.
    void find_suspend_points();

    /// Perform backward dataflow liveness analysis.
    /// Populates live_variables in each SuspendPointInfo.
    void compute_liveness();

    clang::ASTContext& ctx_;
    clang::FunctionDecl* fd_;
    std::vector<SuspendPointInfo> suspend_points_;
    std::set<const clang::VarDecl*> spilled_variables_;
    std::vector<const clang::VarDecl*> local_variables_;
};

} // namespace suspend
} // namespace kotlinx

#endif // KOTLINX_SUSPEND_FUNCTION_ANALYZER_HPP
