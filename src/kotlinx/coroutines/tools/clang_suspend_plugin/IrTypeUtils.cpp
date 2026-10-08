// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/IrTypeUtils.kt
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/IrTypeUtils.kt:236-250
#include "IrTypeUtils.hpp"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"

namespace org::jetbrains::kotlin::ir::util {
namespace {
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/IrTypeUtils.kt:236-244
// NOTE(port): A Clang record definition owns its superclass list; canonical
// declarations supply the same identity across forward declarations.
void collect_all_superclasses(const clang::CXXRecordDecl* ir_class,
                             std::set<const clang::CXXRecordDecl*>& set) {
    const auto* definition = ir_class->getDefinition();
    if (!definition) return;
    for (const auto& super_type : definition->bases()) {
        const auto* super_class = super_type.getType()->getAsCXXRecordDecl();
        if (!super_class) continue;
        super_class = super_class->getCanonicalDecl();
        if (set.insert(super_class).second) collect_all_superclasses(super_class, set);
    }
}
}

// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/IrTypeUtils.kt:246-250
std::set<const clang::CXXRecordDecl*> get_all_superclasses(const clang::CXXRecordDecl* ir_class) {
    std::set<const clang::CXXRecordDecl*> result;
    collect_all_superclasses(ir_class, result);
    return result;
}
}
