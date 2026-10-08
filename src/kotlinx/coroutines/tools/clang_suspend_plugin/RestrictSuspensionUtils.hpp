// port-lint: source compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/RestrictSuspensionUtils.kt
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/RestrictSuspensionUtils.kt:16-26
#pragma once

namespace clang { class CXXRecordDecl; class FunctionDecl; }

namespace org::jetbrains::kotlin::backend::common {
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/RestrictSuspensionUtils.kt:16-20
bool is_restricted_suspension(const clang::CXXRecordDecl* ir_class);
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/RestrictSuspensionUtils.kt:22-26
bool is_restricted_suspension_function(const clang::FunctionDecl* function);
}
