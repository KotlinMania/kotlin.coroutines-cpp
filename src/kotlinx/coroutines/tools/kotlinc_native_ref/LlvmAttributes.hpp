// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:67-105
#pragma once
#include "LlvmUtils.hpp"
#include <string>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:67-69
class LlvmAttribute {
public:
    virtual ~LlvmAttribute() = default;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:68-68
    virtual LLVMAttributeKindId as_attribute_kind_id() const = 0;
};
// Preserve the source's class representation for attributes with parameters.
// NOTE(port): Source singleton attributes have static identities and cannot be
// copied. Attribute lists borrow those singleton objects.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:71-85
class LlvmParameterAttribute : public LlvmAttribute {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:75-77
    LLVMAttributeKindId as_attribute_kind_id() const override;
    LlvmParameterAttribute(const LlvmParameterAttribute&) = delete;
    LlvmParameterAttribute& operator=(const LlvmParameterAttribute&) = delete;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:83-83
    static const LlvmParameterAttribute SIGN_EXT;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:84-84
    static const LlvmParameterAttribute ZERO_EXT;
private:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:73-73
    explicit LlvmParameterAttribute(std::string llvm_attribute_name);
    const std::string llvm_attribute_name_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:87-105
class LlvmFunctionAttribute : public LlvmAttribute {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:89-91
    LLVMAttributeKindId as_attribute_kind_id() const override;
    LlvmFunctionAttribute(const LlvmFunctionAttribute&) = delete;
    LlvmFunctionAttribute& operator=(const LlvmFunctionAttribute&) = delete;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:97-97
    static const LlvmFunctionAttribute NO_UNWIND;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:98-98
    static const LlvmFunctionAttribute NO_RETURN;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:99-99
    static const LlvmFunctionAttribute NO_INLINE;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:100-100
    static const LlvmFunctionAttribute ALWAYS_INLINE;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:101-101
    static const LlvmFunctionAttribute SANITIZE_THREAD;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:102-102
    static const LlvmFunctionAttribute SSP;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:103-103
    static const LlvmFunctionAttribute SSP_STRONG;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:104-104
    static const LlvmFunctionAttribute SSP_REQ;
private:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:87-87
    explicit LlvmFunctionAttribute(std::string llvm_attribute_name);
    const std::string llvm_attribute_name_;
};
}
