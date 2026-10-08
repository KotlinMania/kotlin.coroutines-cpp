#include "SuspendFunctionAnalyzer.hpp"
#include <algorithm>

#include "clang/AST/Attr.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/SourceManager.h"

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
    // NOTE(port): Local class declarations and their methods have separate
    // analysis contexts, matching Kotlin's independent class-member lowering.
    bool TraverseDecl(Decl* declaration) {
        if (declaration && (isa<RecordDecl>(declaration) || isa<FunctionDecl>(declaration))) return true;
        return RecursiveASTVisitor<LocalVariableCollector>::TraverseDecl(declaration);
    }
    // NOTE(port): Clang retains closures in the enclosing AST. Only capture
    // initializers execute in this function; lambda locals belong to its call operator.
    bool TraverseLambdaExpr(LambdaExpr* expression) {
        for (auto* initializer : expression->capture_inits())
            if (!TraverseStmt(initializer)) return false;
        return true;
    }

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
        if (param != continuation_parameter(fd_)) {
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

// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractSuspendFunctionsLowering.kt:159-185
// NOTE(port): Resolve the trailing continuation through Clang's canonical types,
// rather than requiring Kotlin's synthesized parameter spelling in C++ source.
const ParmVarDecl* SuspendFunctionAnalyzer::continuation_parameter(const FunctionDecl* function) {
    if (!function || function->getNumParams() == 0) return nullptr;
    const auto* parameter = function->getParamDecl(function->getNumParams() - 1);
    const auto* handle = dyn_cast_or_null<ClassTemplateSpecializationDecl>(parameter->getType().getCanonicalType()->getAsCXXRecordDecl());
    if (!handle || handle->getSpecializedTemplate()->getName() != "shared_ptr" ||
        !handle->getSpecializedTemplate()->getQualifiedNameAsString().starts_with("std::")) return nullptr;
    const auto& arguments = handle->getTemplateArgs();
    if (arguments.size() != 1 || arguments[0].getKind() != TemplateArgument::Type) return nullptr;
    const auto* continuation = dyn_cast_or_null<ClassTemplateSpecializationDecl>(arguments[0].getAsType().getCanonicalType()->getAsCXXRecordDecl());
    if (!continuation || continuation->getSpecializedTemplate()->getQualifiedNameAsString() != "kotlin::coroutines::Continuation") return nullptr;
    const auto& result = continuation->getTemplateArgs();
    if (result.size() != 1 || result[0].getKind() != TemplateArgument::Type) return nullptr;
    auto type = result[0].getAsType().getCanonicalType();
    if (!type->isPointerType() || !type->getPointeeType()->isVoidType()) return nullptr;
    return parameter;
}

namespace {
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/DefaultArgumentStubGenerator.kt:61-130
// NOTE(port): Clang provides the selected default expressions on the call AST.
class DefaultArgumentReferences : public PrinterHelper {
public:
    bool handledStmt(Stmt* statement, llvm::raw_ostream& output) override {
        const auto* reference = dyn_cast<DeclRefExpr>(statement);
        if (!reference || reference->hasExplicitTemplateArgs()) return false;
        if (const auto* variable = dyn_cast<VarDecl>(reference->getDecl()); variable && variable->isLocalVarDeclOrParm()) return false;
        output << "::" << reference->getDecl()->getQualifiedNameAsString();
        return true;
    }
};
}

// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/DefaultArgumentStubGenerator.kt:91-130
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/coroutines/AddContinuationToFunctionCallsLowering.kt:90-99
// NOTE(port): Preserve omitted C++ default values when selecting the ABI overload;
// the continuation is appended after every original argument.
std::string SuspendFunctionAnalyzer::continuation_arguments(const CallExpr* call, const std::string& continuation,
                                                           const PrintingPolicy& policy,
                                                           const std::vector<std::string>& defaults) {
    std::string suffix;
    bool preceding = false;
    unsigned index = 0;
    for (const auto* argument : call->arguments()) {
        if (const auto* omitted = dyn_cast<CXXDefaultArgExpr>(argument)) {
            if (preceding) suffix += ", ";
            suffix += defaults.empty() ? default_argument(omitted, policy) : defaults.at(index++);
        }
        preceding = true;
    }
    if (preceding) suffix += ", ";
    suffix += continuation;
    return suffix;
}

// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/DefaultArgumentStubGenerator.kt:105-117
// NOTE(port): Print the resolved Clang default expression in its caller context.
std::string SuspendFunctionAnalyzer::default_argument(const CXXDefaultArgExpr* argument, const PrintingPolicy& policy) {
    std::string text;
    llvm::raw_string_ostream output(text);
    DefaultArgumentReferences references;
    argument->getExpr()->printPretty(output, &references, policy);
    return text;
}

namespace {
std::vector<const FunctionDecl*> call_candidates(const CallExpr* call) {
    if (auto* direct = call->getDirectCallee()) return {direct};
    std::vector<const FunctionDecl*> functions;
    if (auto* lookup = dyn_cast<OverloadExpr>(call->getCallee()->IgnoreParenImpCasts())) {
        for (auto* declaration : lookup->decls()) {
            declaration = declaration->getUnderlyingDecl();
            if (auto* function = dyn_cast<FunctionDecl>(declaration)) functions.push_back(function);
            else if (auto* pattern = dyn_cast<FunctionTemplateDecl>(declaration))
                functions.push_back(pattern->getTemplatedDecl());
        }
    }
    return functions;
}
}

bool SuspendFunctionAnalyzer::is_suspend_wrapper(const CallExpr* call) {
    auto candidates = call_candidates(call);
    return !candidates.empty() && std::all_of(candidates.begin(), candidates.end(), [](const auto* candidate) {
        return candidate->getQualifiedNameAsString() == "kotlinx::coroutines::dsl::suspend";
    });
}

// NOTE(port): Clang evaluation contexts adapt the Kotlin IR call walk; these
// C++ query operands are type-checked but introduce no runtime call or spill.
bool SuspendFunctionAnalyzer::is_unevaluated_expression(const Stmt* statement) {
    if (const auto* query = dyn_cast_or_null<UnaryExprOrTypeTraitExpr>(statement))
        return query->isArgumentType() || !query->getArgumentExpr()->getType()->isVariablyModifiedType();
    if (isa_and_nonnull<CXXNoexceptExpr>(statement)) return true;
    if (const auto* query = dyn_cast_or_null<CXXTypeidExpr>(statement))
        return !query->isPotentiallyEvaluated();
    return false;
}

bool SuspendFunctionAnalyzer::requires_overload_resolution(const FunctionDecl* function) {
    class UnresolvedCalls : public RecursiveASTVisitor<UnresolvedCalls> {
    public:
        explicit UnresolvedCalls(const ASTContext& context) : context_(context) {}
        bool unresolved = false;
        bool TraverseStmt(Stmt* statement) {
            if (SuspendFunctionAnalyzer::is_unevaluated_expression(statement)) return true;
            return RecursiveASTVisitor<UnresolvedCalls>::TraverseStmt(statement);
        }
        bool TraverseDecltypeTypeLoc(DecltypeTypeLoc, bool = true) { return true; }
        bool TraverseIfStmt(IfStmt* branch) {
            if (!branch->isConstexpr()) return RecursiveASTVisitor<UnresolvedCalls>::TraverseIfStmt(branch);
            auto selected = branch->getNondiscardedCase(context_);
            if (!selected) {
                // NOTE(port): Instantiation chooses the C++ arm before its
                // calls and field types can participate in frame lowering.
                unresolved = true;
                return true;
            }
            return TraverseStmt(branch->getInit()) && TraverseStmt(branch->getConditionVariableDeclStmt()) &&
                TraverseStmt(*selected);
        }
        // NOTE(port): Local class member calls resolve in their own context.
        bool TraverseDecl(Decl* declaration) {
            if (declaration && (isa<RecordDecl>(declaration) || isa<FunctionDecl>(declaration))) return true;
            return RecursiveASTVisitor<UnresolvedCalls>::TraverseDecl(declaration);
        }
        // NOTE(port): Resolve calls in a closure's own function context.
        bool TraverseLambdaExpr(LambdaExpr* expression) {
            for (auto* initializer : expression->capture_inits())
                if (!TraverseStmt(initializer)) return false;
            return true;
        }
        bool VisitCallExpr(CallExpr* call) {
            if (call->getDirectCallee()) return true;
            if (auto* lookup = dyn_cast<UnresolvedLookupExpr>(call->getCallee()->IgnoreParenImpCasts()))
                unresolved |= lookup->requiresADL();
            bool suspend = false, ordinary = false;
            auto candidates = call_candidates(call);
            if (candidates.empty() && call->isTypeDependent()) unresolved = true;
            for (auto* candidate : candidates) {
                bool annotated = false;
                for (const auto* attribute : candidate->attrs())
                    if (const auto* annotation = dyn_cast<AnnotateAttr>(attribute))
                        if (annotation->getAnnotation() == kSuspendAnnot) annotated = true;
                suspend |= annotated;
                ordinary |= !annotated;
            }
            unresolved |= suspend && ordinary;
            return true;
        }
    private:
        const ASTContext& context_;
    } calls(function->getASTContext());
    calls.TraverseStmt(function->getBody());
    return calls.unresolved;
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
        if (is_suspend_wrapper(call)) return true;
        auto candidates = call_candidates(call);
        if (!candidates.empty() && std::all_of(candidates.begin(), candidates.end(), [](const auto* callee) {
            for (const auto* attribute : callee->attrs())
                if (const auto* annotation = dyn_cast<AnnotateAttr>(attribute))
                    if (annotation->getAnnotation() == kSuspendAnnot) return true;
            return false;
        })) return true;
    }

    return false;
}

