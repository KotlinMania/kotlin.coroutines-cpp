#include <algorithm>
#include <functional>
#include <set>
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
#include "llvm/Support/SaveAndRestore.h"

#include "SuspendFunctionAnalyzer.hpp"
#include "NativeSuspendLowering.hpp"
#include "CompilerFrameLowering.hpp"
#include "UpgradeCallableReferences.hpp"
#include "NativeFunctionReferenceLowering.hpp"
#include "FirSuspendCallChecker.hpp"
#include "TailSuspendCallsCollector.hpp"

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

// NOTE(port): These attributes encode Kotlin's class annotation and parameter
// kind in the Clang AST. An ordinary argument of the same class is not a receiver.
class KotlinxRestrictsSuspensionAttrInfo : public ParsedAttrInfo {
public:
    KotlinxRestrictsSuspensionAttrInfo() {
        static constexpr Spelling spellings[] = {{ParsedAttr::AS_CXX11, "kotlinx::restricts_suspension"}};
        Spellings = spellings;
    }
    bool diagAppertainsToDecl(Sema&, const ParsedAttr&, const Decl* declaration) const override {
        return isa<CXXRecordDecl>(declaration);
    }
    AttrHandling handleDeclAttribute(Sema& sema, Decl* declaration, const ParsedAttr& attribute) const override {
        AttributeCommonInfo info(attribute.getRange(), AttributeCommonInfo::UnknownAttribute, AttributeCommonInfo::Form::CXX11());
        declaration->addAttr(AnnotateAttr::CreateImplicit(sema.Context, "kotlin.coroutines.RestrictsSuspension", info));
        return AttributeApplied;
    }
};
static ParsedAttrInfoRegistry::Add<KotlinxRestrictsSuspensionAttrInfo>
    restricts_suspension_reg("restricts_suspension", "Mark a Kotlin restricted-suspension receiver class");

class KotlinxExtensionReceiverAttrInfo : public ParsedAttrInfo {
public:
    KotlinxExtensionReceiverAttrInfo() {
        static constexpr Spelling spellings[] = {{ParsedAttr::AS_CXX11, "kotlinx::extension_receiver"}};
        Spellings = spellings;
    }
    bool diagAppertainsToDecl(Sema&, const ParsedAttr&, const Decl* declaration) const override {
        return isa<ParmVarDecl>(declaration);
    }
    AttrHandling handleDeclAttribute(Sema& sema, Decl* declaration, const ParsedAttr& attribute) const override {
        AttributeCommonInfo info(attribute.getRange(), AttributeCommonInfo::UnknownAttribute, AttributeCommonInfo::Form::CXX11());
        declaration->addAttr(AnnotateAttr::CreateImplicit(sema.Context, "kotlin.ir.ExtensionReceiver", info));
        return AttributeApplied;
    }
};
static ParsedAttrInfoRegistry::Add<KotlinxExtensionReceiverAttrInfo>
    extension_receiver_reg("extension_receiver", "Mark the Kotlin IR extension-receiver parameter");

// -----------------------------------------------------------------------------
// Suspend function visitor
// -----------------------------------------------------------------------------
class KotlinxSuspendVisitor : public RecursiveASTVisitor<KotlinxSuspendVisitor> {
public:
    explicit KotlinxSuspendVisitor(DiagnosticsEngine& diags)
        : diags_(diags) {}

    // NOTE(port): Include constructor and lambda method declarations as well
    // as free functions in the checker's containing-declaration context.
    bool TraverseDecl(Decl* declaration) {
        llvm::SaveAndRestore<bool> evaluation(evaluated_,
            isa_and_nonnull<FunctionDecl>(declaration) ? true : evaluated_);
        // NOTE(port): Source diagnostics precede coroutine lowering. Imported
        // COROUTINE_IMPL declarations contain checked, lowered bodies, not
        // source-local classes inheriting suspension permission.
        if (const auto* record = dyn_cast_or_null<RecordDecl>(declaration)) {
            for (const auto* attribute : record->specific_attrs<AnnotateAttr>())
                if (attribute->getAnnotation() == "kotlin.ir.origin.COROUTINE_IMPL") return true;
        }
        const bool context = declaration && (isa<FunctionDecl, RecordDecl>(declaration) ||
            (isa<VarDecl>(declaration) && !isa<ParmVarDecl>(declaration)));
        if (context) containing_declarations_.push_back(declaration);
        bool result = RecursiveASTVisitor::TraverseDecl(declaration);
        if (context) containing_declarations_.pop_back();
        return result;
    }

