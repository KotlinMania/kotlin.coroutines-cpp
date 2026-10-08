// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:21-92
#pragma once
#include <llvm-c/Core.h>
#include <memory>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// Add attributes to LLVM function declaration and its invocation.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:24-36
class LlvmFunctionAttributeProvider {
public:
    virtual ~LlvmFunctionAttributeProvider() = default;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:25-25
    virtual void add_call_site_attributes(LLVMValueRef call_site) = 0;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:27-27
    virtual void add_function_attributes(LLVMValueRef function) = 0;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:30-31
    static std::shared_ptr<LlvmFunctionAttributeProvider> make_empty();
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:33-34
    static std::shared_ptr<LlvmFunctionAttributeProvider> copy_from_external(LLVMValueRef external_function);
};
}