void SuspendFunctionAnalyzer::find_suspend_points() {
    suspend_points_.clear();

    // Collect CFG suspension sites, then assign IDs in source traversal order.
    for (const CFGBlock* block : *cfg_) {
        if (!block) continue;

        for (const CFGElement& elem : *block) {
            if (auto stmt_elem = elem.getAs<CFGStmt>()) {
                const Stmt* stmt = stmt_elem->getStmt();
                if (is_suspend_call(stmt)) {
                    // A wrapper around an already marked callee represents one
                    // suspension site, rather than a second nested suspension.
                    if (const auto* expression = dyn_cast<Expr>(stmt)) {
                        const auto* call = dyn_cast<CallExpr>(expression->IgnoreParenImpCasts());
                        if (call && is_suspend_wrapper(call) &&
                            call->getNumArgs() == 1 && is_suspend_call(call->getArg(0)->IgnoreUnlessSpelledInSource()))
                            continue;
                    }
                    SuspendPointInfo info;
                    info.suspend_stmt = stmt;
                    info.state_id = 0;
                    // live_variables will be populated by compute_liveness()
                    suspend_points_.push_back(info);
                }
            }
        }
    }
    const auto& manager = ctx_.getSourceManager();
    std::stable_sort(suspend_points_.begin(), suspend_points_.end(), [&](const auto& left, const auto& right) {
        return manager.isBeforeInTranslationUnit(left.suspend_stmt->getBeginLoc(), right.suspend_stmt->getBeginLoc());
    });
    for (size_t i = 0; i < suspend_points_.size(); ++i)
        suspend_points_[i].state_id = static_cast<unsigned>(i + 1);

}

