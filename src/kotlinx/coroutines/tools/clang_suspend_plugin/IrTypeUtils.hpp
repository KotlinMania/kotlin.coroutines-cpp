// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/IrTypeUtils.kt
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/IrTypeUtils.kt:236-250
#pragma once
#include <set>
namespace clang { class CXXRecordDecl; }
namespace org::jetbrains::kotlin::ir::util {
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/IrTypeUtils.kt:246-250
std::set<const clang::CXXRecordDecl*> get_all_superclasses(const clang::CXXRecordDecl* ir_class);
}