    // NOTE(port): Clang's default spelled-lambda traversal skips the call
    // operator declaration. Visit it explicitly in its own function context.
    bool TraverseLambdaExpr(LambdaExpr* expression) {
        for (auto* initializer : expression->capture_inits())
            if (!TraverseStmt(initializer)) return false;
        auto* previous = currentSuspend_;
        currentSuspend_ = nullptr;
        bool result = TraverseDecl(expression->getCallOperator());
        currentSuspend_ = previous;
        return result;
    }

    // NOTE(port): Supply Clang's declaration stack to the translated checker.
    bool TraverseParmVarDecl(ParmVarDecl* parameter) {
        containing_declarations_.push_back(parameter);
        bool result = RecursiveASTVisitor::TraverseParmVarDecl(parameter);
        containing_declarations_.pop_back();
        return result;
    }

    bool VisitCallExpr(CallExpr* expression) {
        if (!evaluated_) return true;
        org::jetbrains::kotlin::fir::analysis::checkers::expression::FirSuspendCallChecker::check(
            expression, containing_declarations_, diags_);
        return true;
    }
    // NOTE(port): Unevaluated C++ operands remain checked by Clang. They do not
    // execute a suspend call; a nested function still has its own body context.
    bool TraverseStmt(Stmt* statement) {
        llvm::SaveAndRestore<bool> evaluation(evaluated_,
            evaluated_ && !SuspendFunctionAnalyzer::is_unevaluated_expression(statement));
        return RecursiveASTVisitor::TraverseStmt(statement);
    }
    bool TraverseDecltypeTypeLoc(DecltypeTypeLoc location, bool traverse_qualifier = true) {
        llvm::SaveAndRestore<bool> evaluation(evaluated_, false);
        return RecursiveASTVisitor::TraverseDecltypeTypeLoc(location, traverse_qualifier);
    }

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
    bool evaluated_ = true;
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

    DiagnosticsEngine& diags_;
    FunctionDecl* currentSuspend_ = nullptr;
    std::vector<const Decl*> containing_declarations_;
    std::vector<FunctionDecl*> suspendFns_;
};

// -----------------------------------------------------------------------------
// AST Consumer with LLVM injection authoring regions
// -----------------------------------------------------------------------------
class KotlinxSuspendConsumer : public ASTConsumer {
public:
    KotlinxSuspendConsumer(CompilerInstance& compiler,
                           std::string outDir, DispatchMode dispatchMode, SpillMode spillMode)
        : compiler_(compiler), visitor_(compiler.getDiagnostics()), outDir_(std::move(outDir)),
          dispatchMode_(dispatchMode), spillMode_(spillMode) {}

    bool HandleTopLevelDecl(DeclGroupRef declarations) override {
        if (!outDir_.empty()) return true;
        KotlinxSuspendVisitor definitions(compiler_.getDiagnostics());
        for (Decl* declaration : declarations) {
            org::jetbrains::kotlin::backend::common::lower::UpgradeCallableReferences().lower(declaration);
            org::jetbrains::kotlin::backend::konan::lower::NativeFunctionReferenceLowering().lower(declaration);
            definitions.TraverseDecl(declaration);
        }
        // Lower nested callables before an enclosing replacement imports them.
        const auto& functions = definitions.suspendFunctions();
        for (auto iterator = functions.rbegin(); iterator != functions.rend(); ++iterator) {
            auto* function = *iterator;
            if (!function->doesThisDeclarationHaveABody()) continue;
            if ((function->getDescribedFunctionTemplate() || function->isDependentContext()) &&
                SuspendFunctionAnalyzer::requires_overload_resolution(function)) continue;
            auto& context = compiler_.getASTContext();
            SuspendFunctionAnalyzer analyzer(context, function);
            if (!analyzer.analyze()) return false;
            if (analyzer.get_suspend_points().empty()) continue;
            if (is_direct_entry(context, function)) {
                auto [body, changed] = add_tail_continuation(context, function);
                if (changed && !install_native_frame(compiler_, function, tail_entry(context, function, body))) return false;
                continue;
            }
            if (!install_native_frame(compiler_, function, lower_native_suspend(context, function))) return false;
        }
        return true;
    }