/// AST visitor to collect variable uses (reads).
class UseCollector : public RecursiveASTVisitor<UseCollector> {
public:
    std::set<const VarDecl*>& uses;
    explicit UseCollector(std::set<const VarDecl*>& u) : uses(u) {}
    // NOTE(port): A class declaration does not execute its member bodies.
    bool TraverseDecl(Decl* declaration) {
        if (declaration && (isa<RecordDecl>(declaration) || isa<FunctionDecl>(declaration))) return true;
        return RecursiveASTVisitor<UseCollector>::TraverseDecl(declaration);
    }
    // NOTE(port): Capture construction reads enclosing variables. The closure
    // body executes later and has its own local-variable liveness analysis.
    bool TraverseLambdaExpr(LambdaExpr* expression) {
        for (auto* initializer : expression->capture_inits())
            if (!TraverseStmt(initializer)) return false;
        return true;
    }

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

    while (changed) {
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
        for (auto it = block->rbegin(); it != block->rend(); ++it) {
            if (auto stmt_elem = it->getAs<CFGStmt>()) {
                const Stmt* stmt = stmt_elem->getStmt();

                if (stmt == sp.suspend_stmt) {
                    // Record liveness AFTER this statement (for spilling).
                    // Variables live after the suspend call need to be preserved.
                    sp.live_variables = live;
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
