#include <vector>
#include "clang/AST/AST.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/ParsedAttrInfo.h"
#include "clang/Sema/Sema.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

#include "SuspendFunctionAnalyzer.hpp"

using namespace clang;
using namespace kotlinx::suspend;

namespace {

// Kotlin-aligned tokens:
//  - "suspend" marks a suspend function (attribute) and a suspend point (statement annotate or wrapper call).
static constexpr const char* SUSPEND_ANNOT = "suspend";

// -----------------------------------------------------------------------------
// Attribute registration
// -----------------------------------------------------------------------------
// Provides [[suspend]] / [[kotlinx::suspend]] for FunctionDecls. We lower this to an implicit
// AnnotateAttr with annotation "suspend" for uniform detection logic.
class KotlinxSuspendAttrInfo : public ParsedAttrInfo {
public:
    KotlinxSuspendAttrInfo() {
        static constexpr Spelling S[] = {
            {ParsedAttr::AS_CXX11, "suspend"},
            {ParsedAttr::AS_CXX11, "kotlinx::suspend"}
        };
        Spellings = S;
    }

    bool diagAppertainsToDecl(Sema& S, const ParsedAttr& Attr, const Decl* D) const override {
        (void)S;
        (void)Attr;
        return isa<FunctionDecl>(D);
    }

    AttrHandling handleDeclAttribute(Sema& S, Decl* D, const ParsedAttr& Attr) const override {
        // Adapt to newer Clang API for CreateImplicit
        AttributeCommonInfo Info(Attr.getRange(), AttributeCommonInfo::UnknownAttribute, AttributeCommonInfo::Form::CXX11());
        auto* ann = AnnotateAttr::CreateImplicit(S.Context, SUSPEND_ANNOT, Info);
        D->addAttr(ann);
        return AttributeApplied;
    }
};

static ParsedAttrInfoRegistry::Add<KotlinxSuspendAttrInfo>
    SuspendReg("suspend", "Mark a function as Kotlin-style suspend");

// -----------------------------------------------------------------------------
// Suspend function visitor
// -----------------------------------------------------------------------------
class KotlinxSuspendVisitor : public RecursiveASTVisitor<KotlinxSuspendVisitor> {
public:
    explicit KotlinxSuspendVisitor(ASTContext& ctx, DiagnosticsEngine& diags)
        : ctx_(ctx), diags_(diags) {}

    bool VisitFunctionDecl(FunctionDecl* fd) {
        if (!fd || !fd->hasBody())
            return true;

        if (hasAnnotate(fd, SUSPEND_ANNOT)) {
            auto id = diags_.getCustomDiagID(DiagnosticsEngine::Remark,
                                            "kotlinx-suspend: found suspend function '%0'");
            diags_.Report(fd->getLocation(), id) << fd->getNameAsString();
            suspendFns_.push_back(fd);
            currentSuspend_ = fd;
        }
        return true;
    }

    bool VisitAttributedStmt(AttributedStmt* stmt) {
        if (!stmt || !currentSuspend_)
            return true;

        for (const Attr* a : stmt->getAttrs()) {
            if (const auto* ann = dyn_cast<AnnotateAttr>(a)) {
                if (ann->getAnnotation() == SUSPEND_ANNOT) {
                    auto id = diags_.getCustomDiagID(DiagnosticsEngine::Remark,
                                                    "kotlinx-suspend: suspend point in '%0'");
                    diags_.Report(stmt->getBeginLoc(), id)
                        << currentSuspend_->getNameAsString();
                }
            }
        }
        return true;
    }

    bool TraverseFunctionDecl(FunctionDecl* fd) {
        FunctionDecl* prev = currentSuspend_;
        if (fd && hasAnnotate(fd, SUSPEND_ANNOT))
            currentSuspend_ = fd;
        bool res = RecursiveASTVisitor::TraverseFunctionDecl(fd);
        currentSuspend_ = prev;
        return res;
    }

    const std::vector<FunctionDecl*>& suspendFunctions() const { return suspendFns_; }

private:
    static bool hasAnnotate(const Decl* d, StringRef annotation) {
        if (!d) return false;
        for (const Attr* a : d->attrs()) {
            if (const auto* ann = dyn_cast<AnnotateAttr>(a)) {
                if (ann->getAnnotation() == annotation)
                    return true;
            }
        }
        return false;
    }

