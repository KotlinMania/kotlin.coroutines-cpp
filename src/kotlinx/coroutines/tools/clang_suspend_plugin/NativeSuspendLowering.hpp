// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractSuspendFunctionsLowering.kt:55-234
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:119-335
#pragma once
#include <string>
namespace clang { class ASTContext; class FunctionDecl; }
namespace org::jetbrains::kotlin::backend::konan::lower {
// Create retained frame fields and native normal/resume regions. The LLVM pass
// consumes the exact label-field and blockaddress operands during compilation.
// NOTE(port): Clang-facing adapter, not a translated Kotlin public API.
std::string build_coroutine(clang::ASTContext& context, clang::FunctionDecl* function);
}
