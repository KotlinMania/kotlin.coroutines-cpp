// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:10-18
#pragma once
#include "LlvmAttributes.hpp"
#include <functional>
#include <vector>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// LLVM function's parameter type with its attributes.
// NOTE(port): These immutable descriptors retain actual LLVM type identity and
// borrow source attribute singletons; they own their attribute-list containers.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:10-13
class LlvmParamType {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:13-13
    explicit LlvmParamType(LLVMTypeRef llvm_type,
        std::vector<std::reference_wrapper<const LlvmParameterAttribute>> attributes = {});
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:13-13
    LLVMTypeRef llvm_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:13-13
    const std::vector<std::reference_wrapper<const LlvmParameterAttribute>>& attributes() const;
private:
    const LLVMTypeRef llvm_type_;
    const std::vector<std::reference_wrapper<const LlvmParameterAttribute>> attributes_;
};
// LLVM function's return type with its attributes.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:15-18
class LlvmRetType {
public:
    // NOTE(port): Overloads preserve Kotlin's omitted attribute-list argument
    // before the required object-return metadata.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:18-18
    LlvmRetType(LLVMTypeRef llvm_type, bool is_object_type);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:18-18
    LlvmRetType(LLVMTypeRef llvm_type,
        std::vector<std::reference_wrapper<const LlvmParameterAttribute>> attributes, bool is_object_type);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:18-18
    LLVMTypeRef llvm_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:18-18
    const std::vector<std::reference_wrapper<const LlvmParameterAttribute>>& attributes() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmParamType.kt:18-18
    bool is_object_type() const;
private:
    const LLVMTypeRef llvm_type_;
    const std::vector<std::reference_wrapper<const LlvmParameterAttribute>> attributes_;
    const bool is_object_type_;
};
}