    ASTContext& ctx_;
    DiagnosticsEngine& diags_;
    FunctionDecl* currentSuspend_ = nullptr;
    std::vector<FunctionDecl*> suspendFns_;
};

// -----------------------------------------------------------------------------
// AST Consumer with dual-mode code generation
// -----------------------------------------------------------------------------
class KotlinxSuspendConsumer : public ASTConsumer {
public:
    KotlinxSuspendConsumer(ASTContext& ctx, DiagnosticsEngine& diags,
                           std::string outDir, DispatchMode dispatchMode, SpillMode spillMode)
        : ctx_(ctx), visitor_(ctx, diags), outDir_(std::move(outDir)),
          dispatchMode_(dispatchMode), spillMode_(spillMode) {}

    void HandleTranslationUnit(ASTContext& ctx) override {
        llvm::errs() << "DEBUG: HandleTranslationUnit (dispatch="
                     << (dispatchMode_ == DispatchMode::ComputedGoto ? "goto" : "switch")
                     << ", spill="
                     << (spillMode_ == SpillMode::Liveness ? "liveness" : "all")
                     << ")\n";
        visitor_.TraverseDecl(ctx.getTranslationUnitDecl());
        emitSidecar(ctx);
    }

private:
    static std::string getStmtText(const Stmt* st, const SourceManager& sm, const LangOptions& lo) {
        if (!st) return {};
        CharSourceRange range = CharSourceRange::getTokenRange(st->getSourceRange());
        return Lexer::getSourceText(range, sm, lo).str();
    }

    static bool isSuspendCallStmt(const Stmt* st) {
        return SuspendFunctionAnalyzer::is_suspend_call(st);
    }

    void emitSidecar(ASTContext& ctx) {
        llvm::errs() << "DEBUG: emitSidecar. SuspendFns: " << visitor_.suspendFunctions().size() << "\n";
        const auto& fns = visitor_.suspendFunctions();
        if (fns.empty()) return;

        const SourceManager& sm = ctx.getSourceManager();
        const LangOptions& lo = ctx.getLangOpts();
        PrintingPolicy pp(lo);

        auto fileEntry = sm.getFileEntryForID(sm.getMainFileID());
        if (!fileEntry) {
             llvm::errs() << "DEBUG: No file entry for main file ID\n";
             return;
        }
        std::string tuName = fileEntry->tryGetRealPathName().str();
        llvm::errs() << "DEBUG: tuName: " << tuName << "\n";

        llvm::SmallString<256> outPath(outDir_);
        llvm::sys::path::append(outPath, llvm::sys::path::filename(tuName));
        llvm::sys::path::replace_extension(outPath, ".kx.cpp");

        llvm::sys::fs::create_directories(outDir_);
        std::error_code ec;
        llvm::raw_fd_ostream os(outPath, ec, llvm::sys::fs::OF_Text);
        if (ec) {
            return;
        }

        // Header with mode information.
        os << "// Generated by KotlinxSuspendPlugin\n";
        os << "// Dispatch: " << (dispatchMode_ == DispatchMode::ComputedGoto ? "computed-goto" : "switch") << "\n";
        os << "// Spill: " << (spillMode_ == SpillMode::Liveness ? "liveness-analysis" : "all-parameters") << "\n";
        os << "// Source: " << tuName << "\n\n";
        os << "#include <kotlinx/coroutines/ContinuationImpl.hpp>\n";
        os << "#include <kotlinx/coroutines/Result.hpp>\n";
        os << "#include <kotlinx/coroutines/intrinsics/Intrinsics.hpp>\n";
        os << "#include <memory>\n\n";
        os << "using namespace kotlinx::coroutines;\n";
        os << "using namespace kotlinx::coroutines::intrinsics;\n\n";
        os << "extern \"C\" void __kxs_suspend_point(int id);\n\n";

        for (FunctionDecl* fd : fns) {
            if (!fd || !fd->hasBody()) continue;

            if (dispatchMode_ == DispatchMode::ComputedGoto) {
                emitComputedGotoCoroutine(os, ctx, fd, pp, sm, lo);
            } else {
                emitSwitchCoroutine(os, ctx, fd, pp, sm, lo);
            }
        }

        auto id = ctx.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Remark,
                                                     "kotlinx-suspend: wrote %0");
        ctx.getDiagnostics().Report(fns.front()->getLocation(), id) << outPath.str();
    }

