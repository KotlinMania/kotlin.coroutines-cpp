// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:11-103
#pragma once
#include "LlvmFunctionPrototype.hpp"
#include "LlvmUtils.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// NOTE(port): LLVM values/types stay borrowed from the compilation context;
// the explicitly shared provider retains the source object's attribute policy.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:11-36
class LlvmCallable {
public:
    virtual ~LlvmCallable();
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:12-12
    LLVMTypeRef function_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:13-13
    bool returns_object_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:18-18
    const std::optional<std::string>& name() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:19-19
    LLVMTypeRef return_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:20-20
    int num_params() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:21-21
    bool is_constant() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:23-26
    LLVMValueRef build_call(LLVMBuilderRef builder, const std::vector<LLVMValueRef>& args, const std::string& name = "") const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:28-31
    LLVMValueRef build_invoke(LLVMBuilderRef builder, const std::vector<LLVMValueRef>& args,
        LLVMBasicBlockRef success, LLVMBasicBlockRef catch_block, const std::string& name = "") const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:33-33
    std::shared_ptr<ConstPointer> to_const_pointer() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:35-35
    LLVMValueRef as_callback() const;
protected:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:11-16
    LlvmCallable(LLVMTypeRef function_type, bool returns_object_type, LLVMValueRef llvm_value,
        std::shared_ptr<LlvmFunctionAttributeProvider> attribute_provider);
    const LLVMValueRef llvm_value_;
    const std::shared_ptr<LlvmFunctionAttributeProvider> attribute_provider_;
private:
    const LLVMTypeRef function_type_;
    const bool returns_object_type_;
    class LazyProperties;
    std::unique_ptr<LazyProperties> lazy_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:38-43
class LlvmFunctionPointer final : public LlvmCallable {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:38-43
    LlvmFunctionPointer(LLVMTypeRef function_type, bool returns_object_type, LLVMValueRef llvm_value,
        std::shared_ptr<LlvmFunctionAttributeProvider> attribute_provider);
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:48-103
class LlvmFunction : public LlvmCallable {
public:
    ~LlvmFunction() override;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:55-60
    bool is_no_unwind() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:62-67
    LLVMValueRef param(int index) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:69-70
    LLVMValueRef build_landingpad(LLVMBuilderRef builder, LLVMTypeRef landingpad_type,
        int num_clauses, const std::string& name = "") const;
    class Declaration;
    class Definition;
protected:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:48-53
    LlvmFunction(LLVMTypeRef function_type, bool returns_object_type, LLVMValueRef llvm_value,
        std::shared_ptr<LlvmFunctionAttributeProvider> attribute_provider);
private:
    class LazyProperties;
    std::unique_ptr<LazyProperties> lazy_;
};
// Function prototypes (declarations in LLVM terms) do not belong to a specific module.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:72-80
class LlvmFunction::Declaration final : public LlvmFunction {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:75-80
    Declaration(LLVMTypeRef function_type, bool returns_object_type, LLVMValueRef llvm_value,
        std::shared_ptr<LlvmFunctionAttributeProvider> attribute_provider);
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:85-103
class LlvmFunction::Definition final : public LlvmFunction {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:85-90
    Definition(LLVMTypeRef function_type, bool returns_object_type, LLVMValueRef llvm_value,
        std::shared_ptr<LlvmFunctionAttributeProvider> attribute_provider);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:95-96
    LLVMBasicBlockRef add_basic_block(LLVMContextRef context, const std::string& name = "") const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:98-99
    LLVMValueRef block_address(LLVMBasicBlockRef label) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:101-103
    void add_debug_info_subprogram(LLVMMetadataRef subprogram) const;
};
}
