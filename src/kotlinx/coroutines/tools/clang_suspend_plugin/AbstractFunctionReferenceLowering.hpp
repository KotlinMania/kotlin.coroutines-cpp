// port-lint: source compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractFunctionReferenceLowering.kt
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractFunctionReferenceLowering.kt:135-295
#pragma once
#include <string>
namespace clang { class ASTContext; class LambdaExpr; }
namespace org::jetbrains::kotlin::backend::common::lower {
// NOTE(port): Clang lambda carrier for callable class construction. Kotlin
// reflection, SAM interfaces and named references require their own lowering.
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractFunctionReferenceLowering.kt:83-295
class AbstractFunctionReferenceLowering {
public:
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractFunctionReferenceLowering.kt:135-194
    std::string build_class(clang::ASTContext& context, const clang::LambdaExpr* function_reference,
                            const std::string& name) const;
private:
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractFunctionReferenceLowering.kt:210-295
    std::string build_invoke_method(clang::ASTContext& context,
                                    const clang::LambdaExpr* function_reference) const;
};
}
