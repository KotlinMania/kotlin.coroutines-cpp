// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeFunctionReferenceLowering.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeFunctionReferenceLowering.kt:33-62
#pragma once
namespace clang { class Decl; class FunctionDecl; class CXXRecordDecl; }
namespace org::jetbrains::kotlin::backend::konan::lower {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeFunctionReferenceLowering.kt:38-62
class NativeFunctionReferenceLowering {
public:
    // NOTE(port): Clang closure traversal adapter. Kotlin creates these classes
    // in AbstractFunctionReferenceLowering.kt:84-133 and postprocesses at :187-191.
    void lower(clang::Decl* ir_file);
private:
    class Transformer;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeFunctionReferenceLowering.kt:58-62
    void postprocess_invoke(clang::FunctionDecl* invoke_function, const clang::CXXRecordDecl* function_reference);
};
}
