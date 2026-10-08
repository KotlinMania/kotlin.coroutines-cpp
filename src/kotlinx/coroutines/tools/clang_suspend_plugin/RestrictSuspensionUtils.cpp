// port-lint: source compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/RestrictSuspensionUtils.kt
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/RestrictSuspensionUtils.kt:16-26
#include "RestrictSuspensionUtils.hpp"
#include "IrTypeUtils.hpp"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/DeclCXX.h"
#include <algorithm>

namespace org::jetbrains::kotlin::backend::common {
namespace {
// NOTE(port): Clang annotations carry Kotlin class annotations and the explicit
// IrParameterKind.ExtensionReceiver role. Parameter position is not a role.
bool has_annotation(const clang::Decl* declaration, llvm::StringRef name) {
    return std::any_of(declaration->specific_attr_begin<clang::AnnotateAttr>(),
                       declaration->specific_attr_end<clang::AnnotateAttr>(),
                       [name](const auto* annotation) { return annotation->getAnnotation() == name; });
}

}

// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/RestrictSuspensionUtils.kt:16-20
bool is_restricted_suspension(const clang::CXXRecordDecl* ir_class) {
    if (!ir_class) return false;
    const auto has_restriction = [](const clang::CXXRecordDecl* declaration) {
        // NOTE(port): Clang keeps annotations on their individual redeclarations.
        return std::any_of(declaration->redecls_begin(), declaration->redecls_end(),
            [](const auto* redeclaration) {
                return has_annotation(redeclaration, "kotlin.coroutines.RestrictsSuspension");
            });
    };
    if (has_restriction(ir_class)) return true;
    const auto superclasses = org::jetbrains::kotlin::ir::util::get_all_superclasses(ir_class);
    return std::any_of(superclasses.begin(), superclasses.end(), has_restriction);
}

// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/RestrictSuspensionUtils.kt:22-26
bool is_restricted_suspension_function(const clang::FunctionDecl* function) {
    // Kotlin's unlabelled return inside inline any exits this function.
    for (const auto* parameter : function->parameters()) {
        auto type = parameter->getType().getNonReferenceType();
        // NOTE(port): C++ pointer/reference receivers represent Kotlin class values.
        if (type->isPointerType()) type = type->getPointeeType();
        return has_annotation(parameter, "kotlin.ir.ExtensionReceiver") &&
               is_restricted_suspension(type->getAsCXXRecordDecl());
    }
    return false;
}
}
