// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:18-110,118-134,235-243,251-254,307-351,358-359
#include "LlvmUtils.hpp"
#include <cassert>
#include <stdexcept>
#include <utility>
#include "LlvmAttributes.hpp"
#include "LlvmParamType.hpp"

namespace org::jetbrains::kotlin::backend::konan::llvm {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:94-95
LLVMTypeRef type_info_type(const RuntimeAware& runtime_aware) { return runtime_aware.runtime().type_info_type(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:96-97
LLVMTypeRef obj_header_type(const RuntimeAware& runtime_aware) { return runtime_aware.runtime().obj_header_type(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:98-99
LlvmRetType obj_header_ptr_return_type(const RuntimeAware& runtime_aware) { return LlvmRetType(runtime_aware.runtime().pointer_type(), true); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:100-101
LLVMTypeRef array_header_type(const RuntimeAware& runtime_aware) { return runtime_aware.runtime().array_header_type(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:103-105
LLVMValueRef nothing_fake_value(const RuntimeAware& runtime_aware) { return LLVMGetUndef(runtime_aware.runtime().pointer_type()); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:107-110
std::int64_t extract_const_unsigned_int(LLVMValueRef value) {
    assert(LLVMIsConstant(value) != 0);
    return static_cast<std::int64_t>(LLVMConstIntGetZExtValue(value));
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:251-254
std::string to_type_string(LLVMTypeRef type) {
    if (!type) return "<null type>";
    // NOTE(port): Own the LLVM print buffer through result construction and failure.
    const std::unique_ptr<char, decltype(&LLVMDisposeMessage)> message(LLVMPrintTypeToString(type), LLVMDisposeMessage);
    return std::string(message.get());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:45-54
ConstArray::ConstArray(LLVMTypeRef element_type, std::shared_ptr<std::vector<std::shared_ptr<const ConstValue>>> elements)
    : elements_(std::move(elements)), llvm_([this, element_type] {
        // NOTE(port): Source assertions follow the C++ NDEBUG policy; failures become compiler exceptions.
#ifndef NDEBUG
        for (const auto& element : *elements_) {
            const auto actual_type = llvm_type(*element);
            if (actual_type != element_type)
                throw std::logic_error("Expected element type: " + to_type_string(element_type) +
                    ", actual: " + to_type_string(actual_type));
        }
#endif
        std::vector<LLVMValueRef> values;
        values.reserve(elements_->size());
        for (const auto& element : *elements_) values.push_back(element->llvm());
        return LLVMConstArray(element_type, values.data(), static_cast<unsigned>(values.size()));
    }()) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:45-45
const std::shared_ptr<std::vector<std::shared_ptr<const ConstValue>>>& ConstArray::elements() const { return elements_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:53-53
LLVMValueRef ConstArray::llvm() const { return llvm_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:82-83
Zero::Zero(LLVMTypeRef type) : type_(type), llvm_(LLVMConstNull(type)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:82-82
LLVMTypeRef Zero::type() const { return type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:83-83
LLVMValueRef Zero::llvm() const { return llvm_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:56-82
Struct::Struct(LLVMTypeRef type, std::shared_ptr<std::vector<std::shared_ptr<const ConstValue>>> elements)
    : type_(type), elements_(std::move(elements)), llvm_([this] {
        std::vector<LLVMValueRef> values;
        values.reserve(elements_->size());
        for (std::size_t index = 0; index < elements_->size(); ++index) {
            const auto expected_type = LLVMStructGetTypeAtIndex(type_, index);
            const auto& element = (*elements_)[index];
            const auto value = element ? element->llvm() : LLVMConstNull(expected_type);
            // NOTE(port): Source assertions follow the C++ NDEBUG policy; failures become compiler exceptions.
#ifndef NDEBUG
            if (LLVMTypeOf(value) != expected_type)
                throw std::logic_error("Unexpected type at " + std::to_string(index) + ": expected " +
                    to_type_string(expected_type) + ", got " + to_type_string(LLVMTypeOf(value)) +
                    " in " + to_type_string(type_));
#endif
            values.push_back(value);
        }
        return LLVMConstNamedStruct(type_, values.data(), static_cast<unsigned>(values.size()));
    }()) {
#ifndef NDEBUG
    if (elements_->size() != LLVMCountStructElementTypes(type_))
        throw std::logic_error("Should have " + std::to_string(LLVMCountStructElementTypes(type_)) +
            " elements, have " + std::to_string(elements_->size()) + " for type " + to_type_string(type_));
#endif
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:58-58
Struct::Struct(LLVMTypeRef type, const std::vector<std::shared_ptr<const ConstValue>>& elements)
    : Struct(type, std::make_shared<std::vector<std::shared_ptr<const ConstValue>>>(elements)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:56-56
LLVMTypeRef Struct::type() const { return type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:56-56
const std::shared_ptr<std::vector<std::shared_ptr<const ConstValue>>>& Struct::elements() const { return elements_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:60-73
LLVMValueRef Struct::llvm() const { return llvm_; }

namespace {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:86-92
class ConstantValue final : public ConstValue {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:86-89
    explicit ConstantValue(LLVMValueRef value) : value_(value) { assert(LLVMIsConstant(value) == 1); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:91-91
    LLVMValueRef llvm() const override { return value_; }
private:
    const LLVMValueRef value_;
};
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
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:86-92
std::shared_ptr<ConstValue> const_value(LLVMValueRef value) { return std::make_shared<ConstantValue>(value); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:32-38
std::shared_ptr<ConstPointer> const_pointer(LLVMValueRef value) { return std::make_shared<ConstantPointer>(value); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:311-317
LLVMAttributeKindId get_llvm_attribute_kind_id(const std::string& attribute_name) {
    const auto kind = LLVMGetEnumAttributeKindForName(attribute_name.c_str(), attribute_name.size());
    if (kind == 0) throw std::runtime_error("Unable to find '" + attribute_name + "' attribute kind id");
    return LLVMAttributeKindId{static_cast<int>(kind)};
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:346-347
LLVMAttributeRef create_llvm_enum_attribute(LLVMContextRef context, LLVMAttributeKindId attribute_kind_id, std::int64_t value) {
    return LLVMCreateEnumAttribute(context, attribute_kind_id.value, static_cast<std::uint64_t>(value));
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:349-351
void add_llvm_function_attribute(LLVMValueRef function, LLVMAttributeRef attribute) {
    LLVMAddAttributeAtIndex(function, LLVMAttributeFunctionIndex, attribute);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:338-341
void add_llvm_function_enum_attribute(LLVMValueRef function, LLVMAttributeKindId attribute_kind_id, std::int64_t value) {
    const auto attribute = create_llvm_enum_attribute(LLVMGetTypeContext(LLVMTypeOf(function)), attribute_kind_id, value);
    add_llvm_function_attribute(function, attribute);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:307-309
bool is_function_no_unwind(LLVMValueRef function) {
    return LLVMGetEnumAttributeAtIndex(function, LLVMAttributeFunctionIndex,
        LlvmFunctionAttribute::NO_UNWIND.as_attribute_kind_id().value) != nullptr;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:235-243
LLVMTypeRef function_type(LLVMTypeRef return_type, bool is_vararg, const std::vector<LLVMTypeRef>& param_types) {
    // NOTE(port): Copy the borrowed operand handles for LLVM-C's mutable array.
    auto params = param_types;
    return LLVMFunctionType(return_type, params.data(), params.size(), is_vararg ? 1 : 0);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:321-323
void set_function_no_unwind(LLVMValueRef function) {
    add_llvm_function_enum_attribute(function, LlvmFunctionAttribute::NO_UNWIND);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:325-327
void set_function_no_return(LLVMValueRef function) {
    add_llvm_function_enum_attribute(function, LlvmFunctionAttribute::NO_RETURN);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:329-331
void set_function_no_inline(LLVMValueRef function) {
    add_llvm_function_enum_attribute(function, LlvmFunctionAttribute::NO_INLINE);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:333-335
void set_function_always_inline(LLVMValueRef function) {
    add_llvm_function_enum_attribute(function, LlvmFunctionAttribute::ALWAYS_INLINE);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:343-344
void add_llvm_function_enum_attribute(LLVMValueRef function, const LlvmAttribute& attribute, std::int64_t value) {
    add_llvm_function_enum_attribute(function, attribute.as_attribute_kind_id(), value);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:235-243
LLVMTypeRef function_type(LLVMTypeRef return_type, const std::vector<LLVMTypeRef>& param_types) {
    return function_type(return_type, false, param_types);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:118-125
std::string get_as_c_string(LLVMValueRef value) {
    std::size_t length = 0;
    const auto* data = LLVMGetAsString(value, &length);
    if (!data || length < 1 || data[length - 1] != '\0')
        throw std::invalid_argument("Expected null-terminated string from llvm");
    return std::string(data);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:358-359
std::vector<LLVMValueRef> get_operands(LLVMValueRef value) {
    std::vector<LLVMValueRef> operands;
    const auto count = LLVMGetNumOperands(value);
    operands.reserve(count);
    for (int index = 0; index < count; ++index) operands.push_back(LLVMGetOperand(value, index));
    return operands;
}

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:128-130
LLVMTypeRef get_global_function_type(LLVMValueRef function) { return get_global_type(function); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:132-134
LLVMTypeRef get_global_type(LLVMValueRef global) { return LLVMGlobalGetValueType(global); }

}
