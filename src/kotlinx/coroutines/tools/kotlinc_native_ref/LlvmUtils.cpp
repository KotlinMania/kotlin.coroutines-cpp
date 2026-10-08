// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:18-43
#include "LlvmUtils.hpp"
#include <cassert>

namespace org::jetbrains::kotlin::backend::konan::llvm {
namespace {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:32-38
class ConstantPointer final : public ConstPointer {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:32-38
    explicit ConstantPointer(LLVMValueRef value) : value_(value) { assert(LLVMIsConstant(value) == 1); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:37-37
    LLVMValueRef llvm() const override { return value_; }
private:
    const LLVMValueRef value_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:40-43
class ConstGetElementPtr final : public ConstPointer {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:40-41
    ConstGetElementPtr(LLVMContextRef context, LLVMTypeRef pointee_type, const ConstPointer& pointer, int index) {
        LLVMValueRef indices[] = {LLVMConstInt(LLVMInt32TypeInContext(context), 0, false),
            LLVMConstInt(LLVMInt32TypeInContext(context), static_cast<unsigned long long>(index), true)};
        value_ = LLVMConstInBoundsGEP2(pointee_type, pointer.llvm(), indices, 2);
    }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:41-41
    LLVMValueRef llvm() const override { return value_; }
private:
    LLVMValueRef value_;
};
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:24-25
LLVMTypeRef llvm_type(const ConstValue& value) { return LLVMTypeOf(value.llvm()); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:28-29
std::shared_ptr<ConstPointer> ConstPointer::get_element_ptr(LLVMContextRef context, LLVMTypeRef pointee_type, int index) const {
    return std::make_shared<ConstGetElementPtr>(context, pointee_type, *this, index);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:32-38
std::shared_ptr<ConstPointer> const_pointer(LLVMValueRef value) { return std::make_shared<ConstantPointer>(value); }
}
