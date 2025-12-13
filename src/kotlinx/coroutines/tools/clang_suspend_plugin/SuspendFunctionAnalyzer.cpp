#include "SuspendFunctionAnalyzer.hpp"

#include "clang/AST/Attr.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"

using namespace clang;

namespace kotlinx {
namespace suspend {

// Annotation string for suspend points.
static constexpr const char* kSuspendAnnot = "suspend";

SuspendFunctionAnalyzer::SuspendFunctionAnalyzer(ASTContext& ctx, FunctionDecl* fd)
    : ctx_(ctx), fd_(fd) {}

bool SuspendFunctionAnalyzer::analyze() {
    if (!fd_ || !fd_->hasBody()) {
        return false;
    }

    if (!build_cfg()) {
        return false;
    }

    collect_local_variables();
    find_suspend_points();

    if (!suspend_points_.empty()) {
        compute_liveness();
    }

    return true;
}

bool SuspendFunctionAnalyzer::build_cfg() {
    CFG::BuildOptions options;
    options.AddEHEdges = true;
    options.AddInitializers = true;
    options.AddImplicitDtors = false;  // Keep it simpler for now
    options.AddTemporaryDtors = false;

    cfg_ = CFG::buildCFG(fd_, fd_->getBody(), &ctx_, options);
    if (!cfg_) {
        return false;
    }

    // Build statement-to-block mapping for quick lookup.
    for (const CFGBlock* block : *cfg_) {
        if (!block) continue;
        for (const CFGElement& elem : *block) {
            if (auto stmt_elem = elem.getAs<CFGStmt>()) {
                stmt_to_block_[stmt_elem->getStmt()] = block;
            }
        }
    }

    return true;
}

/// AST visitor to collect all local variable declarations.
class LocalVariableCollector : public RecursiveASTVisitor<LocalVariableCollector> {
public:
    std::vector<const VarDecl*> variables;

    bool VisitVarDecl(VarDecl* vd) {
        // Only collect local variables, not parameters (handled separately).
        if (vd->isLocalVarDecl() && !isa<ParmVarDecl>(vd)) {
            variables.push_back(vd);
        }
        return true;
    }
};

void SuspendFunctionAnalyzer::collect_local_variables() {
    local_variables_.clear();

    // Add function parameters first.
    for (ParmVarDecl* param : fd_->parameters()) {
        // Skip the completion parameter - it's not spilled.
        if (param->getNameAsString() != "completion") {
            local_variables_.push_back(param);
        }
    }

    // Collect local variables from function body.
    LocalVariableCollector collector;
    collector.TraverseStmt(fd_->getBody());
    for (const VarDecl* vd : collector.variables) {
        local_variables_.push_back(vd);
    }
}

bool SuspendFunctionAnalyzer::is_suspend_call(const Stmt* stmt) {
    if (!stmt) return false;

    // Check for [[clang::annotate("suspend")]] attributed statement.
    if (const auto* attributed = dyn_cast<AttributedStmt>(stmt)) {
        for (const Attr* a : attributed->getAttrs()) {
            if (const auto* ann = dyn_cast<AnnotateAttr>(a)) {
                if (ann->getAnnotation() == kSuspendAnnot) {
                    return true;
                }
            }
        }
    }

    // Check for suspend(expr) wrapper call.
    const Expr* expr = dyn_cast<Expr>(stmt);
    if (!expr) return false;

    // Strip any implicit casts.
    expr = expr->IgnoreParenImpCasts();

    if (const auto* call = dyn_cast<CallExpr>(expr)) {
        if (const FunctionDecl* callee = call->getDirectCallee()) {
            if (callee->getName() == "suspend") {
                return true;
            }
        }
    }

    return false;
}

void SuspendFunctionAnalyzer::find_suspend_points() {
    suspend_points_.clear();
    unsigned state_id = 1;

    // Walk CFG in forward order to assign state IDs.
    for (const CFGBlock* block : *cfg_) {
        if (!block) continue;

        for (const CFGElement& elem : *block) {
            if (auto stmt_elem = elem.getAs<CFGStmt>()) {
                const Stmt* stmt = stmt_elem->getStmt();
                if (is_suspend_call(stmt)) {
                    SuspendPointInfo info;
                    info.suspend_stmt = stmt;
                    info.state_id = state_id++;
                    // live_variables will be populated by compute_liveness()
                    suspend_points_.push_back(info);
                }
            }
        }
    }
}

/// AST visitor to collect variable uses (reads).
class UseCollector : public RecursiveASTVisitor<UseCollector> {
public:
    std::set<const VarDecl*>& uses;
    explicit UseCollector(std::set<const VarDecl*>& u) : uses(u) {}