    // -------------------------------------------------------------------------
    // Phase 1: Switch-based dispatch (original implementation)
    // -------------------------------------------------------------------------
    void emitSwitchCoroutine(llvm::raw_ostream& os, ASTContext& ctx, FunctionDecl* fd,
                              const PrintingPolicy& pp, const SourceManager& sm,
                              const LangOptions& lo) {
        std::string fnName = fd->getNameAsString();
        static int uniqueCounter = 0;
        std::string coroName = "__kxs_coroutine_" + fnName + "_" + std::to_string(++uniqueCounter);

        // Determine which variables to spill.
        std::set<const VarDecl*> spillVars;
        std::vector<SuspendPointInfo> suspendPoints;

        if (spillMode_ == SpillMode::Liveness) {
            SuspendFunctionAnalyzer analyzer(ctx, fd);
            if (analyzer.analyze()) {
                spillVars = analyzer.get_all_spilled_variables();
                suspendPoints = analyzer.get_suspend_points();
            }
        }

        // Build parameter list.
        std::string params;
        std::string ctorParams;
        std::string ctorInits;
        std::string callArgs;
        bool firstParam = true;
        bool firstCtor = true;

        for (ParmVarDecl* p : fd->parameters()) {
            std::string ty = p->getType().getAsString(pp);
            std::string nm = p->getNameAsString();
            if (nm.empty()) nm = "arg" + std::to_string(p->getFunctionScopeIndex());

            if (!firstParam) params += ", ";
            params += ty + " " + nm;
            firstParam = false;

            if (nm != "completion") {
                if (!firstCtor) ctorParams += ", ";
                ctorParams += ty + " " + nm;
                firstCtor = false;

                ctorInits += (ctorInits.empty() ? "" : ", ") + nm + "_(" + nm + ")";
                callArgs += (callArgs.empty() ? "" : ", ") + nm;
            }
        }

        std::string retTy = fd->getReturnType().getAsString(pp);

        // Emit coroutine class.
        os << "struct " << coroName << " : public ContinuationImpl {\n";
        os << "    int _label = 0;\n";

        // Emit fields based on spill mode.
        if (spillMode_ == SpillMode::Liveness && !spillVars.empty()) {
            // Only emit spill fields for live variables.
            for (const VarDecl* vd : spillVars) {
                os << "    " << vd->getType().getAsString(pp) << " "
                   << vd->getNameAsString() << "_spill;\n";
            }
        } else {
            // Phase 1: emit all parameters.
            for (ParmVarDecl* p : fd->parameters()) {
                std::string nm = p->getNameAsString();
                if (nm.empty()) nm = "arg" + std::to_string(p->getFunctionScopeIndex());
                if (nm == "completion") continue;
                os << "    " << p->getType().getAsString(pp) << " " << nm << "_;\n";
            }
        }
        os << "\n";

        // Constructor.
        os << "    explicit " << coroName << "(std::shared_ptr<Continuation<void*>> completion";
        if (!ctorParams.empty()) os << ", " << ctorParams;
        os << ")\n";
        os << "        : ContinuationImpl(completion)";
        if (spillMode_ != SpillMode::Liveness && !ctorInits.empty()) {
            os << ", " << ctorInits;
        }
        os << " {}\n\n";

        // invoke_suspend with switch dispatch.
        os << "    void* invoke_suspend(Result<void*> result) override {\n";
        os << "        switch (_label) {\n";
        os << "        case 0:\n";
        os << "            (void)result.get_or_throw();\n";

        const auto* body = dyn_cast<CompoundStmt>(fd->getBody());
        int stateId = 1;
        if (body) {
            for (const Stmt* st : body->body()) {
                if (isSuspendCallStmt(st)) {
                    std::string callText = getStmtText(st, sm, lo);
                    if (!callText.empty() && callText.back() == ';')
                        callText.pop_back();

                    // Emit spill code if using liveness analysis.
                    if (spillMode_ == SpillMode::Liveness) {
                        for (const auto& sp : suspendPoints) {
                            if (sp.state_id == (unsigned)stateId) {
                                for (const VarDecl* vd : sp.live_variables) {
                                    os << "            " << vd->getNameAsString()
                                       << "_spill = " << vd->getNameAsString() << ";\n";
                                }
                                break;
                            }
                        }
                    }

                    os << "            _label = " << stateId << ";\n";
                    os << "            __kxs_suspend_point(" << stateId << ");\n";
                    os << "            {\n";
                    os << "                void* _tmp = " << callText << ";\n";
                    os << "                if (is_coroutine_suspended(_tmp)) return COROUTINE_SUSPENDED;\n";
                    os << "            }\n";
                    os << "            goto __kxs_cont" << stateId << ";\n";
                    os << "        case " << stateId << ":\n";

                    // Emit restore code if using liveness analysis.
                    if (spillMode_ == SpillMode::Liveness) {
                        for (const auto& sp : suspendPoints) {
                            if (sp.state_id == (unsigned)stateId) {
                                for (const VarDecl* vd : sp.live_variables) {
                                    os << "            " << vd->getNameAsString()
                                       << " = " << vd->getNameAsString() << "_spill;\n";
                                }
                                break;
                            }
                        }
                    }

                    os << "            (void)result.get_or_throw();\n";
                    os << "        __kxs_cont" << stateId << ":\n";
                    stateId++;
                } else {
                    os << "            " << getStmtText(st, sm, lo) << "\n";
                }
            }
        }

        os << "            break;\n";
        os << "        }\n";
        os << "        return nullptr;\n";
        os << "    }\n";
        os << "};\n\n";

        // Wrapper function.
        os << retTy << " " << fnName << "(" << params << ") {\n";
        os << "    auto __coro = std::make_shared<" << coroName << ">(completion";
        if (!callArgs.empty()) os << ", " << callArgs;
        os << ");\n";
        os << "    return __coro->invoke_suspend(Result<void*>::success(nullptr));\n";
        os << "}\n\n";
    }

