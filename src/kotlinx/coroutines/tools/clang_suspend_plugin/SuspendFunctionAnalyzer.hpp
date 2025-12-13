#ifndef KOTLINX_SUSPEND_FUNCTION_ANALYZER_HPP
#define KOTLINX_SUSPEND_FUNCTION_ANALYZER_HPP

#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Analysis/CFG.h"

#include <memory>
#include <set>
#include <vector>
#include <map>

namespace kotlinx {
namespace suspend {

/// Information about a single suspension point in a suspend function.
struct SuspendPointInfo {
    const clang::Stmt* suspend_stmt;
    unsigned state_id;
    std::set<const clang::VarDecl*> live_variables;
};

/// Dispatch mode for generated state machines.
enum class DispatchMode {
    Switch,       // Phase 1: switch(_label) { case 0: ... }
    ComputedGoto  // Phase 3: goto *_label; (indirectbr parity)
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
/// Uses Clang's CFG infrastructure and implements backward dataflow
/// liveness analysis matching Kotlin/Native's approach.
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

private:
    /// Build the CFG for the function.
    bool build_cfg();

    /// Traverse the function to collect all local variable declarations.
    void collect_local_variables();

    /// Find all suspension points in the CFG.
    void find_suspend_points();

    /// Perform backward dataflow liveness analysis.
    /// Populates live_variables in each SuspendPointInfo.
    void compute_liveness();

    /// Helper: collect all variables used (read) in a statement.
    void collect_uses(const clang::Stmt* stmt, std::set<const clang::VarDecl*>& uses);

    /// Helper: collect all variables defined (written) in a statement.
    void collect_defs(const clang::Stmt* stmt, std::set<const clang::VarDecl*>& defs);

    /// Helper: find the CFG block containing a statement.
    const clang::CFGBlock* find_block_containing(const clang::Stmt* stmt) const;

    clang::ASTContext& ctx_;
    clang::FunctionDecl* fd_;
    std::unique_ptr<clang::CFG> cfg_;
    std::vector<SuspendPointInfo> suspend_points_;
    std::set<const clang::VarDecl*> spilled_variables_;
    std::vector<const clang::VarDecl*> local_variables_;

    // Maps CFG block ID to the set of variables live at block exit.
    std::map<unsigned, std::set<const clang::VarDecl*>> live_out_;
    // Maps CFG block ID to the set of variables live at block entry.
    std::map<unsigned, std::set<const clang::VarDecl*>> live_in_;
    // Maps statement to its containing CFG block.
    std::map<const clang::Stmt*, const clang::CFGBlock*> stmt_to_block_;
};

} // namespace suspend
} // namespace kotlinx

#endif // KOTLINX_SUSPEND_FUNCTION_ANALYZER_HPP
