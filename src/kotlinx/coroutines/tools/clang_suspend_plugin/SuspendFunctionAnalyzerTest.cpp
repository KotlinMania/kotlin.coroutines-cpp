#include "SuspendFunctionAnalyzer.hpp"
#include "RestrictSuspensionUtils.hpp"
#include "IrTypeUtils.hpp"
#include "UpgradeCallableReferences.hpp"
#include "NativeFunctionReferenceLowering.hpp"
#include "AbstractFunctionReferenceLowering.hpp"
#include "clang/AST/ASTImporter.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Basic/SourceManager.h"
#include <cassert>
#include <iostream>

using namespace clang;
using namespace kotlinx::suspend;

static FunctionDecl* function(ASTContext& context) {
    for (auto* declaration : context.getTranslationUnitDecl()->decls()) {
        auto* candidate = dyn_cast<FunctionDecl>(declaration);
        if (auto* pattern = dyn_cast<FunctionTemplateDecl>(declaration)) candidate = pattern->getTemplatedDecl();
        if (candidate && candidate->getName() == "probe") return candidate;
    }
    return nullptr;
}

class LambdaFinder : public RecursiveASTVisitor<LambdaFinder> {
public:
    bool VisitLambdaExpr(LambdaExpr* expression) { lambda = expression; return true; }
    LambdaExpr* lambda = nullptr;
};

static bool has_annotation(const Decl* declaration, llvm::StringRef name) {
    for (const auto* annotation : declaration->specific_attrs<AnnotateAttr>())
        if (annotation->getAnnotation() == name) return true;
    return false;
}

