// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:10-18
#include "LlvmParamType.hpp"
#include <utility>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:13-13
LlvmParamType::LlvmParamType(LLVMTypeRef llvm_type, std::vector<std::reference_wrapper<const LlvmParameterAttribute>> attributes)
    : llvm_type_(llvm_type), attributes_(std::move(attributes)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:13-13
LLVMTypeRef LlvmParamType::llvm_type() const { return llvm_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:13-13
const std::vector<std::reference_wrapper<const LlvmParameterAttribute>>& LlvmParamType::attributes() const { return attributes_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:18-18
LlvmRetType::LlvmRetType(LLVMTypeRef llvm_type, bool is_object_type) : LlvmRetType(llvm_type, {}, is_object_type) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:18-18
LlvmRetType::LlvmRetType(LLVMTypeRef llvm_type, std::vector<std::reference_wrapper<const LlvmParameterAttribute>> attributes, bool is_object_type)
    : llvm_type_(llvm_type), attributes_(std::move(attributes)), is_object_type_(is_object_type) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:18-18
LLVMTypeRef LlvmRetType::llvm_type() const { return llvm_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:18-18
const std::vector<std::reference_wrapper<const LlvmParameterAttribute>>& LlvmRetType::attributes() const { return attributes_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:18-18
bool LlvmRetType::is_object_type() const { return is_object_type_; }
}