    // -------------------------------------------------------------------------
    // Phase 3: Computed-goto dispatch (Kotlin/Native indirectbr parity)
    // -------------------------------------------------------------------------
    void emitComputedGotoCoroutine(llvm::raw_ostream& os, ASTContext& ctx, FunctionDecl* fd,
                                    const PrintingPolicy& pp, const SourceManager& sm,
                                    const LangOptions& lo) {
        std::string fnName = fd->getNameAsString();
        static int uniqueCounter = 0;
        std::string coroName = "__kxs_coroutine_" + fnName + "_" + std::to_string(++uniqueCounter);

        // Run liveness analysis (always for computed-goto mode).
        SuspendFunctionAnalyzer analyzer(ctx, fd);
        std::set<const VarDecl*> spillVars;
        std::vector<SuspendPointInfo> suspendPoints;

        if (analyzer.analyze()) {
            spillVars = analyzer.get_all_spilled_variables();
            suspendPoints = analyzer.get_suspend_points();
        }

        // Build parameter list.
        std::string params;
        std::string ctorParams;
        std::string callArgs;
        bool firstParam = true;
        bool firstCtor = true;

        for (ParmVarDecl* p : fd->parameters()) {
            std::string ty = p->getType().getAsString(pp);
            std::string nm = p->getNameAsString();
            if (nm.empty()) nm = "arg" + std::to_string(p->getFunctionScopeIndex());

            if (!firstParam) params += ", ";
            params += ty + " " + nm;
            firstParam = false;

            if (nm != "completion") {
                if (!firstCtor) ctorParams += ", ";
                ctorParams += ty + " " + nm;
                firstCtor = false;
                callArgs += (callArgs.empty() ? "" : ", ") + nm;
            }
        }

        std::string retTy = fd->getReturnType().getAsString(pp);

        // Emit coroutine class with void* label.
        os << "struct " << coroName << " : public ContinuationImpl {\n";
        os << "    void* _label = nullptr;  // Block address for computed goto\n";

        // Emit spill fields.
        for (const VarDecl* vd : spillVars) {
            os << "    " << vd->getType().getAsString(pp) << " "
               << vd->getNameAsString() << "_spill;\n";
        }
        os << "\n";

        // Constructor.
        os << "    explicit " << coroName << "(std::shared_ptr<Continuation<void*>> completion";
        if (!ctorParams.empty()) os << ", " << ctorParams;
        os << ")\n";
        os << "        : ContinuationImpl(completion) {}\n\n";

        // invoke_suspend with computed goto dispatch.
        os << "    void* invoke_suspend(Result<void*> result) override {\n";
        os << "\n";

        // Entry dispatch - matches Kotlin's IrSuspendableExpression pattern.
        os << "        // Entry dispatch (Kotlin/Native indirectbr pattern)\n";
        os << "        if (_label == nullptr) goto __kxs_start;\n";
        os << "        goto *_label;  // Computed goto -> LLVM indirectbr\n\n";

        os << "    __kxs_start:\n";
        os << "        (void)result.get_or_throw();\n";

        const auto* body = dyn_cast<CompoundStmt>(fd->getBody());
        int resumeId = 0;
        if (body) {
            for (const Stmt* st : body->body()) {
                if (isSuspendCallStmt(st)) {
                    std::string callText = getStmtText(st, sm, lo);
                    if (!callText.empty() && callText.back() == ';')
                        callText.pop_back();

                    // Find the corresponding suspend point for spill info.
                    const SuspendPointInfo* spInfo = nullptr;
                    for (const auto& sp : suspendPoints) {
                        if (sp.state_id == (unsigned)(resumeId + 1)) {
                            spInfo = &sp;
                            break;
                        }
                    }

                    // Emit spill code.
                    if (spInfo) {
                        for (const VarDecl* vd : spInfo->live_variables) {
                            os << "        " << vd->getNameAsString()
                               << "_spill = " << vd->getNameAsString() << ";\n";
                        }
                    }

                    // Store block address (becomes blockaddress in LLVM IR).
                    os << "        _label = &&__kxs_resume" << resumeId << ";\n";
                    os << "        __kxs_suspend_point(" << resumeId << ");\n";
                    os << "        {\n";
                    os << "            void* _tmp = " << callText << ";\n";
                    os << "            if (is_coroutine_suspended(_tmp)) return COROUTINE_SUSPENDED;\n";
                    os << "        }\n";
                    os << "        goto __kxs_cont" << resumeId << ";\n";

                    // Resume label.
                    os << "    __kxs_resume" << resumeId << ":\n";

                    // Emit restore code.
                    if (spInfo) {
                        for (const VarDecl* vd : spInfo->live_variables) {
                            os << "        " << vd->getNameAsString()
                               << " = " << vd->getNameAsString() << "_spill;\n";
                        }
                    }
                    os << "        (void)result.get_or_throw();\n";
                    os << "    __kxs_cont" << resumeId << ":\n";

                    resumeId++;
                } else {
                    os << "        " << getStmtText(st, sm, lo) << "\n";
                }
            }
        }

        os << "        return nullptr;\n";
        os << "    }\n";
        os << "};\n\n";

        // Wrapper function.
        os << retTy << " " << fnName << "(" << params << ") {\n";
        os << "    auto __coro = std::make_shared<" << coroName << ">(completion";
        if (!callArgs.empty()) os << ", " << callArgs;
        os << ");\n";
        os << "    return __coro->invoke_suspend(Result<void*>::success(nullptr));\n";
        os << "}\n\n";
    }