    void HandleTranslationUnit(ASTContext& ctx) override {
        if (!outDir_.empty()) {
            visitor_.TraverseDecl(ctx.getTranslationUnitDecl());
            emitSidecar(ctx);
        }
    }

private:
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/coroutines/AddContinuationToFunctionCallsLowering.kt:74-99
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeAddContinuationToFunctionCallsLowering.kt:15-22
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:74-91
    // NOTE(port): Clang reparses the matching ABI overload in memory. A direct
    // entry supplies its trailing continuation, rather than constructing a frame.
    std::pair<std::string, bool> add_tail_continuation(ASTContext& context, FunctionDecl* function) {
        auto& manager = context.getSourceManager();
        auto* body = function->getBody();
        auto text = getStmtText(body, manager, context.getLangOpts());
        const auto* continuation = SuspendFunctionAnalyzer::continuation_parameter(function);
        struct Edit { unsigned begin; unsigned end; std::string text; };
        std::vector<Edit> edits;
        const unsigned begin = manager.getFileOffset(body->getBeginLoc());
        const auto tail = org::jetbrains::kotlin::backend::common::collect_tail_suspend_calls(function);
        std::set<const Expr*> returned_expressions;
        // NOTE(port): During incremental top-level parsing, Clang's global parent
        // map is incomplete. Track the actual body path while visiting children.
        std::function<void(const Stmt*, const Expr*, bool)> visit = [&](const Stmt* statement, const Expr* root, bool already_returned) {
            if (!statement || isa<LambdaExpr>(statement)) return;
            if (SuspendFunctionAnalyzer::is_unevaluated_expression(statement)) return;
            if (isa<ReturnStmt>(statement)) already_returned = true;
            if (const auto* expression = dyn_cast<Expr>(statement)) {
                if (!root) root = expression;
            } else root = nullptr;
            if (const auto* call = dyn_cast<CallExpr>(statement)) {
                // Unit tail statements become explicit returns of the suspend result.
                if (tail.call_sites.contains(call) && !already_returned && returned_expressions.insert(root).second) {
                    const auto position = manager.getFileOffset(root->getBeginLoc()) - begin;
                    edits.push_back({position, position, "return "});
                }
                // For a tail call, returnIfSuspended is replaced by its argument.
                if (SuspendFunctionAnalyzer::is_suspend_wrapper(call) && call->getNumArgs() == 1) {
                    const auto* argument = call->getArg(0)->IgnoreUnlessSpelledInSource();
                    const auto after_argument = Lexer::getLocForEndOfToken(argument->getEndLoc(), 0, manager, context.getLangOpts());
                    const auto after_call = Lexer::getLocForEndOfToken(call->getEndLoc(), 0, manager, context.getLangOpts());
                    edits.push_back({manager.getFileOffset(call->getBeginLoc()) - begin,
                        manager.getFileOffset(argument->getBeginLoc()) - begin, ""});
                    edits.push_back({manager.getFileOffset(after_argument) - begin,
                        manager.getFileOffset(after_call) - begin, ""});
                }
                if (const auto* callee = call->getDirectCallee()) {
                    for (const auto* attribute : callee->attrs()) {
                        const auto* annotation = dyn_cast<AnnotateAttr>(attribute);
                        if (!annotation || annotation->getAnnotation() != "kxs_implicit_continuation") continue;
                        if (!continuation) {
                            auto id = context.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Error,
                                "direct suspend entry needs a trailing shared Continuation<void*> parameter");
                            context.getDiagnostics().Report(function->getLocation(), id);
                            return;
                        }
                        const auto position = manager.getFileOffset(call->getRParenLoc()) - begin;
                        edits.push_back({position, position,
                            SuspendFunctionAnalyzer::continuation_arguments(call, continuation->getNameAsString(), PrintingPolicy(context.getLangOpts()))});
                        break;
                    }
                }
            }
            for (const auto* child : statement->children()) visit(child, root, already_returned);
        };
        visit(body, nullptr, false);
        std::sort(edits.begin(), edits.end(), [](const auto& left, const auto& right) {
            if (left.begin != right.begin) return left.begin > right.begin;
            return left.end > right.end;
        });
        for (const auto& edit : edits) text.replace(edit.begin, edit.end - edit.begin, edit.text);
        return {text, !edits.empty()};
    }

    // NOTE(port): Clang AST-import adapter for an unchanged direct entry ABI.
    std::string tail_entry(ASTContext& context, FunctionDecl* function, const std::string& body) {
        const auto* method = dyn_cast<CXXMethodDecl>(function);
        auto& manager = context.getSourceManager();
        if (method || function->getDescribedFunctionTemplate() || function->getTemplateSpecializationInfo() ||
            !manager.isWrittenInMainFile(function->getLocation())) return body;
        std::vector<const NamespaceDecl*> namespaces;
        for (auto* scope = function->getDeclContext(); !scope->isTranslationUnit(); scope = scope->getParent())
            namespaces.push_back(cast<NamespaceDecl>(scope));
        std::string text;
        for (auto scope = namespaces.rbegin(); scope != namespaces.rend(); ++scope)
            text += std::string((*scope)->isInline() ? "inline namespace " : "namespace ") + (*scope)->getNameAsString() + " {\n";
        text += "void* " + function->getNameAsString() + "(";
        PrintingPolicy policy(context.getLangOpts());
        for (unsigned i = 0; i < function->getNumParams(); ++i) {
            if (i) text += ", ";
            std::string declaration;
            llvm::raw_string_ostream output(declaration);
            function->getParamDecl(i)->getType().print(output, policy, function->getParamDecl(i)->getNameAsString());
            text += declaration;
        }
        text += ")";
        const auto* prototype = function->getType()->getAs<FunctionProtoType>();
        if (prototype && prototype->getNoexceptExpr())
            text += " noexcept(" + getStmtText(prototype->getNoexceptExpr(), manager, context.getLangOpts()) + ")";
        else if (prototype && prototype->isNothrow()) text += " noexcept";
        text += " " + body;
        for (size_t i = 0; i < namespaces.size(); ++i) text += "\n}";
        return text;
    }
    static std::string getStmtText(const Stmt* st, const SourceManager& sm, const LangOptions& lo) {
        if (!st) return {};
        CharSourceRange range = CharSourceRange::getTokenRange(st->getSourceRange());
        return Lexer::getSourceText(range, sm, lo).str();
    }

    void emitSidecar(ASTContext& ctx) {
        const auto& fns = visitor_.suspendFunctions();
        if (fns.empty()) return;

        const SourceManager& sm = ctx.getSourceManager();
        const LangOptions& lo = ctx.getLangOpts();
        PrintingPolicy pp(lo);

        auto fileEntry = sm.getFileEntryForID(sm.getMainFileID());
        if (!fileEntry) {
             return;
        }
        std::string tuName = fileEntry->tryGetRealPathName().str();

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
        os << "#include <kotlinx/coroutines/dsl/Suspend.hpp>\n";
        os << "#include <kotlinx/coroutines/intrinsics/Intrinsics.hpp>\n";
        os << "#include <memory>\n";
        os << "#include <cstdint>\n#include <optional>\n#include <functional>\n\n";
        os << "using namespace kotlinx::coroutines;\n";
        os << "using namespace kotlinx::coroutines::intrinsics;\n";
        os << "using namespace kotlinx::coroutines::dsl;\n\n";

        for (FunctionDecl* fd : fns) {
            if (!fd || !fd->hasBody()) continue;

            // Functions without suspension, and eligible tail-only returns, keep
            // their direct ABI entry. No frame or resume dispatch is needed.
            if (emit_direct_entry(os, ctx, fd, pp, sm, lo)) continue;
            if (is_direct_entry(ctx, fd)) continue;

            os << lower_native_suspend(ctx, fd);
        }

        auto id = ctx.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Remark,
                                                     "kotlinx-suspend: wrote %0");
        ctx.getDiagnostics().Report(fns.front()->getLocation(), id) << outPath.str();
    }

    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:55-69
    // NOTE(port): C++ retained storage is checked after the translated collector;
    // Kotlin's GC-local tail optimization does not destroy these objects.
    bool is_direct_entry(ASTContext& ctx, FunctionDecl* fd) {
        SuspendFunctionAnalyzer analyzer(ctx, fd);
        if (!analyzer.analyze()) return false;
        const auto& points = analyzer.get_suspend_points();
        if (points.empty()) return true;
        const auto tail = org::jetbrains::kotlin::backend::common::collect_tail_suspend_calls(fd);
        if (tail.has_not_tail_suspend_calls) return false;
        bool needs_retained_storage = false;
        for (const auto* call : tail.call_sites) {
            if (const auto* callee = call->getDirectCallee()) {
                for (unsigned i = 0; i < std::min(call->getNumArgs(), callee->getNumParams()); ++i)
                    if (callee->getParamDecl(i)->getType()->isReferenceType() &&
                        call->getArg(i)->IgnoreUnlessSpelledInSource()->isPRValue()) needs_retained_storage = true;
            }
            if (const auto* member = dyn_cast<CXXMemberCallExpr>(call); member &&
                member->getImplicitObjectArgument()->IgnoreUnlessSpelledInSource()->isPRValue()) needs_retained_storage = true;
        }
        std::function<void(const Stmt*)> visit = [&](const Stmt* statement) {
            if (!statement) return;
            if (const auto* lambda = dyn_cast<LambdaExpr>(statement)) {
                for (const auto* initializer : lambda->capture_inits()) visit(initializer);
                return;
            }
            // NOTE(port): C++ catch regions own the caught exception's lifetime.
            // Keep that exception alive until a suspended handler finishes.
            if (isa<CXXCatchStmt>(statement)) needs_retained_storage = true;
            if (const auto* declaration = dyn_cast<DeclStmt>(statement)) {
                for (const auto* item : declaration->decls())
                    if (const auto* variable = dyn_cast<VarDecl>(item)) {
                        const auto* record = ctx.getBaseElementType(variable->getType())->getAsCXXRecordDecl();
                        if (record && !record->hasTrivialDestructor()) needs_retained_storage = true;
                        // A reference local can extend a temporary's lifetime.
                        if (variable->getType()->isReferenceType()) needs_retained_storage = true;
                    }
            }
            for (const auto* child : statement->children()) visit(child);
        };
        visit(fd->getBody());
        return !needs_retained_storage && std::all_of(points.begin(), points.end(), [&](const auto& point) {
            return tail.call_sites.contains(dyn_cast<CallExpr>(point.suspend_stmt));
        });
    }

    bool emit_direct_entry(llvm::raw_ostream& os, ASTContext& ctx, FunctionDecl* fd,
                         const PrintingPolicy& pp, const SourceManager& sm,
                         const LangOptions& lo) {
        // Keep this path to free functions whose names can be emitted without
        // synthesizing an enclosing class or namespace declaration.
        if (!fd->getDeclContext()->isTranslationUnit()) return false;
        if (!is_direct_entry(ctx, fd)) return false;

        os << "// Direct continuation ABI entry; no non-tail suspension.\n";
        if (fd->getStorageClass() == SC_Static) os << "static ";
        if (fd->isInlineSpecified()) os << "inline ";
        if (fd->isConstexpr()) os << "constexpr ";
        os << fd->getReturnType().getAsString(pp) << " " << fd->getNameAsString() << "(";
        bool first = true;
        for (const auto* parameter : fd->parameters()) {
            if (!first) os << ", ";
            first = false;
            parameter->getType().print(os, pp, parameter->getNameAsString());
        }
        const auto* prototype = fd->getType()->getAs<FunctionProtoType>();
        if (prototype && prototype->isVariadic()) os << (first ? "..." : ", ...");
        os << ")";
        if (prototype && prototype->getNoexceptExpr())
            os << " noexcept(" << getStmtText(prototype->getNoexceptExpr(), sm, lo) << ")";
        else if (prototype && prototype->isNothrow()) os << " noexcept";
        os << " " << add_tail_continuation(ctx, fd).first << "\n\n";
        return true;
    }

    CompilerInstance& compiler_;
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
        if (is_lowering_parser_active()) return std::make_unique<ASTConsumer>();
        return std::make_unique<KotlinxSuspendConsumer>(
            ci,
            outDir_, dispatchMode_, spillMode_);
    }

    bool ParseArgs(const CompilerInstance& ci, const std::vector<std::string>& args) override {
        for (const std::string& a : args) {
            if (a.rfind("out-dir=", 0) == 0) {
                outDir_ = a.substr(std::string("out-dir=").size());
            }
            else if (a == "dispatch=switch") {
                auto id = ci.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Error,
                    "switch dispatch is unsupported: Kotlin/Native LLVM injection is required");
                ci.getDiagnostics().Report(id);
                return false;
            }
            else if (a == "dispatch=goto") {
                dispatchMode_ = DispatchMode::ComputedGoto;
            }
            else if (a == "spill=all") {
                spillMode_ = SpillMode::All;
            }
            else if (a == "spill=liveness") {
                auto id = ci.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Error,
                    "liveness-guided frame-field selection is not implemented");
                ci.getDiagnostics().Report(id);
                return false;
            }
        }
        return true;
    }

    ActionType getActionType() override {
        return AddBeforeMainAction;
    }

private:
    std::string outDir_;
    DispatchMode dispatchMode_ = DispatchMode::ComputedGoto;  // K/N binary compatible
    SpillMode spillMode_ = SpillMode::All;
};

} // namespace

static FrontendPluginRegistry::Add<KotlinxSuspendAction>
    X("kotlinx-suspend", "Kotlin-style suspend function transformer with computed-goto support");
