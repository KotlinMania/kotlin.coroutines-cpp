// port-lint: source compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/UpgradeCallableReferences.kt
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/UpgradeCallableReferences.kt:32-89
#pragma once
namespace clang { class Decl; }
namespace org::jetbrains::kotlin::backend::common::lower {
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/UpgradeCallableReferences.kt:32-42
class UpgradeCallableReferences {
public:
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/UpgradeCallableReferences.kt:37-39
    void lower(clang::Decl* ir_file);
private:
    class UpgradeTransformer;
};
}
