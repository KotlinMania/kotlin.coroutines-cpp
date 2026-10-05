#include "SuspendFunctionAnalyzer.hpp"
#include "clang/Tooling/Tooling.h"
#include "clang/Basic/SourceManager.h"
#include <cassert>
#include <iostream>

using namespace clang;
using namespace kotlinx::suspend;

static FunctionDecl* function(ASTContext& context) {
    for (auto* declaration : context.getTranslationUnitDecl()->decls())
        if (auto* candidate = dyn_cast<FunctionDecl>(declaration))
            if (candidate->getName() == "probe") return candidate;
    return nullptr;
}

int main() {
    const std::string declarations = "void* suspend(void*); void* external_call(int);\n";
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
    std::cout << "Source-ordered suspension IDs and complete 300-block liveness passed\n";
}
