// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2884-2887
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:390-393
#pragma once
#include <llvm-c/Core.h>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// NOTE(port): Scope metadata and inline locations belong to the owning compiler
// context. This descriptor borrows them for that compilation's lifetime.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2884-2887
class LocationInfo {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2884-2887
    LocationInfo(LLVMMetadataRef scope, int line, int column, LocationInfo* inlined_at = nullptr);
    const LLVMMetadataRef scope;
    const int line;
    const int column;
    LocationInfo* const inlined_at;
};

// There are cases when an end position is unnecessary or meaningless.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:390-393
struct LocationInfoRange {
    LocationInfo* start;
    LocationInfo* end;
};
}