    bool VisitDeclRefExpr(DeclRefExpr* dre) {
        if (const VarDecl* vd = dyn_cast<VarDecl>(dre->getDecl())) {
            // Check if this is a read (not a write).
            // For simplicity, treat all DeclRefExpr as potential reads.
            // The def collector will handle writes.
            uses.insert(vd);
        }
        return true;
    }
};

void SuspendFunctionAnalyzer::collect_uses(const Stmt* stmt, std::set<const VarDecl*>& uses) {
    if (!stmt) return;
    UseCollector collector(uses);
    collector.TraverseStmt(const_cast<Stmt*>(stmt));
}

void SuspendFunctionAnalyzer::collect_defs(const Stmt* stmt, std::set<const VarDecl*>& defs) {
    if (!stmt) return;

    // Check for variable declarations with initializers.
    if (const auto* ds = dyn_cast<DeclStmt>(stmt)) {
        for (const Decl* d : ds->decls()) {
            if (const auto* vd = dyn_cast<VarDecl>(d)) {
                defs.insert(vd);
            }
        }
        return;
    }

    // Check for assignment operators.
    if (const auto* bo = dyn_cast<BinaryOperator>(stmt)) {
        if (bo->isAssignmentOp()) {
            if (const auto* dre = dyn_cast<DeclRefExpr>(bo->getLHS()->IgnoreParenImpCasts())) {
                if (const auto* vd = dyn_cast<VarDecl>(dre->getDecl())) {
                    defs.insert(vd);
                }
            }
        }
    }

    // Check for unary increment/decrement.
    if (const auto* uo = dyn_cast<UnaryOperator>(stmt)) {
        if (uo->isIncrementDecrementOp()) {
            if (const auto* dre = dyn_cast<DeclRefExpr>(uo->getSubExpr()->IgnoreParenImpCasts())) {
                if (const auto* vd = dyn_cast<VarDecl>(dre->getDecl())) {
                    defs.insert(vd);
                }
            }
        }
    }
}

const CFGBlock* SuspendFunctionAnalyzer::find_block_containing(const Stmt* stmt) const {
    auto it = stmt_to_block_.find(stmt);
    if (it != stmt_to_block_.end()) {
        return it->second;
    }
    return nullptr;
}

void SuspendFunctionAnalyzer::compute_liveness() {
    // Classic backward dataflow liveness analysis.
    //
    // For each basic block B:
    //   LIVE_out[B] = union of LIVE_in[S] for all successors S
    //   LIVE_in[B] = (LIVE_out[B] - KILL[B]) union GEN[B]
    //
    // Where:
    //   GEN[B] = variables used (read) in B before any definition
    //   KILL[B] = variables defined (written) in B
    //
    // Iterate until fixed point.

    if (!cfg_) return;

    // Initialize all blocks to empty sets.
    for (const CFGBlock* block : *cfg_) {
        if (!block) continue;
        live_in_[block->getBlockID()] = {};
        live_out_[block->getBlockID()] = {};
    }

    // Compute GEN and KILL sets for each block.
    std::map<unsigned, std::set<const VarDecl*>> gen;
    std::map<unsigned, std::set<const VarDecl*>> kill;

    for (const CFGBlock* block : *cfg_) {
        if (!block) continue;
        unsigned id = block->getBlockID();
        gen[id] = {};
        kill[id] = {};

        // Process statements in FORWARD order to compute gen/kill correctly.
        // A use before a def contributes to GEN.
        // A def adds to KILL.
        for (const CFGElement& elem : *block) {
            if (auto stmt_elem = elem.getAs<CFGStmt>()) {
                const Stmt* stmt = stmt_elem->getStmt();

                // Collect uses that are not already killed.
                std::set<const VarDecl*> uses;
                collect_uses(stmt, uses);
                for (const VarDecl* vd : uses) {
                    if (kill[id].find(vd) == kill[id].end()) {
                        gen[id].insert(vd);
                    }
                }

                // Collect definitions.
                std::set<const VarDecl*> defs;
                collect_defs(stmt, defs);
                for (const VarDecl* vd : defs) {
                    kill[id].insert(vd);
                }
            }
        }
    }

    // Iterate until fixed point.
    bool changed = true;
    int max_iterations = 100;  // Safety limit.

    while (changed && max_iterations-- > 0) {
        changed = false;

        // Process blocks in reverse post-order for faster convergence.
        // For simplicity, we just iterate all blocks.
        for (const CFGBlock* block : *cfg_) {
            if (!block) continue;
            unsigned id = block->getBlockID();

            // LIVE_out = union of LIVE_in of all successors.
            std::set<const VarDecl*> new_live_out;
            for (auto succ_it = block->succ_begin(); succ_it != block->succ_end(); ++succ_it) {
                const CFGBlock* succ = *succ_it;
                if (succ) {
                    for (const VarDecl* vd : live_in_[succ->getBlockID()]) {
                        new_live_out.insert(vd);
                    }
                }
            }

            // LIVE_in = (LIVE_out - KILL) union GEN
            std::set<const VarDecl*> new_live_in;

            // Start with LIVE_out.
            new_live_in = new_live_out;

            // Remove KILL.
            for (const VarDecl* vd : kill[id]) {
                new_live_in.erase(vd);
            }

            // Add GEN.
            for (const VarDecl* vd : gen[id]) {
                new_live_in.insert(vd);
            }

            // Check for changes.
            if (live_out_[id] != new_live_out || live_in_[id] != new_live_in) {
                changed = true;
                live_out_[id] = new_live_out;
                live_in_[id] = new_live_in;
            }
        }
    }

    // Now compute live variables at each suspension point.
    // A variable is live at a suspend point if it is in LIVE_out at that statement.
    // Since we have block-level liveness, we need to compute statement-level.

    for (SuspendPointInfo& sp : suspend_points_) {
        const CFGBlock* block = find_block_containing(sp.suspend_stmt);
        if (!block) continue;

        // Compute liveness at this specific statement by walking backward
        // from block exit to the statement.
        std::set<const VarDecl*> live = live_out_[block->getBlockID()];

        // Walk statements in reverse order.
        bool found_stmt = false;
        for (auto it = block->rbegin(); it != block->rend(); ++it) {
            if (auto stmt_elem = it->getAs<CFGStmt>()) {
                const Stmt* stmt = stmt_elem->getStmt();

                if (stmt == sp.suspend_stmt) {
                    // Record liveness AFTER this statement (for spilling).
                    // Variables live after the suspend call need to be preserved.
                    sp.live_variables = live;
                    found_stmt = true;
                    break;
                }

                // Update liveness: LIVE = (LIVE - DEF) union USE
                std::set<const VarDecl*> defs;
                collect_defs(stmt, defs);
                for (const VarDecl* vd : defs) {
                    live.erase(vd);
                }

                std::set<const VarDecl*> uses;
                collect_uses(stmt, uses);
                for (const VarDecl* vd : uses) {
                    live.insert(vd);
                }
            }
        }

        // Add to global spilled variables set.
        for (const VarDecl* vd : sp.live_variables) {
            spilled_variables_.insert(vd);
        }
    }
}

} // namespace suspend
} // namespace kotlinx
