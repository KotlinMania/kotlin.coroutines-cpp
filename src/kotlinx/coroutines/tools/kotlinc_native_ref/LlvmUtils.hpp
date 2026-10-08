// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:18-43
#pragma once
#include <llvm-c/Core.h>
#include <memory>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// Represents the value which can be emitted as bitcode const value.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:18-22
class ConstValue {
public:
    virtual ~ConstValue() = default;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:21-21
    virtual LLVMValueRef llvm() const = 0;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:24-25
LLVMTypeRef llvm_type(const ConstValue& value);
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:27-30
class ConstPointer : public ConstValue {
public:
    // NOTE(port): Supply the owning LLVM context directly for the source
    // CodegenLlvmHelpers.int32 operands. Constant descriptors own no LLVM value.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:28-29
    virtual std::shared_ptr<ConstPointer> get_element_ptr(LLVMContextRef context, LLVMTypeRef pointee_type, int index) const;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:32-38
std::shared_ptr<ConstPointer> const_pointer(LLVMValueRef value);
}
