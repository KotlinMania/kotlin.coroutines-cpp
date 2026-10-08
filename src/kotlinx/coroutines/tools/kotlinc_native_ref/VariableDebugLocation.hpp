// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/VariableManager.kt:150-150
#pragma once
#include <llvm-c/Core.h>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// NOTE(port): These are actual compiler-owned LLVM metadata references.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/VariableManager.kt:150-150
struct VariableDebugLocation {
    const LLVMMetadataRef local_variable;
    const LLVMMetadataRef location;
    const LLVMMetadataRef file;
    const int line;
};
}
