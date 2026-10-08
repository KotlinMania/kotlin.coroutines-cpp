// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:21-106,140-174
#pragma once
#include <llvm-c/Core.h>
#include <memory>
#include "LlvmParamType.hpp"

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
// LLVM function's signature, enriched with attributes.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:140-174
class LlvmFunctionSignature : public LlvmFunctionAttributeProvider {
public:
    // NOTE(port): Immutable type descriptors own their lists and borrow actual
    // LLVM types and attribute singletons. The callable retains this provider.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:143-148
    explicit LlvmFunctionSignature(LlvmRetType return_type, std::vector<LlvmParamType> parameter_types = {},
        bool is_vararg = false, std::vector<std::reference_wrapper<const LlvmFunctionAttribute>> function_attributes = {});
    ~LlvmFunctionSignature() override;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:144-144
    const LlvmRetType& return_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:145-145
    const std::vector<LlvmParamType>& parameter_types() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:146-146
    bool is_vararg() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:147-147
    const std::vector<std::reference_wrapper<const LlvmFunctionAttribute>>& function_attributes() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:150-150
    bool returns_object_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:152-154
    LLVMTypeRef llvm_function_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:156-164
    void add_call_site_attributes(LLVMValueRef call_site) override;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:166-173
    void add_function_attributes(LLVMValueRef function) override;
private:
    const LlvmRetType return_type_;
    const std::vector<LlvmParamType> parameter_types_;
    const bool is_vararg_;
    const std::vector<std::reference_wrapper<const LlvmFunctionAttribute>> function_attributes_;
    class LazyFunctionType;
    std::unique_ptr<LazyFunctionType> lazy_function_type_;
};
}
