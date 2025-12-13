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

using namespace clang;

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
    SuspendReg("suspend", "Mark a function as Kotlin‑style suspend");

// Phase‑1:
// - Detect suspend functions via [[suspend]] or [[kotlinx::suspend]]
// - Detect suspend points via [[clang::annotate("suspend")]] or suspend(...) wrapper call
// - Emit a generated .kx.cpp sidecar containing a Kotlin‑Native‑shape state machine
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
            currentSuspend_ = fd; // for suspend point remarks
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
        // Track current suspend function while traversing its body.
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

class KotlinxSuspendConsumer : public ASTConsumer {
public:
    KotlinxSuspendConsumer(ASTContext& ctx, DiagnosticsEngine& diags, std::string outDir)
        : visitor_(ctx, diags), outDir_(std::move(outDir)) {}

    void HandleTranslationUnit(ASTContext& ctx) override {
        llvm::errs() << "DEBUG: HandleTranslationUnit\n";
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
        if (!st) return false;

        // Attribute-based form: [[clang::annotate("suspend")]]
        if (const auto* attributed = dyn_cast<AttributedStmt>(st)) {
            for (const Attr* a : attributed->getAttrs()) {
                    if (const auto* ann = dyn_cast<AnnotateAttr>(a)) {
                    if (ann->getAnnotation() == SUSPEND_ANNOT)
                        return true;
                }
            }
        }

        // Call-wrapper form: suspend(...)
        if (const auto* exprStmt = dyn_cast<Expr>(st)) {
            if (const auto* call = dyn_cast<CallExpr>(exprStmt)) {
                if (const FunctionDecl* callee = call->getDirectCallee()) {
                    if (callee->getName() == "suspend") return true;
                }
            }
        }
        return false;
    }

    void emitSidecar(ASTContext& ctx) {
        llvm::errs() << "DEBUG: emitSidecar. SuspendFns: " << visitor_.suspendFunctions().size() << "\n";
        const auto& fns = visitor_.suspendFunctions();
        if (fns.empty()) return;

        const SourceManager& sm = ctx.getSourceManager();
        const LangOptions& lo = ctx.getLangOpts();
        PrintingPolicy pp(lo);

        // Compute output file path per translation unit.
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

        os << "// Generated by KotlinxSuspendPlugin (phase‑1)\n";
        os << "// Source: " << tuName << "\n\n";
        os << "#include <kotlinx/coroutines/ContinuationImpl.hpp>\n";
        os << "#include <kotlinx/coroutines/Result.hpp>\n";
        os << "#include <kotlinx/coroutines/intrinsics/Intrinsics.hpp>\n";
        os << "#include <memory>\n\n";
        os << "using namespace kotlinx::coroutines;\n";
        os << "using namespace kotlinx::coroutines::intrinsics;\n\n";

        for (FunctionDecl* fd : fns) {
            if (!fd || !fd->hasBody()) continue;

            std::string fnName = fd->getNameAsString();
        // Append unique index to handle overloads
        static int uniqueCounter = 0;
        std::string coroName = "__kxs_coroutine_" + fnName + "_" + std::to_string(++uniqueCounter);

            // Build parameter list and capture list.
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

                // Capture everything except a parameter named completion.
                if (nm != "completion") {
                    if (!firstCtor) ctorParams += ", ";
                    ctorParams += ty + " " + nm;
                    firstCtor = false;

                    ctorInits += (ctorInits.empty() ? "" : ", ") + nm + "_(" + nm + ")";
                    callArgs += (callArgs.empty() ? "" : ", ") + nm;
                }
            }

            std::string retTy = fd->getReturnType().getAsString(pp);

            os << "struct " << coroName << " : public ContinuationImpl {\n";
            os << "    int _label = 0;\n";
            for (ParmVarDecl* p : fd->parameters()) {
                std::string nm = p->getNameAsString();
                if (nm.empty()) nm = "arg" + std::to_string(p->getFunctionScopeIndex());
                if (nm == "completion") continue;
                os << "    " << p->getType().getAsString(pp) << " " << nm << "_;\n";
            }
            os << "\n";
            os << "    explicit " << coroName << "(std::shared_ptr<Continuation<void*>> completion";
            if (!ctorParams.empty()) os << ", " << ctorParams;
            os << ")\n";
            os << "        : ContinuationImpl(completion)";
            if (!ctorInits.empty()) os << ", " << ctorInits;
            os << " {}\n\n";

            os << "    void* invoke_suspend(Result<void*> result) override {\n";
            os << "        (void)result;\n";
            os << "        switch (_label) {\n";
            os << "        case 0:\n";

            const auto* body = dyn_cast<CompoundStmt>(fd->getBody());
            int stateId = 1;
            if (body) {
                for (const Stmt* st : body->body()) {
                    if (isSuspendCallStmt(st)) {
                        std::string callText = getStmtText(st, sm, lo);
                        // Strip trailing semicolon if present.
                        if (!callText.empty() && callText.back() == ';')
                            callText.pop_back();

                        os << "            _label = " << stateId << ";\n";
                        os << "            {\n";
                        os << "                void* _tmp = " << callText << ";\n";
                        os << "                if (is_coroutine_suspended(_tmp)) return COROUTINE_SUSPENDED;\n";
                        os << "            }\n";
                        os << "            [[fallthrough]];\n";
                        os << "        case " << stateId << ":\n";
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

            os << retTy << " " << fnName << "(" << params << ") {\n";
            os << "    auto __coro = std::make_shared<" << coroName << ">(completion";
            if (!callArgs.empty()) os << ", " << callArgs;
            os << ");\n";
            os << "    return __coro->invoke_suspend(Result<void*>::success(nullptr));\n";
            os << "}\n\n";
        }

        auto id = ctx.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Remark,
                                                     "kotlinx-suspend: wrote %0");
        ctx.getDiagnostics().Report(fns.front()->getLocation(), id) << outPath.str();
    }

    KotlinxSuspendVisitor visitor_;
    std::string outDir_;
};

class KotlinxSuspendAction : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& ci, llvm::StringRef) override {
        return std::make_unique<KotlinxSuspendConsumer>(ci.getASTContext(), ci.getDiagnostics(), outDir_);
    }

    bool ParseArgs(const CompilerInstance&, const std::vector<std::string>& args) override {
        for (const std::string& a : args) {
            if (a.rfind("out-dir=", 0) == 0) {
                outDir_ = a.substr(std::string("out-dir=").size());
            }
        }
        return true;
    }

    ActionType getActionType() override {
        return AddBeforeMainAction;
    }

private:
    std::string outDir_ = "kxs_generated";
};

} // namespace

static FrontendPluginRegistry::Add<KotlinxSuspendAction>
    X("kotlinx-suspend", "Detect kotlinx.coroutines-cpp suspend DSL markers");
