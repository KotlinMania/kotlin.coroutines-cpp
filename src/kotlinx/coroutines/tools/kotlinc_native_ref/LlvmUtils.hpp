// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:18-43,118-134,235-243,307-351,358-359
#pragma once
#include <llvm-c/Core.h>
#include <memory>
#include <cstdint>
#include <string>
#include <vector>

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
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:319-319
struct LLVMAttributeKindId { const int value; };
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:311-317
LLVMAttributeKindId get_llvm_attribute_kind_id(const std::string& attribute_name);
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:346-347
LLVMAttributeRef create_llvm_enum_attribute(LLVMContextRef context, LLVMAttributeKindId attribute_kind_id, std::int64_t value = 0);
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:349-351
void add_llvm_function_attribute(LLVMValueRef function, LLVMAttributeRef attribute);
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:338-341
void add_llvm_function_enum_attribute(LLVMValueRef function, LLVMAttributeKindId attribute_kind_id, std::int64_t value = 0);
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:307-309
bool is_function_no_unwind(LLVMValueRef function);
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:235-243
LLVMTypeRef function_type(LLVMTypeRef return_type, bool is_vararg = false, const std::vector<LLVMTypeRef>& param_types = {});
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:321-323
void set_function_no_unwind(LLVMValueRef function);
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:325-327
void set_function_no_return(LLVMValueRef function);
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:329-331
void set_function_no_inline(LLVMValueRef function);
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:333-335
void set_function_always_inline(LLVMValueRef function);
class LlvmAttribute;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:343-344
void add_llvm_function_enum_attribute(LLVMValueRef function, const LlvmAttribute& attribute, std::int64_t value = 0);
// NOTE(port): Concrete overload for Kotlin's omitted isVarArg named argument.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:235-243
LLVMTypeRef function_type(LLVMTypeRef return_type, const std::vector<LLVMTypeRef>& param_types);
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:118-125
std::string get_as_c_string(LLVMValueRef value);
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:358-359
std::vector<LLVMValueRef> get_operands(LLVMValueRef value);

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:128-130
LLVMTypeRef get_global_function_type(LLVMValueRef function);
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:132-134
LLVMTypeRef get_global_type(LLVMValueRef global);

}