int main() {
    using org::jetbrains::kotlin::backend::common::is_restricted_suspension_function;
    using org::jetbrains::kotlin::backend::common::is_restricted_suspension;
    assert(!is_restricted_suspension(nullptr));
    const std::string restricted_types =
        "struct [[clang::annotate(\"kotlin.coroutines.RestrictsSuspension\")]] Root {}; "
        "struct Left : virtual Root {}; struct Right : virtual Root {}; "
        "struct Derived : Left, Right {}; struct Ordinary {}; ";
    for (bool restricted : {false, true}) {
        auto reference_ast = tooling::buildASTFromCodeWithArgs(restricted_types +
            "void probe() { auto block = []([[clang::annotate(\"kotlin.ir.ExtensionReceiver\"), "
            "clang::annotate(\"preserved\")]] " + std::string(restricted ? "Derived" : "Ordinary") +
            "& receiver, [[clang::annotate(\"kotlin.ir.Context\"), clang::annotate(\"kotlin.ir.UNDERSCORE_PARAMETER\")]] int context) {}; }",
            {"-std=c++20"});
        assert(reference_ast && !reference_ast->getDiagnostics().hasErrorOccurred());
        auto* enclosing = function(reference_ast->getASTContext());
        LambdaFinder finder;
        finder.TraverseDecl(enclosing);
        assert(finder.lambda);
        auto* invoke = finder.lambda->getCallOperator();
        assert(is_restricted_suspension_function(invoke) == restricted);
        org::jetbrains::kotlin::backend::common::lower::UpgradeCallableReferences().lower(enclosing);
        assert(!is_restricted_suspension_function(invoke));
        assert(!has_annotation(invoke->getParamDecl(0), "kotlin.ir.ExtensionReceiver"));
        assert(has_annotation(invoke->getParamDecl(0), "kotlin.ir.LAMBDA_EXTENSION_RECEIVER"));
        assert(has_annotation(invoke->getParamDecl(0), "preserved"));
        assert(!has_annotation(invoke->getParamDecl(1), "kotlin.ir.Context"));
        assert(!has_annotation(invoke->getParamDecl(1), "kotlin.ir.UNDERSCORE_PARAMETER"));
        assert(has_annotation(invoke->getParamDecl(1), "kotlin.ir.DEFINED"));
        org::jetbrains::kotlin::backend::konan::lower::NativeFunctionReferenceLowering().lower(enclosing);
        assert(has_annotation(invoke, "kotlin.ir.isRestrictedSuspensionInvokeMethod") == restricted);
        invoke->addAttr(AnnotateAttr::CreateImplicit(reference_ast->getASTContext(), "invoke_metadata", nullptr, 0));
        org::jetbrains::kotlin::backend::common::lower::AbstractFunctionReferenceLowering lowering;
        auto constructed_ast = tooling::buildASTFromCodeWithArgs(restricted_types +
            lowering.build_class(reference_ast->getASTContext(), finder.lambda, "Reference"), {"-std=c++20"});
        assert(constructed_ast && !constructed_ast->getDiagnostics().hasErrorOccurred());
        CXXMethodDecl* constructed_invoke = nullptr;
        for (auto* declaration : constructed_ast->getASTContext().getTranslationUnitDecl()->decls()) {
            auto* record = dyn_cast<CXXRecordDecl>(declaration);
            if (!record || record->getName() != "Reference") continue;
            for (auto* method : record->methods())
                if (method->isOverloadedOperator() && method->getOverloadedOperator() == OO_Call) constructed_invoke = method;
        }
        assert(constructed_invoke && has_annotation(constructed_invoke, "invoke_metadata"));
        assert(has_annotation(constructed_invoke, "kotlin.ir.isRestrictedSuspensionInvokeMethod") == restricted);
        assert(has_annotation(constructed_invoke->getParamDecl(0), "kotlin.ir.LAMBDA_EXTENSION_RECEIVER"));
        assert(has_annotation(constructed_invoke->getParamDecl(0), "preserved"));
        assert(has_annotation(constructed_invoke->getParamDecl(1), "kotlin.ir.DEFINED"));
        auto destination = tooling::buildASTFromCodeWithArgs("", {"-std=c++20"});
        ASTImporter importer(destination->getASTContext(), destination->getFileManager(),
            reference_ast->getASTContext(), reference_ast->getFileManager(), false);
        auto imported = importer.Import(invoke);
        assert(imported);
        assert(has_annotation(*imported, "kotlin.ir.isRestrictedSuspensionInvokeMethod") == restricted);
    }
    auto dispatch_ast = tooling::buildASTFromCodeWithArgs(
        "void probe() { auto block = []([[clang::annotate(\"kotlin.ir.DispatchReceiver\")]] int receiver) {}; }",
        {"-std=c++20"});
    assert(dispatch_ast && !dispatch_ast->getDiagnostics().hasErrorOccurred());
    bool dispatch_rejected = false;
    try {
        org::jetbrains::kotlin::backend::common::lower::UpgradeCallableReferences().lower(function(dispatch_ast->getASTContext()));
    } catch (const std::invalid_argument& error) {
        dispatch_rejected = true;
        assert(std::string(error.what()) == "No dispatch receiver allowed in wrappers");
    }
    assert(dispatch_rejected);
    auto superclass_ast = tooling::buildASTFromCodeWithArgs(
        restricted_types + "void probe(Derived* receiver) {}", {"-std=c++20"});
    assert(superclass_ast && !superclass_ast->getDiagnostics().hasErrorOccurred());
    const auto* derived = function(superclass_ast->getASTContext())->getParamDecl(0)
        ->getType()->getPointeeType()->getAsCXXRecordDecl();
    const auto superclasses = org::jetbrains::kotlin::ir::util::get_all_superclasses(derived);
    assert(superclasses.size() == 3);
    assert(!superclasses.contains(derived->getCanonicalDecl()));
    assert(is_restricted_suspension(derived));
    for (const auto& [parameter, expected] : std::vector<std::pair<std::string, bool>>{
             {"[[clang::annotate(\"kotlin.ir.ExtensionReceiver\")]] Root& receiver", true},
             {"[[clang::annotate(\"kotlin.ir.ExtensionReceiver\")]] const Derived* receiver", true},
             {"Derived& receiver", false},
             {"[[clang::annotate(\"kotlin.ir.ExtensionReceiver\")]] Ordinary& receiver", false},
             {"[[clang::annotate(\"kotlin.ir.ExtensionReceiver\")]] int receiver", false},
             {"Derived& first, [[clang::annotate(\"kotlin.ir.ExtensionReceiver\")]] Ordinary& second", false},
             {"int first, [[clang::annotate(\"kotlin.ir.ExtensionReceiver\")]] Derived& second", false},
             {"", false}}) {
        auto restricted_ast = tooling::buildASTFromCodeWithArgs(
            restricted_types + "void probe(" + parameter + ") {}", {"-std=c++20"});
        assert(restricted_ast && !restricted_ast->getDiagnostics().hasErrorOccurred());
        assert(is_restricted_suspension_function(function(restricted_ast->getASTContext())) == expected);
    }
    const std::string declarations = "namespace kotlinx::coroutines::dsl { void* suspend(void*); } using kotlinx::coroutines::dsl::suspend; void* external_call(int);\n";
    auto ordered_ast = tooling::buildASTFromCodeWithArgs(declarations +
        "int probe(int value) { suspend(external_call(1)); suspend(external_call(2)); return value; }", {"-std=c++20"});
    assert(ordered_ast);
    auto& ordered_context = ordered_ast->getASTContext();
    SuspendFunctionAnalyzer ordered(ordered_context, function(ordered_context));
    assert(ordered.analyze());
    const auto& points = ordered.get_suspend_points();
    assert(points.size() == 2 && points[0].state_id == 1 && points[1].state_id == 2);
    assert(ordered_context.getSourceManager().isBeforeInTranslationUnit(
        points[0].suspend_stmt->getBeginLoc(), points[1].suspend_stmt->getBeginLoc()));
    assert(points[0].live_variables.contains(function(ordered_context)->getParamDecl(0)));
    assert(points[1].live_variables.contains(function(ordered_context)->getParamDecl(0)));

    auto annotated_ast = tooling::buildASTFromCodeWithArgs(
        "namespace kotlinx::coroutines::dsl { void* suspend(void*); } "
        "[[clang::annotate(\"suspend\")]] void* external_call(int); "
        "void* probe(int value) { return kotlinx::coroutines::dsl::suspend(external_call(value)); }",
        {"-std=c++20"});
    assert(annotated_ast);
    auto& annotated_context = annotated_ast->getASTContext();
    SuspendFunctionAnalyzer annotated(annotated_context, function(annotated_context));
    assert(annotated.analyze() && annotated.get_suspend_points().size() == 1);

    auto dependent_ast = tooling::buildASTFromCodeWithArgs(
        "namespace kotlinx::coroutines::dsl { template<class T> T suspend(T&&); } "
        "template<class T> [[clang::annotate(\"suspend\")]] void* external_call(T&); "
        "template<class T> void* probe(T& value) { "
        "kotlinx::coroutines::dsl::suspend(external_call(value)); return external_call(value); }",
        {"-std=c++20"});
    assert(dependent_ast && !dependent_ast->getDiagnostics().hasErrorOccurred());
    auto& dependent_context = dependent_ast->getASTContext();
    SuspendFunctionAnalyzer dependent(dependent_context, function(dependent_context));
    assert(dependent.analyze() && dependent.get_suspend_points().size() == 2);
    assert(SuspendFunctionAnalyzer::requires_overload_resolution(function(dependent_context)));

    auto adl_ast = tooling::buildASTFromCodeWithArgs(
        "[[clang::annotate(\"suspend\")]] void* known(); void* adl_call(...); "
        "template<class T> void* probe(T value) { known(); return adl_call(value); }",
        {"-std=c++20"});
    assert(adl_ast && !adl_ast->getDiagnostics().hasErrorOccurred());
    auto& adl_context = adl_ast->getASTContext();
    assert(SuspendFunctionAnalyzer::requires_overload_resolution(function(adl_context)));

    auto mixed_ast = tooling::buildASTFromCodeWithArgs(
        "[[clang::annotate(\"suspend\")]] void* known(); "
        "[[clang::annotate(\"suspend\")]] void* mixed(int); void* mixed(double); "
        "template<class T> void* probe(T value) { known(); return mixed(value); }",
        {"-std=c++20"});
    assert(mixed_ast && !mixed_ast->getDiagnostics().hasErrorOccurred());
    auto& mixed_context = mixed_ast->getASTContext();
    assert(SuspendFunctionAnalyzer::requires_overload_resolution(function(mixed_context)));

    auto unknown_ast = tooling::buildASTFromCodeWithArgs(
        "[[clang::annotate(\"suspend\")]] void* known(); "
        "template<class T> void* probe(T& receiver) { known(); return receiver.await(); }",
        {"-std=c++20"});
    assert(unknown_ast && !unknown_ast->getDiagnostics().hasErrorOccurred());
    auto& unknown_context = unknown_ast->getASTContext();
    assert(SuspendFunctionAnalyzer::requires_overload_resolution(function(unknown_context)));

    auto member_ast = tooling::buildASTFromCodeWithArgs(
        "struct Host { template<class T> [[clang::annotate(\"suspend\")]] void* await(T&); }; "
        "template<class T> void* probe(T& value, Host& host) { return host.await(value); }",
        {"-std=c++20"});
    assert(member_ast && !member_ast->getDiagnostics().hasErrorOccurred());
    auto& member_context = member_ast->getASTContext();
    SuspendFunctionAnalyzer member(member_context, function(member_context));
    assert(member.analyze() && member.get_suspend_points().size() == 1);

    auto pointer_ast = tooling::buildASTFromCodeWithArgs(
        "[[clang::annotate(\"suspend\")]] void* target(int); void* ordinary(int); "
        "template<auto Fn> void* probe(int value) { return Fn(value); } "
        "void* invoke() { probe<&target>(1); return probe<&ordinary>(2); }",
        {"-std=c++20"});
    assert(pointer_ast && !pointer_ast->getDiagnostics().hasErrorOccurred());
    auto& pointer_context = pointer_ast->getASTContext();
    unsigned specializations = 0;
    for (auto* concrete : function(pointer_context)->getDescribedFunctionTemplate()->specializations()) {
        assert(concrete->hasBody());
        auto* target = concrete->getTemplateSpecializationArgs()->get(0).getAsDecl();
        SuspendFunctionAnalyzer pointer(pointer_context, concrete);
        assert(pointer.analyze());
        assert(pointer.get_suspend_points().size() == (target->getNameAsString() == "target" ? 1 : 0));
        ++specializations;
    }
    assert(specializations == 2);

    auto lambda_ast = tooling::buildASTFromCodeWithArgs(
        "template<class T> int probe(T value) { auto closure = [value] { "
        "int inner = 7; return deferred_call(value) + inner; }; return 0; }", {"-std=c++20"});
    assert(lambda_ast && !lambda_ast->getDiagnostics().hasErrorOccurred());
    auto& lambda_context = lambda_ast->getASTContext();
    auto* lambda_function = function(lambda_context);
    assert(!SuspendFunctionAnalyzer::requires_overload_resolution(lambda_function));
    SuspendFunctionAnalyzer lambda(lambda_context, lambda_function);
    assert(lambda.analyze() && lambda.get_suspend_points().empty());
    assert(lambda.get_local_variables().size() == 2);
    for (const auto* variable : lambda.get_local_variables()) assert(variable->getName() != "inner");

    auto local_class_ast = tooling::buildASTFromCodeWithArgs(
        "template<class T> int probe(T value) { struct Local { int read(T other) { "
        "int inner = 7; return deferred_call(other) + inner; } }; return 0; }", {"-std=c++20"});
    assert(local_class_ast && !local_class_ast->getDiagnostics().hasErrorOccurred());
    auto& local_class_context = local_class_ast->getASTContext();
    auto* local_class_function = function(local_class_context);
    assert(!SuspendFunctionAnalyzer::requires_overload_resolution(local_class_function));
    SuspendFunctionAnalyzer local_class(local_class_context, local_class_function);
    assert(local_class.analyze() && local_class.get_suspend_points().empty());
    assert(local_class.get_local_variables().size() == 1);
    auto enclosing_call_ast = tooling::buildASTFromCodeWithArgs(
        "template<class T> int probe(T value) { struct Local { int read(T other) { "
        "return deferred_call(other); } }; return deferred_call(value); }", {"-std=c++20"});
    assert(enclosing_call_ast && !enclosing_call_ast->getDiagnostics().hasErrorOccurred());
    assert(SuspendFunctionAnalyzer::requires_overload_resolution(function(enclosing_call_ast->getASTContext())));

    auto class_liveness_ast = tooling::buildASTFromCodeWithArgs(declarations +
        "int probe(int value) { suspend(external_call(1)); struct Local { "
        "int read(int other) { int inner=7; return other+inner; } }; return value; }", {"-std=c++20"});
    assert(class_liveness_ast && !class_liveness_ast->getDiagnostics().hasErrorOccurred());
    auto& class_liveness_context = class_liveness_ast->getASTContext();
    SuspendFunctionAnalyzer class_liveness(class_liveness_context, function(class_liveness_context));
    assert(class_liveness.analyze() && class_liveness.get_suspend_points().size() == 1);
    assert(class_liveness.get_local_variables().size() == 1);
    assert(class_liveness.get_suspend_points()[0].live_variables.contains(function(class_liveness_context)->getParamDecl(0)));
    for (const auto* variable : class_liveness.get_suspend_points()[0].live_variables)
        assert(variable->getName() != "inner" && variable->getName() != "other");

    auto capture_ast = tooling::buildASTFromCodeWithArgs(
        "template<class T> int probe(T value) { auto closure = [init = deferred_call(value)] { "
        "int inner = 7; return inner; }; return 0; }", {"-std=c++20"});
    assert(capture_ast && !capture_ast->getDiagnostics().hasErrorOccurred());
    assert(SuspendFunctionAnalyzer::requires_overload_resolution(function(capture_ast->getASTContext())));

    auto suspend_capture_ast = tooling::buildASTFromCodeWithArgs(declarations +
        "int probe(int value) { auto closure = [init = suspend(external_call(1)), value] { "
        "int inner = 7; return value + inner; }; return 0; }", {"-std=c++20"});
    assert(suspend_capture_ast && !suspend_capture_ast->getDiagnostics().hasErrorOccurred());
    auto& capture_context = suspend_capture_ast->getASTContext();
    SuspendFunctionAnalyzer suspend_capture(capture_context, function(capture_context));
    assert(suspend_capture.analyze() && suspend_capture.get_suspend_points().size() == 1);
    assert(suspend_capture.get_local_variables().size() == 2);
    assert(suspend_capture.get_suspend_points()[0].live_variables.contains(function(capture_context)->getParamDecl(0)));
    for (const auto* variable : suspend_capture.get_suspend_points()[0].live_variables)
        assert(variable->getName() != "inner");

    // Reverse lexical order makes backward liveness cross more than 100 CFG
    // iterations. A capped analysis loses the parameter at the first suspension.
    std::string long_path = declarations +
        "int probe(int value) { suspend(external_call(1)); goto L299; L0: return value; ";
    for (int i = 1; i != 300; ++i)
        long_path += "L" + std::to_string(i) + ": goto L" + std::to_string(i - 1) + "; ";
    long_path += "}";
    auto ast = tooling::buildASTFromCodeWithArgs(long_path, {"-std=c++20"});
    assert(ast);
    auto& context = ast->getASTContext();
    auto* probe = function(context);
    assert(probe);
    SuspendFunctionAnalyzer analyzer(context, probe);
    assert(analyzer.analyze());
    assert(analyzer.get_suspend_points().size() == 1);
    assert(analyzer.get_suspend_points()[0].live_variables.contains(probe->getParamDecl(0)));
    std::cout << "Source-ordered suspension IDs, dependent callees and complete 300-block liveness verified\n";
}
