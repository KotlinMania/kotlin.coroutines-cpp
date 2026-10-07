// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeFunctionReferenceLowering.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeFunctionReferenceLowering.kt:33-62
#include "NativeFunctionReferenceLowering.hpp"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include <algorithm>

namespace org::jetbrains::kotlin::backend::konan::lower {
// NOTE(port): Clang has already constructed the closure class and invoke method.
// This traversal applies the native postprocessing to those real declarations;
// it does not implement Kotlin reflection or named-reference wrapper generation.
class NativeFunctionReferenceLowering::Transformer
    : public clang::RecursiveASTVisitor<Transformer> {
public:
    explicit Transformer(NativeFunctionReferenceLowering& lowering) : lowering_(lowering) {}
    bool TraverseLambdaExpr(clang::LambdaExpr* expression) {
        if (!clang::RecursiveASTVisitor<Transformer>::TraverseLambdaExpr(expression)) return false;
        lowering_.postprocess_invoke(expression->getCallOperator(), expression->getLambdaClass());
        return true;
    }
private:
    NativeFunctionReferenceLowering& lowering_;
};

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeFunctionReferenceLowering.kt:58-62
// NOTE(port): The reference's property and copied function flag are declaration
// metadata. Clang AST import copies annotations with their owning declarations.
void NativeFunctionReferenceLowering::postprocess_invoke(
    clang::FunctionDecl* invoke_function, const clang::CXXRecordDecl* function_reference) {
    const bool restricted = std::any_of(function_reference->specific_attr_begin<clang::AnnotateAttr>(),
        function_reference->specific_attr_end<clang::AnnotateAttr>(), [](const auto* annotation) {
            return annotation->getAnnotation() == "kotlin.ir.isRestrictedSuspension";
        });
    if (restricted)
        invoke_function->addAttr(clang::AnnotateAttr::CreateImplicit(
            invoke_function->getASTContext(), "kotlin.ir.isRestrictedSuspensionInvokeMethod", nullptr, 0));
}

// NOTE(port): Clang closure traversal adapter for native postprocessing; this
// does not claim transliteration of AbstractFunctionReferenceLowering.lower.
void NativeFunctionReferenceLowering::lower(clang::Decl* ir_file) {
    Transformer transformer(*this);
    transformer.TraverseDecl(ir_file);
}
}
