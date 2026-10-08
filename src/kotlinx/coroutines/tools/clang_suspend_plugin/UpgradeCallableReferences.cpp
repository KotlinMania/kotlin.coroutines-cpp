// port-lint: source compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/UpgradeCallableReferences.kt
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/UpgradeCallableReferences.kt:37-89
#include "UpgradeCallableReferences.hpp"
#include "RestrictSuspensionUtils.hpp"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/ADT/STLExtras.h"
#include <algorithm>
#include <stdexcept>

namespace org::jetbrains::kotlin::backend::common::lower {
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/UpgradeCallableReferences.kt:61-89
// NOTE(port): Clang already creates the closure record, capture fields and invoke
// method. Its record carries the rich reference's restricted-suspension property;
// parameter annotations carry role/origin metadata independently of C++ types.
class UpgradeCallableReferences::UpgradeTransformer
    : public clang::RecursiveASTVisitor<UpgradeTransformer> {
public:
    // NOTE(port): Clang's traversal callback preserves transformChildren before
    // visit_function_expression, including nested reference expressions.
    bool TraverseLambdaExpr(clang::LambdaExpr* expression) {
        if (!clang::RecursiveASTVisitor<UpgradeTransformer>::TraverseLambdaExpr(expression)) return false;
        visit_function_expression(expression);
        return true;
    }
private:
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/UpgradeCallableReferences.kt:62-73
    void flatten_parameters(clang::FunctionDecl* function, bool is_lambda) {
        // NOTE(port): The Clang dispatch receiver is implicit, outside parameters.
        // Its ordinary parameter role is represented by absence of receiver metadata.
        for (auto* parameter : function->parameters()) {
            auto attributes = parameter->getAttrs();
            const auto has_annotation = [&](llvm::StringRef name) {
                return std::any_of(attributes.begin(), attributes.end(), [name](const auto* attribute) {
                    const auto* annotation = llvm::dyn_cast<clang::AnnotateAttr>(attribute);
                    return annotation && annotation->getAnnotation() == name;
                });
            };
            if (has_annotation("kotlin.ir.DispatchReceiver"))
                throw std::invalid_argument("No dispatch receiver allowed in wrappers");
            const bool extension_receiver = std::any_of(attributes.begin(), attributes.end(), [](const auto* attribute) {
                const auto* annotation = llvm::dyn_cast<clang::AnnotateAttr>(attribute);
                return annotation && annotation->getAnnotation() == "kotlin.ir.ExtensionReceiver";
            });
            if (extension_receiver && is_lambda)
                attributes.push_back(clang::AnnotateAttr::CreateImplicit(function->getASTContext(), "kotlin.ir.LAMBDA_EXTENSION_RECEIVER", nullptr, 0));
            const bool defined_origin = has_annotation("kotlin.ir.Context") && has_annotation("kotlin.ir.UNDERSCORE_PARAMETER");
            if (defined_origin)
                attributes.push_back(clang::AnnotateAttr::CreateImplicit(function->getASTContext(), "kotlin.ir.DEFINED", nullptr, 0));
            llvm::erase_if(attributes, [](const auto* attribute) {
                const auto* annotation = llvm::dyn_cast<clang::AnnotateAttr>(attribute);
                return annotation && (annotation->getAnnotation() == "kotlin.ir.ExtensionReceiver" ||
                                      annotation->getAnnotation() == "kotlin.ir.Context");
            });
            if (defined_origin)
                llvm::erase_if(attributes, [](const auto* attribute) {
                    const auto* annotation = llvm::dyn_cast<clang::AnnotateAttr>(attribute);
                    return annotation && annotation->getAnnotation() == "kotlin.ir.UNDERSCORE_PARAMETER";
                });
            parameter->setAttrs(attributes);
        }
    }

    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/UpgradeCallableReferences.kt:75-89
    void visit_function_expression(clang::LambdaExpr* expression) {
        const bool restricted = is_restricted_suspension_function(expression->getCallOperator());
        flatten_parameters(expression->getCallOperator(), true);
        if (restricted)
            expression->getLambdaClass()->addAttr(clang::AnnotateAttr::CreateImplicit(
                expression->getLambdaClass()->getASTContext(), "kotlin.ir.isRestrictedSuspension", nullptr, 0));
    }
};

// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/UpgradeCallableReferences.kt:37-39
void UpgradeCallableReferences::lower(clang::Decl* ir_file) {
    UpgradeTransformer transformer;
    transformer.TraverseDecl(ir_file);
}
}