    ASTContext& ctx_;
    KotlinxSuspendVisitor visitor_;
    std::string outDir_;
    DispatchMode dispatchMode_;
    SpillMode spillMode_;
};

// -----------------------------------------------------------------------------
// Plugin action with argument parsing
// -----------------------------------------------------------------------------
class KotlinxSuspendAction : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& ci, llvm::StringRef) override {
        return std::make_unique<KotlinxSuspendConsumer>(
            ci.getASTContext(), ci.getDiagnostics(),
            outDir_, dispatchMode_, spillMode_);
    }

    bool ParseArgs(const CompilerInstance&, const std::vector<std::string>& args) override {
        for (const std::string& a : args) {
            if (a.rfind("out-dir=", 0) == 0) {
                outDir_ = a.substr(std::string("out-dir=").size());
            }
            else if (a == "dispatch=switch") {
                dispatchMode_ = DispatchMode::Switch;
            }
            else if (a == "dispatch=goto") {
                dispatchMode_ = DispatchMode::ComputedGoto;
            }
            else if (a == "spill=all") {
                spillMode_ = SpillMode::All;
            }
            else if (a == "spill=liveness") {
                spillMode_ = SpillMode::Liveness;
            }
        }
        return true;
    }

    ActionType getActionType() override {
        return AddBeforeMainAction;
    }

private:
    std::string outDir_ = "kxs_generated";
    DispatchMode dispatchMode_ = DispatchMode::Switch;  // Default: Phase 1 behavior
    SpillMode spillMode_ = SpillMode::All;              // Default: Phase 1 behavior
};

} // namespace

static FrontendPluginRegistry::Add<KotlinxSuspendAction>
    X("kotlinx-suspend", "Kotlin-style suspend function transformer with computed-goto support");
