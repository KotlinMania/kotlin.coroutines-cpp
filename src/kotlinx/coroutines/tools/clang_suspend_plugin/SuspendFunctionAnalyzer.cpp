// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:24-267
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/CoroutinesLivenessAnalysis.kt:31-44
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/InitializersLowering.kt:34-55
#include "SuspendFunctionAnalyzer.hpp"
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <utility>

#include "clang/AST/Attr.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/QualTypeNames.h"
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

    spilled_variables_.clear();
    collect_local_variables();
    find_suspend_points();

    if (!suspend_points_.empty()) {
        compute_liveness();
    }

    return true;
}

// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/InitializersLowering.kt:34-55
// NOTE(port): Clang retains written and semantic forms of an initializer list.
// Only the semantic form describes actual field order and selected defaults.
const InitListExpr* SuspendFunctionAnalyzer::evaluated_initializer_list(const InitListExpr* initializer) {
    if (initializer->isSemanticForm()) return initializer;
    const auto* semantic = initializer->getSemanticForm();
    return semantic ? semantic : initializer;
}

/// AST visitor to collect all local variable declarations.
class LocalVariableCollector : public RecursiveASTVisitor<LocalVariableCollector> {
public:
    std::vector<const VarDecl*> variables;
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/InitializersLowering.kt:34-55
    // NOTE(port): Visit the resolved operands once, rather than both AST forms.
    bool TraverseInitListExpr(InitListExpr* initializer) {
        const auto* selected = SuspendFunctionAnalyzer::evaluated_initializer_list(initializer);
        for (const auto* value : selected->inits())
            if (!TraverseStmt(const_cast<Expr*>(value))) return false;
        return TraverseStmt(const_cast<Expr*>(selected->getArrayFiller()));
    }

    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/DefaultArgumentStubGenerator.kt:91-108
    bool TraverseCXXDefaultArgExpr(CXXDefaultArgExpr* expression) {
        return TraverseStmt(expression->getExpr());
    }
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/InitializersLowering.kt:34-55
    bool TraverseCXXDefaultInitExpr(CXXDefaultInitExpr* expression) {
        return TraverseStmt(expression->getExpr());
    }
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
        // NOTE(port): Shared default ASTs reuse declaration identities across
        // uses; the variable inventory records each actual VarDecl once.
        if (vd->isLocalVarDecl() && !isa<ParmVarDecl>(vd) &&
            std::find(variables.begin(), variables.end(), vd) == variables.end()) {
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
    DefaultArgumentReferences(ASTContext& context, const PrintingPolicy& policy)
        : context_(context), policy_(policy) {}
    bool handledStmt(Stmt* statement, llvm::raw_ostream& output) override {
        const auto* reference = dyn_cast<DeclRefExpr>(statement);
        if (!reference) return false;
        return SuspendFunctionAnalyzer::print_declaration_reference(reference, context_, policy_, output);
    }
private:
    ASTContext& context_;
    const PrintingPolicy& policy_;
};
}

// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/DefaultArgumentStubGenerator.kt:91-108
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractFunctionReferenceLowering.kt:261-288
// NOTE(port): Preserve resolved non-local Clang declaration identity when a
// default expression or callable body moves into another lexical context.
// Kotlin IR retains these symbols while remapping locals and declaration parents.
bool SuspendFunctionAnalyzer::print_declaration_reference(
    const DeclRefExpr* reference, ASTContext& context, const PrintingPolicy& policy,
    llvm::raw_ostream& output) {
    const auto* declaration = reference->getDecl();
    if (const auto* variable = dyn_cast<VarDecl>(declaration); variable && variable->isLocalVarDeclOrParm()) return false;
    if (isa<NonTypeTemplateParmDecl>(declaration)) return false;
    const auto* parent = declaration->getDeclContext();
    if (const auto* record = dyn_cast<CXXRecordDecl>(parent)) {
        for (const auto* owner = record->getDeclContext(); !owner->isTranslationUnit(); owner = owner->getParent())
            if (owner->isFunctionOrMethod()) return false;
        auto type = TypeName::getFullyQualifiedName(context.getCanonicalTypeDeclType(record), context, policy);
        if (!type.starts_with("::")) output << "::";
        output << type << "::";
    } else {
        std::vector<const NamedDecl*> scopes;
        std::string prefix = "::";
        for (; !parent->isTranslationUnit(); parent = parent->getParent()) {
            if (const auto* scope = dyn_cast<NamespaceDecl>(parent)) {
                if (!scope->isAnonymousNamespace()) scopes.push_back(scope);
            } else if (const auto* scope = dyn_cast<EnumDecl>(parent)) {
                if (scope->getIdentifier()) scopes.push_back(scope);
            } else if (const auto* scope = dyn_cast<CXXRecordDecl>(parent)) {
                for (const auto* owner = scope->getDeclContext(); !owner->isTranslationUnit(); owner = owner->getParent())
                    if (owner->isFunctionOrMethod()) return false;
                prefix = TypeName::getFullyQualifiedName(context.getCanonicalTypeDeclType(scope), context, policy);
                if (!prefix.starts_with("::")) prefix = "::" + prefix;
                prefix += "::";
                break;
            } else return false;
        }
        output << prefix;
        for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope)
            output << (*scope)->getNameAsString() << "::";
    }
    output << declaration->getNameAsString();
    if (reference->hasExplicitTemplateArgs()) {
        output << "<";
        bool preceding = false;
        for (const auto& argument : reference->template_arguments()) {
            if (preceding) output << ", ";
            argument.getArgument().print(policy, output, true);
            preceding = true;
        }
        output << ">";
    }
    return true;
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
    const auto* method = dyn_cast_or_null<CXXMethodDecl>(call->getDirectCallee());
    const bool operator_receiver = isa<CXXOperatorCallExpr>(call) && method && !method->isStatic();
    for (const auto* argument : call->arguments()) {
        // NOTE(port): Clang stores an operator receiver as argument zero,
        // but it does not appear inside the authored call's parentheses.
        if (operator_receiver && argument == call->getArg(0)) continue;
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
    auto resolved_policy = policy;
    resolved_policy.PrintAsCanonical = true;
    resolved_policy.SuppressScope = false;
    DefaultArgumentReferences references(argument->getParam()->getASTContext(), resolved_policy);
    argument->getExpr()->printPretty(output, &references, resolved_policy);
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
        // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/InitializersLowering.kt:34-55
        // NOTE(port): Visit the resolved operands once, rather than both AST forms.
        bool TraverseInitListExpr(InitListExpr* initializer) {
            const auto* selected = SuspendFunctionAnalyzer::evaluated_initializer_list(initializer);
            for (const auto* value : selected->inits())
                if (!TraverseStmt(const_cast<Expr*>(value))) return false;
            return TraverseStmt(const_cast<Expr*>(selected->getArrayFiller()));
        }

        bool TraverseDecltypeTypeLoc(DecltypeTypeLoc, bool = true) { return true; }
        // NOTE(port): A selected default is an evaluated call operand, so its
        // dependent overloads must resolve before the caller frame is installed.
        bool TraverseCXXDefaultArgExpr(CXXDefaultArgExpr* expression) {
            return TraverseStmt(expression->getExpr());
        }
        // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/InitializersLowering.kt:34-55
        bool TraverseCXXDefaultInitExpr(CXXDefaultInitExpr* expression) {
            return TraverseStmt(expression->getExpr());
        }
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

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:367-388
// NOTE(port): Clang keeps unevaluated operands, discarded constexpr arms and
// nested function bodies in its AST. Only this function's executed expressions
// participate in the Kotlin suspend-call walk.
void SuspendFunctionAnalyzer::find_suspend_points() {
    class SuspensionPoints : public RecursiveASTVisitor<SuspensionPoints> {
    public:
        explicit SuspensionPoints(ASTContext& context) : context_(context) {}
        std::vector<SuspendPointInfo> points;
        bool shouldTraversePostOrder() const { return true; }
        bool TraverseStmt(Stmt* statement) {
            if (SuspendFunctionAnalyzer::is_unevaluated_expression(statement)) return true;
            return RecursiveASTVisitor<SuspensionPoints>::TraverseStmt(statement);
        }
        // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/InitializersLowering.kt:34-55
        // NOTE(port): Visit the resolved operands once, rather than both AST forms.
        bool TraverseInitListExpr(InitListExpr* initializer) {
            const auto* selected = SuspendFunctionAnalyzer::evaluated_initializer_list(initializer);
            for (const auto* value : selected->inits())
                if (!TraverseStmt(const_cast<Expr*>(value))) return false;
            return TraverseStmt(const_cast<Expr*>(selected->getArrayFiller()));
        }

        bool TraverseDecltypeTypeLoc(DecltypeTypeLoc, bool = true) { return true; }
        // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/DefaultArgumentStubGenerator.kt:91-108
        bool TraverseCXXDefaultArgExpr(CXXDefaultArgExpr* expression) {
            default_expression_path_.push_back(expression);
            const bool traversed = TraverseStmt(expression->getExpr());
            default_expression_path_.pop_back();
            return traversed;
        }
        // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/InitializersLowering.kt:34-55
        bool TraverseCXXDefaultInitExpr(CXXDefaultInitExpr* expression) {
            default_expression_path_.push_back(expression);
            const bool traversed = TraverseStmt(expression->getExpr());
            default_expression_path_.pop_back();
            return traversed;
        }
        bool TraverseDecl(Decl* declaration) {
            if (declaration && (isa<RecordDecl>(declaration) || isa<FunctionDecl>(declaration))) return true;
            return RecursiveASTVisitor<SuspensionPoints>::TraverseDecl(declaration);
        }
        bool TraverseLambdaExpr(LambdaExpr* expression) {
            for (auto* initializer : expression->capture_inits())
                if (!TraverseStmt(initializer)) return false;
            return true;
        }
        bool TraverseIfStmt(IfStmt* branch) {
            if (!branch->isConstexpr()) return RecursiveASTVisitor<SuspensionPoints>::TraverseIfStmt(branch);
            auto selected = branch->getNondiscardedCase(context_);
            if (!selected) throw std::runtime_error("suspend-call walk requires resolved constexpr branch");
            return TraverseStmt(branch->getInit()) && TraverseStmt(branch->getConditionVariableDeclStmt()) &&
                TraverseStmt(*selected);
        }
        bool VisitStmt(Stmt* statement) {
            if (!SuspendFunctionAnalyzer::is_suspend_call(statement)) return true;
            // NOTE(port): Implicit expression wrappers do not add IR calls.
            if (const auto* expression = dyn_cast<Expr>(statement);
                expression && expression != expression->IgnoreParenImpCasts()) return true;
            if (const auto* call = dyn_cast<CallExpr>(statement);
                call && SuspendFunctionAnalyzer::is_suspend_wrapper(call) && call->getNumArgs() == 1 &&
                SuspendFunctionAnalyzer::is_suspend_call(call->getArg(0)->IgnoreUnlessSpelledInSource())) return true;
            points.push_back({statement, static_cast<unsigned>(points.size() + 1), {}, default_expression_path_});
            return true;
        }
    private:
        ASTContext& context_;
        std::vector<const Stmt*> default_expression_path_;
    } visitor(ctx_);
    visitor.TraverseStmt(fd_->getBody());
    suspend_points_ = std::move(visitor.points);
}

namespace {
using LiveVariables = std::set<const VarDecl*>;

// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/DefaultArgumentStubGenerator.kt:91-108
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:47,79-89
// NOTE(port): Kotlin prepares default expressions in the selected function's IR.
// Clang shares their declaration AST instead. Actual enclosing default-use nodes
// distinguish evaluated occurrences, including nested defaults and member defaults.
using SuspensionOccurrence = std::pair<const Stmt*, std::vector<const Stmt*>>;

// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:47,79-89
// NOTE(port): The ordered C++ map needs a total order over actual AST pointers.
struct SuspensionOccurrenceLess {
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:47,79-89
    bool operator()(const SuspensionOccurrence& left, const SuspensionOccurrence& right) const {
        const std::less<const Stmt*> before;
        if (left.first != right.first) return before(left.first, right.first);
        return std::lexicographical_compare(left.second.begin(), left.second.end(),
                                            right.second.begin(), right.second.end(), before);
    }
};
using SuspensionLiveness = std::map<SuspensionOccurrence, LiveVariables, SuspensionOccurrenceLess>;

// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:44-267
// NOTE(port): Clang node dispatch adapts Kotlin's IR visitor. Concrete declaration
// identities replace variable bit indices; C++ parameters are also tracked because
// their source bodies have not yet been rewritten to argument-field reads.
class LivenessAnalysisVisitor {
public:
    explicit LivenessAnalysisVisitor(const ASTContext& context) : context_(context) {}

    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:54-59,248-265
    SuspensionLiveness run(const Stmt* body) {
        // NOTE(port): Kotlin returnable-block and loop symbols have structured
        // targets. C++ labels can form cycles outside loops; saturate their
        // actual declaration targets with the same backwards fixed-point rule.
        for (;;) {
            auto previous_targets = label_starts_;
            accept(body, {});
            if (previous_targets == label_starts_) break;
        }
        return filtered_element_ends_;
    }

private:
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:79-89
    void save(const Stmt* element, const LiveVariables& data) {
        if (!SuspendFunctionAnalyzer::is_suspend_call(element)) return;
        auto& live = filtered_element_ends_[{element, default_expression_path_}];
        live.insert(data.begin(), data.end());
        live.insert(catches_.begin(), catches_.end());
        // NOTE(port): The LLVM label field and point IDs are not source VarDecls,
        // so Kotlin's suspensionPointIdParameters cannot enter this set.
    }

    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:95-102
    LiveVariables visit_element(const Stmt* element, LiveVariables data) {
        std::vector<const Stmt*> children;
        for (const auto* child : element->children()) if (child) children.push_back(child);
        for (auto child = children.rbegin(); child != children.rend(); ++child)
            data = accept(*child, std::move(data));
        return data;
    }

    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:109-121
    LiveVariables visit_variable(const VarDecl* variable, LiveVariables data) {
        // NOTE(port): Clang owns diagnostics for uninitialized C++ reads;
        // liveness does not introduce another runtime initialization check.
        data.erase(variable);
        return accept(variable->getInit(), std::move(data));
    }

    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:154-165
    LiveVariables visit_when(const Stmt* condition, const Stmt* selected,
                             const Stmt* otherwise, const LiveVariables& data) {
        auto live = accept(otherwise, data);
        auto branch = accept(selected, data);
        live.insert(branch.begin(), branch.end());
        return accept(condition, std::move(live));
    }

    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:177-192
    LiveVariables visit_try(const CXXTryStmt* region, const LiveVariables& data) {
        LiveVariables current_catches;
        for (unsigned index = 0; index < region->getNumHandlers(); ++index) {
            const auto* handler = region->getHandler(index);
            auto live = accept(handler->getHandlerBlock(), data);
            if (const auto* variable = handler->getExceptionDecl()) live.erase(variable);
            current_catches.insert(live.begin(), live.end());
        }
        auto previous_catches = catches_;
        catches_.insert(current_catches.begin(), current_catches.end());
        auto after_try = data;
        after_try.insert(current_catches.begin(), current_catches.end());
        auto before_try = accept(region->getTryBlock(), std::move(after_try));
        current_catches.insert(before_try.begin(), before_try.end());
        catches_ = std::move(previous_catches);
        return current_catches;
    }

    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:222-265
    // NOTE(port): For/range-for increments execute before the next condition;
    // continue targets that increment, while break targets the loop's end.
    LiveVariables handle_loop(const Stmt* condition,
                              const Stmt* body, const Stmt* increment,
                              const VarDecl* condition_variable,
                              const DeclStmt* iteration_variable,
                              const LiveVariables& data, bool at_least_once) {
        break_targets_.push_back(data);
        continue_targets_.emplace_back();
        auto condition_start = [&](LiveVariables after) {
            if (condition) after.insert(data.begin(), data.end());
            after = accept(condition, std::move(after));
            if (condition_variable) after = visit_variable(condition_variable, std::move(after));
            return after;
        };
        auto body_end = accept(increment, condition_start(condition ? data : LiveVariables{}));
        LiveVariables body_start;
        for (;;) {
            continue_targets_.back() = body_end;
            auto current_start = accept(body, body_end);
            current_start = accept(iteration_variable, std::move(current_start));
            body_start.insert(current_start.begin(), current_start.end());
            auto next_end = accept(increment, condition_start(std::move(current_start)));
            if (next_end == body_end) break;
            body_end = std::move(next_end);
        }
        continue_targets_.pop_back();
        break_targets_.pop_back();
        if (at_least_once) return body_start;
        // NOTE(port): A C++ for(;;) has no zero-iteration condition edge.
        if (!condition) return body_start;
        body_start.insert(data.begin(), data.end());
        return condition_start(std::move(body_start));
    }

    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:154-165,202-204
    // NOTE(port): C++ switch permits fallthrough. Traverse the body backwards
    // and union each label's incoming values at its dispatch instead of treating
    // every labelled result as an independent Kotlin when branch.
    LiveVariables visit_switch(const SwitchStmt* branch, const LiveVariables& data) {
        break_targets_.push_back(data);
        switch_entries_.emplace_back();
        accept(branch->getBody(), data);
        auto live = std::move(switch_entries_.back());
        switch_entries_.pop_back();
        break_targets_.pop_back();
        bool exhaustive = false;
        for (const auto* label = branch->getSwitchCaseList(); label; label = label->getNextSwitchCase())
            exhaustive = exhaustive || isa<DefaultStmt>(label);
        if (!exhaustive) live.insert(data.begin(), data.end());
        live = accept(branch->getCond(), std::move(live));
        if (const auto* variable = branch->getConditionVariable())
            live = visit_variable(variable, std::move(live));
        return accept(branch->getInit(), std::move(live));
    }

    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:104-265
    LiveVariables accept(const Stmt* element, LiveVariables data) {
        if (!element || SuspendFunctionAnalyzer::is_unevaluated_expression(element)) return data;
        save(element, data);
        // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/InitializersLowering.kt:34-55
        // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:95-102
        if (const auto* initializer = dyn_cast<InitListExpr>(element)) {
            const auto* selected = SuspendFunctionAnalyzer::evaluated_initializer_list(initializer);
            data = accept(selected->getArrayFiller(), std::move(data));
            for (unsigned index = selected->getNumInits(); index > 0; --index)
                data = accept(selected->getInit(index - 1), std::move(data));
            return data;
        }
        if (const auto* reference = dyn_cast<DeclRefExpr>(element)) {
            if (const auto* variable = dyn_cast<VarDecl>(reference->getDecl())) data.insert(variable);
            else if (const auto* binding = dyn_cast<BindingDecl>(reference->getDecl())) {
                if (const auto* variable = dyn_cast<VarDecl>(binding->getDecomposedDecl())) data.insert(variable);
            }
            return data;
        }
        if (const auto* declaration = dyn_cast<DeclStmt>(element)) {
            std::vector<const VarDecl*> variables;
            for (const auto* child : declaration->decls())
                if (const auto* variable = dyn_cast<VarDecl>(child)) variables.push_back(variable);
            for (auto variable = variables.rbegin(); variable != variables.rend(); ++variable)
                data = visit_variable(*variable, std::move(data));
            // NOTE(port): Nested class/function bodies and type declarations
            // do not execute in this function's value-liveness traversal.
            return data;
        }
        if (const auto* assignment = dyn_cast<BinaryOperator>(element)) {
            if (assignment->isAssignmentOp()) {
                const auto* target = dyn_cast<DeclRefExpr>(assignment->getLHS()->IgnoreParenImpCasts());
                const auto* variable = target ? dyn_cast<VarDecl>(target->getDecl()) : nullptr;
                // NOTE(port): Writing through a reference does not redefine the
                // retained binding; member/subscript destinations read receivers.
                if (variable && !variable->getType()->isReferenceType()) {
                    data.erase(variable);
                    if (assignment->isCompoundAssignmentOp()) data.insert(variable);
                    return accept(assignment->getRHS(), std::move(data));
                }
                return visit_element(element, std::move(data));
            }
            if (assignment->isLogicalOp()) {
                auto selected = accept(assignment->getRHS(), data);
                selected.insert(data.begin(), data.end());
                return accept(assignment->getLHS(), std::move(selected));
            }
        }
        // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/DefaultArgumentStubGenerator.kt:91-108
        // NOTE(port): Keep the use path until the selected declaration expression
        // has been visited; sibling uses of that AST have independent snapshots.
        if (const auto* omitted = dyn_cast<CXXDefaultArgExpr>(element)) {
            default_expression_path_.push_back(omitted);
            data = accept(omitted->getExpr(), std::move(data));
            default_expression_path_.pop_back();
            return data;
        }
        // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/InitializersLowering.kt:34-55
        if (const auto* initialized = dyn_cast<CXXDefaultInitExpr>(element)) {
            default_expression_path_.push_back(initialized);
            data = accept(initialized->getExpr(), std::move(data));
            default_expression_path_.pop_back();
            return data;
        }
        // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/optimizations/LivenessAnalysis.kt:123-136,248-265
        // NOTE(port): Retain Clang's declaration-bound jump targets. Values on
        // the unreachable lexical suffix do not flow through an unconditional jump.
        if (const auto* jump = dyn_cast<GotoStmt>(element)) return label_starts_[jump->getLabel()];
        if (const auto* label = dyn_cast<LabelStmt>(element)) {
            auto before = accept(label->getSubStmt(), std::move(data));
            auto& target = label_starts_[label->getDecl()];
            target.insert(before.begin(), before.end());
            return before;
        }
        if (const auto* address = dyn_cast<AddrLabelExpr>(element)) {
            addressed_labels_.insert(address->getLabel());
            return data;
        }
        if (const auto* jump = dyn_cast<IndirectGotoStmt>(element)) {
            LiveVariables targets;
            for (const auto* label : addressed_labels_) {
                const auto& live = label_starts_[label];
                targets.insert(live.begin(), live.end());
            }
            return accept(jump->getTarget(), std::move(targets));
        }
        if (const auto* returned = dyn_cast<ReturnStmt>(element))
            return accept(returned->getRetValue(), {});
        if (const auto* thrown = dyn_cast<CXXThrowExpr>(element))
            return accept(thrown->getSubExpr(), catches_);
        if (const auto* branch = dyn_cast<ConditionalOperator>(element))
            return visit_when(branch->getCond(), branch->getTrueExpr(), branch->getFalseExpr(), data);
        if (const auto* branch = dyn_cast<IfStmt>(element)) {
            if (branch->isConstexpr()) {
                auto selected = branch->getNondiscardedCase(context_);
                if (!selected) throw std::runtime_error("liveness requires resolved constexpr branch");
                data = accept(*selected, std::move(data));
            } else data = visit_when(branch->getCond(), branch->getThen(), branch->getElse(), data);
            if (const auto* variable = branch->getConditionVariable())
                data = visit_variable(variable, std::move(data));
            return accept(branch->getInit(), std::move(data));
        }
        if (const auto* region = dyn_cast<CXXTryStmt>(element)) return visit_try(region, data);
        if (const auto* loop = dyn_cast<WhileStmt>(element))
            return handle_loop(loop->getCond(), loop->getBody(), nullptr,
                               loop->getConditionVariable(), nullptr, data, false);
        if (const auto* loop = dyn_cast<DoStmt>(element))
            return handle_loop(loop->getCond(), loop->getBody(), nullptr, nullptr, nullptr, data, true);
        if (const auto* loop = dyn_cast<ForStmt>(element)) {
            data = handle_loop(loop->getCond(), loop->getBody(), loop->getInc(),
                               loop->getConditionVariable(), nullptr, data, false);
            return accept(loop->getInit(), std::move(data));
        }
        if (const auto* loop = dyn_cast<CXXForRangeStmt>(element)) {
            data = handle_loop(loop->getCond(), loop->getBody(), loop->getInc(),
                               nullptr, loop->getLoopVarStmt(), data, false);
            data = accept(loop->getEndStmt(), std::move(data));
            data = accept(loop->getBeginStmt(), std::move(data));
            data = accept(loop->getRangeStmt(), std::move(data));
            return accept(loop->getInit(), std::move(data));
        }
        if (isa<BreakStmt>(element)) {
            if (break_targets_.empty()) throw std::runtime_error("unknown liveness break target");
            return break_targets_.back();
        }
        if (isa<ContinueStmt>(element)) {
            if (continue_targets_.empty()) throw std::runtime_error("unknown liveness continue target");
            return continue_targets_.back();
        }
        if (const auto* branch = dyn_cast<SwitchStmt>(element)) return visit_switch(branch, data);
        if (const auto* label = dyn_cast<SwitchCase>(element)) {
            data = accept(label->getSubStmt(), std::move(data));
            if (switch_entries_.empty()) throw std::runtime_error("unknown liveness switch target");
            switch_entries_.back().insert(data.begin(), data.end());
            return data;
        }
        if (const auto* lambda = dyn_cast<LambdaExpr>(element)) {
            std::vector<const Expr*> initializers(lambda->capture_init_begin(), lambda->capture_init_end());
            for (auto initializer = initializers.rbegin(); initializer != initializers.rend(); ++initializer)
                data = accept(*initializer, std::move(data));
            return data;
        }
        return visit_element(element, std::move(data));
    }

    const ASTContext& context_;
    SuspensionLiveness filtered_element_ends_;
    std::vector<const Stmt*> default_expression_path_;
    std::map<const LabelDecl*, LiveVariables> label_starts_;
    std::set<const LabelDecl*> addressed_labels_;
    std::vector<LiveVariables> break_targets_;
    std::vector<LiveVariables> continue_targets_;
    std::vector<LiveVariables> switch_entries_;
    LiveVariables catches_;
};
}

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/CoroutinesLivenessAnalysis.kt:31-44
void SuspendFunctionAnalyzer::compute_liveness() {
    const auto live = LivenessAnalysisVisitor(ctx_).run(fd_->getBody());
    const auto* completion = continuation_parameter(fd_);
    for (auto& point : suspend_points_) {
        auto found = live.find({point.suspend_stmt, point.default_expression_path});
        if (found == live.end()) continue;
        point.live_variables = found->second;
        // NOTE(port): Completion is stored by the base continuation; source
        // variables from nested declarations are excluded by the visitor.
        point.live_variables.erase(completion);
        for (auto variable = point.live_variables.begin(); variable != point.live_variables.end();) {
            if (!(*variable)->isLocalVarDeclOrParm()) variable = point.live_variables.erase(variable);
            else ++variable;
        }
        spilled_variables_.insert(point.live_variables.begin(), point.live_variables.end());
    }
}

} // namespace suspend
} // namespace kotlinx
