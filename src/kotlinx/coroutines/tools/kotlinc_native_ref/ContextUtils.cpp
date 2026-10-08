// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:21-130,264-365,411-441,445-571,573-579
#include "ContextUtils.hpp"
#include "LlvmUtils.hpp"
#include <mutex>
#include <cassert>
#include <stdexcept>
#include <utility>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:264-265
ConstInt1::ConstInt1(const CodegenLlvmHelpers& llvm, bool value)
    : value_(value), llvm_(LLVMConstInt(llvm.int1_type(), value ? 1 : 0, 1)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:264-264
bool ConstInt1::value() const { return value_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:265-265
LLVMValueRef ConstInt1::llvm() const { return llvm_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:268-269
ConstInt8::ConstInt8(const CodegenLlvmHelpers& llvm, std::int8_t value)
    : value_(value), llvm_(LLVMConstInt(llvm.int8_type(), static_cast<std::uint64_t>(static_cast<std::int64_t>(value)), 1)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:268-268
std::int8_t ConstInt8::value() const { return value_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:269-269
LLVMValueRef ConstInt8::llvm() const { return llvm_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:272-273
ConstUInt8::ConstUInt8(const CodegenLlvmHelpers& llvm, std::uint8_t value)
    : value_(value), llvm_(LLVMConstInt(llvm.int8_type(), static_cast<std::uint64_t>(static_cast<std::int64_t>(value)), 1)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:272-272
std::uint8_t ConstUInt8::value() const { return value_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:273-273
LLVMValueRef ConstUInt8::llvm() const { return llvm_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:276-277
ConstInt16::ConstInt16(const CodegenLlvmHelpers& llvm, std::int16_t value)
    : value_(value), llvm_(LLVMConstInt(llvm.int16_type(), static_cast<std::uint64_t>(static_cast<std::int64_t>(value)), 1)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:276-276
std::int16_t ConstInt16::value() const { return value_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:277-277
LLVMValueRef ConstInt16::llvm() const { return llvm_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:280-281
ConstChar16::ConstChar16(const CodegenLlvmHelpers& llvm, char16_t value)
    : value_(value), llvm_(LLVMConstInt(llvm.int16_type(), static_cast<std::uint64_t>(static_cast<std::int64_t>(value)), 1)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:280-280
char16_t ConstChar16::value() const { return value_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:281-281
LLVMValueRef ConstChar16::llvm() const { return llvm_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:284-285
ConstInt32::ConstInt32(const CodegenLlvmHelpers& llvm, std::int32_t value)
    : value_(value), llvm_(LLVMConstInt(llvm.int32_type(), static_cast<std::uint64_t>(static_cast<std::int64_t>(value)), 1)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:284-284
std::int32_t ConstInt32::value() const { return value_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:285-285
LLVMValueRef ConstInt32::llvm() const { return llvm_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:288-289
ConstInt64::ConstInt64(const CodegenLlvmHelpers& llvm, std::int64_t value)
    : value_(value), llvm_(LLVMConstInt(llvm.int64_type(), static_cast<std::uint64_t>(static_cast<std::int64_t>(value)), 1)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:288-288
std::int64_t ConstInt64::value() const { return value_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:289-289
LLVMValueRef ConstInt64::llvm() const { return llvm_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:292-293
ConstFloat32::ConstFloat32(const CodegenLlvmHelpers& llvm, float value)
    : value_(value), llvm_(LLVMConstReal(llvm.float_type(), static_cast<double>(value))) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:292-292
float ConstFloat32::value() const { return value_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:293-293
LLVMValueRef ConstFloat32::llvm() const { return llvm_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:296-297
ConstFloat64::ConstFloat64(const CodegenLlvmHelpers& llvm, double value)
    : value_(value), llvm_(LLVMConstReal(llvm.double_type(), static_cast<double>(value))) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:296-296
double ConstFloat64::value() const { return value_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:297-297
LLVMValueRef ConstFloat64::llvm() const { return llvm_; }

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:21-21
SlotType::SlotType() = default;
// NOTE(port): Polymorphic C++ lifetime boundary.
SlotType::~SlotType() = default;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:23-23
SlotType::Stack::Stack() = default;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:23-23
const SlotType::Stack SlotType::STACK;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:26-26
SlotType::Arena::Arena() = default;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:26-26
const SlotType::Arena SlotType::ARENA;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:29-29
SlotType::Return::Return() = default;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:29-29
const SlotType::Return SlotType::RETURN;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:32-32
SlotType::ReturnIfArena::ReturnIfArena() = default;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:32-32
const SlotType::ReturnIfArena SlotType::RETURN_IF_ARENA;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:41-41
SlotType::Anonymous::Anonymous() = default;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:41-41
const SlotType::Anonymous SlotType::ANONYMOUS;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:44-44
SlotType::Unknown::Unknown() = default;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:44-44
const SlotType::Unknown SlotType::UNKNOWN;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:35-35
SlotType::ParamIfArena::ParamIfArena(int parameter) : parameter_(parameter) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:35-35
int SlotType::ParamIfArena::parameter() const { return parameter_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:38-38
SlotType::ParamsIfArena::ParamsIfArena(std::shared_ptr<std::vector<int>> parameters, bool use_return_slot)
    : parameters_(std::move(parameters)), use_return_slot_(use_return_slot) {
    if (!parameters_) throw std::invalid_argument("parameters must not be null");
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:38-38
const std::shared_ptr<std::vector<int>>& SlotType::ParamsIfArena::parameters() const { return parameters_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:38-38
bool SlotType::ParamsIfArena::use_return_slot() const { return use_return_slot_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:48-48
Lifetime::Lifetime(const SlotType& slot_type) : slot_type_(&slot_type) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:48-48,81-81,88-89
Lifetime::Lifetime(std::unique_ptr<const SlotType> slot_type)
    : owned_slot_type_(std::move(slot_type)), slot_type_(owned_slot_type_.get()) {}
// NOTE(port): Own only the parameter slot object allocated by this lifetime.
Lifetime::~Lifetime() = default;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:48-48
const SlotType& Lifetime::slot_type() const { return *slot_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:49-53
Lifetime::Stack::Stack() : Lifetime(SlotType::STACK) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:49-53
const Lifetime::Stack Lifetime::STACK;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:49-53
std::string Lifetime::Stack::to_string() const { return "STACK"; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:60-64
Lifetime::Local::Local() : Lifetime(SlotType::ARENA) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:60-64
const Lifetime::Local Lifetime::LOCAL;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:60-64
std::string Lifetime::Local::to_string() const { return "LOCAL"; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:67-71
Lifetime::ReturnValue::ReturnValue() : Lifetime(SlotType::ANONYMOUS) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:67-71
const Lifetime::ReturnValue Lifetime::RETURN_VALUE;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:67-71
std::string Lifetime::ReturnValue::to_string() const { return "RETURN_VALUE"; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:74-78
Lifetime::IndirectReturnValue::IndirectReturnValue() : Lifetime(SlotType::RETURN_IF_ARENA) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:74-78
const Lifetime::IndirectReturnValue Lifetime::INDIRECT_RETURN_VALUE;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:74-78
std::string Lifetime::IndirectReturnValue::to_string() const { return "INDIRECT_RETURN_VALUE"; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:96-100
Lifetime::Global::Global() : Lifetime(SlotType::ANONYMOUS) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:96-100
const Lifetime::Global Lifetime::GLOBAL;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:96-100
std::string Lifetime::Global::to_string() const { return "GLOBAL"; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:103-107
Lifetime::Throw::Throw() : Lifetime(SlotType::ANONYMOUS) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:103-107
const Lifetime::Throw Lifetime::THROW;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:103-107
std::string Lifetime::Throw::to_string() const { return "THROW"; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:111-115
Lifetime::Argument::Argument() : Lifetime(SlotType::ANONYMOUS) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:111-115
const Lifetime::Argument Lifetime::ARGUMENT;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:111-115
std::string Lifetime::Argument::to_string() const { return "ARGUMENT"; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:118-122
Lifetime::Unknown::Unknown() : Lifetime(SlotType::UNKNOWN) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:118-122
const Lifetime::Unknown Lifetime::UNKNOWN;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:118-122
std::string Lifetime::Unknown::to_string() const { return "UNKNOWN"; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:125-129
Lifetime::Irrelevant::Irrelevant() : Lifetime(SlotType::UNKNOWN) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:125-129
const Lifetime::Irrelevant Lifetime::IRRELEVANT;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:125-129
std::string Lifetime::Irrelevant::to_string() const { return "IRRELEVANT"; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:55-57
Lifetime::StackArray::StackArray(int size) : Lifetime(SlotType::STACK), size_(size) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:55-55
int Lifetime::StackArray::size() const { return size_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:56-56
std::string Lifetime::StackArray::to_string() const { return "STACK_ARRAY[" + std::to_string(size_) + "]"; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:81-85
Lifetime::ParameterField::ParameterField(int parameter)
    : Lifetime(std::make_unique<SlotType::ParamIfArena>(parameter)), parameter_(parameter) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:81-81
int Lifetime::ParameterField::parameter() const { return parameter_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:82-84
std::string Lifetime::ParameterField::to_string() const { return "PARAMETER_FIELD(" + std::to_string(parameter_) + ")"; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:88-93
Lifetime::ParametersField::ParametersField(std::shared_ptr<std::vector<int>> parameters, bool use_return_slot)
    : Lifetime(std::make_unique<SlotType::ParamsIfArena>(parameters, use_return_slot)),
      parameters_(std::move(parameters)), use_return_slot_(use_return_slot) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:88-88
const std::shared_ptr<std::vector<int>>& Lifetime::ParametersField::parameters() const { return parameters_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:88-88
bool Lifetime::ParametersField::use_return_slot() const { return use_return_slot_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:90-92
std::string Lifetime::ParametersField::to_string() const {
    std::string content = "[";
    for (std::size_t index = 0; index < parameters_->size(); ++index) {
        if (index != 0) content += ", ";
        content += std::to_string((*parameters_)[index]);
    }
    return "PARAMETERS_FIELD(" + content + "], useReturnSlot='" +
        (use_return_slot_ ? "true" : "false") + "')";
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:303-325
// NOTE(port): Serialize first initialization like Kotlin's default lazy mode.
class BasicLlvmHelpers::LazyProperties {
public:
    std::once_flag target_triple_once;
    std::string target_triple;
    std::once_flag runtime_annotation_map_once;
    std::map<std::string, std::vector<LLVMValueRef>> runtime_annotation_map;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:300-302
BasicLlvmHelpers::BasicLlvmHelpers(LLVMContextRef llvm_context, LLVMModuleRef module,
    bool use_llvm_opaque_pointers)
    : llvm_context_(llvm_context), module_(module), use_llvm_opaque_pointers_(use_llvm_opaque_pointers),
      lazy_(std::make_unique<LazyProperties>()) {}
// NOTE(port): Only lazy storage is owned; the enclosing compiler owns LLVM.
BasicLlvmHelpers::~BasicLlvmHelpers() = default;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:300-300
LLVMModuleRef BasicLlvmHelpers::module() const { return module_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:302-302
LLVMContextRef BasicLlvmHelpers::llvm_context() const { return llvm_context_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:303-305
const std::string& BasicLlvmHelpers::target_triple() const {
    std::call_once(lazy_->target_triple_once, [&] { lazy_->target_triple = LLVMGetTarget(module_); });
    return lazy_->target_triple;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:307-325
const std::map<std::string, std::vector<LLVMValueRef>>& BasicLlvmHelpers::runtime_annotation_map() const {
    std::call_once(lazy_->runtime_annotation_map_once, [&] {
        const auto global = LLVMGetNamedGlobal(module_, "llvm.global.annotations");
        const auto initializer = global ? LLVMGetInitializer(global) : nullptr;
        if (!initializer) return;
        // Build the snapshot locally so a failed lazy initialization can retry.
        std::map<std::string, std::vector<LLVMValueRef>> annotations;
        for (const auto entry : get_operands(initializer)) {
            const auto string_operand = LLVMGetOperand(entry, 1);
            const auto string_global = use_llvm_opaque_pointers_ ? string_operand : LLVMGetOperand(string_operand, 0);
            const auto string_initializer = LLVMGetInitializer(string_global);
            const auto key = string_initializer ? get_as_c_string(string_initializer) : "";
            const auto value_operand = LLVMGetOperand(entry, 0);
            const auto value = use_llvm_opaque_pointers_ ? value_operand : LLVMGetOperand(value_operand, 0);
            if (!key.empty()) annotations[key].push_back(value);
        }
        lazy_->runtime_annotation_map = std::move(annotations);
    });
    return lazy_->runtime_annotation_map;
}

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:445-504
struct CodegenLlvmHelpers::LazyRuntimeFunctions {
    std::once_flag imm_int32_zero_once;
    LLVMValueRef imm_int32_zero = nullptr;
    std::once_flag imm_int32_one_once;
    LLVMValueRef imm_int32_one = nullptr;
    std::once_flag true_constant_once;
    LLVMValueRef true_constant = nullptr;
    std::once_flag false_constant_once;
    LLVMValueRef false_constant = nullptr;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:445-445
    std::once_flag kotlin_mm_create_retained_external_rc_ref_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_mm_create_retained_external_rc_ref;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:446-446
    std::once_flag kotlin_mm_release_external_rc_ref_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_mm_release_external_rc_ref;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:447-447
    std::once_flag kotlin_mm_dispose_external_rc_ref_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_mm_dispose_external_rc_ref;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:449-449
    std::once_flag create_kotlin_obj_c_class_once;
    std::unique_ptr<LlvmFunction::Declaration> create_kotlin_obj_c_class;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:450-450
    std::once_flag get_obj_c_kotlin_type_info_once;
    std::unique_ptr<LlvmFunction::Declaration> get_obj_c_kotlin_type_info;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:451-451
    std::once_flag missing_init_imp_once;
    std::unique_ptr<LlvmFunction::Declaration> missing_init_imp;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:453-458
    std::once_flag kotlin_mm_switch_thread_state_native_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_mm_switch_thread_state_native;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:459-464
    std::once_flag kotlin_mm_switch_thread_state_runnable_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_mm_switch_thread_state_runnable;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:466-466
    std::once_flag kotlin_interop_does_object_conform_to_protocol_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_interop_does_object_conform_to_protocol;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:467-467
    std::once_flag kotlin_interop_does_object_conform_to_protocol_by_name_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_interop_does_object_conform_to_protocol_by_name;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:468-468
    std::once_flag kotlin_interop_is_object_kind_of_class_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_interop_is_object_kind_of_class;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:470-470
    std::once_flag kotlin_obj_c_export_ref_to_local_obj_c_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_ref_to_local_obj_c;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:471-471
    std::once_flag kotlin_obj_c_export_ref_to_retained_obj_c_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_ref_to_retained_obj_c;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:472-472
    std::once_flag kotlin_obj_c_export_ref_from_obj_c_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_ref_from_obj_c;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:473-473
    std::once_flag kotlin_obj_c_export_create_retained_ns_string_from_k_string_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_create_retained_ns_string_from_k_string;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:474-474
    std::once_flag kotlin_obj_c_export_convert_unit_to_retained_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_convert_unit_to_retained;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:475-475
    std::once_flag kotlin_obj_c_export_get_associated_object_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_get_associated_object;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:476-476
    std::once_flag kotlin_obj_c_export_abstract_method_called_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_abstract_method_called;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:477-477
    std::once_flag kotlin_obj_c_export_abstract_class_constructor_called_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_abstract_class_constructor_called;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:478-478
    std::once_flag kotlin_obj_c_export_rethrow_exception_as_ns_error_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_rethrow_exception_as_ns_error;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:479-479
    std::once_flag kotlin_obj_c_export_wrap_exception_to_ns_error_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_wrap_exception_to_ns_error;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:480-480
    std::once_flag kotlin_obj_c_export_ns_error_as_exception_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_ns_error_as_exception;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:481-481
    std::once_flag kotlin_obj_c_export_alloc_instance_with_associated_object_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_alloc_instance_with_associated_object;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:482-482
    std::once_flag kotlin_obj_c_export_create_continuation_argument_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_create_continuation_argument;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:483-483
    std::once_flag kotlin_obj_c_export_create_unit_continuation_argument_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_create_unit_continuation_argument;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:484-484
    std::once_flag kotlin_obj_c_export_resume_continuation_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_resume_continuation;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:486-486
    std::once_flag kotlin_obj_c_export_ns_integer_type_provider_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_obj_c_export_ns_integer_type_provider;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:487-487
    std::once_flag kotlin_long_type_provider_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_long_type_provider;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:489-489
    std::once_flag kotlin_mm_safe_point_function_prologue_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_mm_safe_point_function_prologue;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:490-490
    std::once_flag kotlin_mm_safe_point_while_loop_body_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_mm_safe_point_while_loop_body;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:492-492
    std::once_flag kotlin_process_object_in_mark_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_process_object_in_mark;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:493-493
    std::once_flag kotlin_process_array_in_mark_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_process_array_in_mark;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:494-494
    std::once_flag kotlin_process_empty_object_in_mark_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_process_empty_object_in_mark;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:496-496
    std::once_flag update_volatile_heap_ref_once;
    std::unique_ptr<LlvmFunction::Declaration> update_volatile_heap_ref;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:497-497
    std::once_flag compare_and_set_volatile_heap_ref_once;
    std::unique_ptr<LlvmFunction::Declaration> compare_and_set_volatile_heap_ref;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:498-498
    std::once_flag compare_and_swap_volatile_heap_ref_once;
    std::unique_ptr<LlvmFunction::Declaration> compare_and_swap_volatile_heap_ref;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:499-499
    std::once_flag get_and_set_volatile_heap_ref_once;
    std::unique_ptr<LlvmFunction::Declaration> get_and_set_volatile_heap_ref;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:502-502
    std::once_flag kotlin_array_get_element_address_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_array_get_element_address;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:503-503
    std::once_flag kotlin_int_array_get_element_address_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_int_array_get_element_address;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:504-504
    std::once_flag kotlin_long_array_get_element_address_once;
    std::unique_ptr<LlvmFunction::Declaration> kotlin_long_array_get_element_address;
};

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:329-330,411-441,453-464
CodegenLlvmHelpers::CodegenLlvmHelpers(LLVMContextRef llvm_context, LLVMModuleRef module,
    bool use_llvm_opaque_pointers, const Runtime& runtime, bool should_optimize)
    : BasicLlvmHelpers(llvm_context, module, use_llvm_opaque_pointers),
      lazy_runtime_functions_(std::make_unique<LazyRuntimeFunctions>()), should_optimize_(should_optimize),
      runtime_(runtime), use_llvm_opaque_pointers_(use_llvm_opaque_pointers),
      alloc_instance_function_((LLVMSetDataLayout(module, runtime.data_layout().c_str()),
          LLVMSetTarget(module, runtime.target().c_str()), import_rt_function("AllocInstance", true))),
      alloc_array_function_(import_rt_function("AllocArrayInstance", true)),
      register_global_function_(import_rt_function("RegisterGlobal", false)),
      update_heap_ref_function_(import_rt_function("UpdateHeapRef", false)),
      update_stack_ref_function_(import_rt_function("UpdateStackRef", false)),
      update_return_ref_function_(import_rt_function("UpdateReturnRef", false)),
      zero_heap_ref_function_(import_rt_function("ZeroHeapRef", false)),
      zero_array_refs_function_(import_rt_function("ZeroArrayRefs", false)),
      enter_frame_function_(import_rt_function("EnterFrame", false)),
      leave_frame_function_(import_rt_function("LeaveFrame", false)),
      set_current_frame_function_(import_rt_function("SetCurrentFrame", false)),
      check_current_frame_function_(import_rt_function("CheckCurrentFrame", false)),
      lookup_interface_table_record_(import_rt_function("LookupInterfaceTableRecord", false)),
      is_subtype_function_(import_rt_function("IsSubtype", false)),
      is_subclass_fast_function_(import_rt_function("IsSubclassFast", false)),
      get_type_info_(import_rt_function("Kotlin_Any_getTypeInfo", false)),
      throw_exception_function_(import_rt_function("ThrowException", false)),
      append_to_initalizers_tail_(import_rt_function("AppendToInitializersTail", false)),
      call_init_global_possibly_lock_(import_rt_function("CallInitGlobalPossiblyLock", false)),
      call_init_thread_local_(import_rt_function("CallInitThreadLocal", false)),
      add_tls_record_(import_rt_function("AddTLSRecord", false)),
      lookup_tls_(import_rt_function("LookupTLS", false)),
      init_runtime_if_needed_(import_rt_function("Kotlin_initRuntimeIfNeeded", false)),
      kotlin_get_exception_object_(import_rt_function("Kotlin_getExceptionObject", true)),
      int1_type_(LLVMInt1TypeInContext(llvm_context)),
      int8_type_(LLVMInt8TypeInContext(llvm_context)),
      int16_type_(LLVMInt16TypeInContext(llvm_context)),
      int32_type_(LLVMInt32TypeInContext(llvm_context)),
      int64_type_(LLVMInt64TypeInContext(llvm_context)),
      intptr_type_(LLVMIntPtrTypeInContext(llvm_context, runtime.target_data())),
      float_type_(LLVMFloatTypeInContext(llvm_context)),
      double_type_(LLVMDoubleTypeInContext(llvm_context)),
      vector128_type_(LLVMVectorType(float_type_, 4)),
      void_type_(LLVMVoidTypeInContext(llvm_context)),
      pointer_type_(runtime.pointer_type()),
      null_constant_(LLVMConstNull(pointer_type_)), null_pointer_(const_pointer(null_constant_)),
      memset_function_(import_memset()),
      llvm_trap_(llvm_intrinsic("llvm.trap", function_type(LLVMVoidTypeInContext(llvm_context)),
          {"cold", "noreturn", "nounwind"})) {}
// NOTE(port): Own compiler-side callable descriptors; LLVM modules stay borrowed.
CodegenLlvmHelpers::~CodegenLlvmHelpers() = default;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:409-409
const Runtime& CodegenLlvmHelpers::runtime() const { return runtime_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:418-418
const LlvmFunction& CodegenLlvmHelpers::alloc_instance_function() const { return *alloc_instance_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:419-419
const LlvmFunction& CodegenLlvmHelpers::alloc_array_function() const { return *alloc_array_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:420-420
const LlvmFunction& CodegenLlvmHelpers::register_global_function() const { return *register_global_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:421-421
const LlvmFunction& CodegenLlvmHelpers::update_heap_ref_function() const { return *update_heap_ref_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:422-422
const LlvmFunction& CodegenLlvmHelpers::update_stack_ref_function() const { return *update_stack_ref_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:423-423
const LlvmFunction& CodegenLlvmHelpers::update_return_ref_function() const { return *update_return_ref_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:424-424
const LlvmFunction& CodegenLlvmHelpers::zero_heap_ref_function() const { return *zero_heap_ref_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:425-425
const LlvmFunction& CodegenLlvmHelpers::zero_array_refs_function() const { return *zero_array_refs_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:426-426
const LlvmFunction& CodegenLlvmHelpers::enter_frame_function() const { return *enter_frame_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:427-427
const LlvmFunction& CodegenLlvmHelpers::leave_frame_function() const { return *leave_frame_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:428-428
const LlvmFunction& CodegenLlvmHelpers::set_current_frame_function() const { return *set_current_frame_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:429-429
const LlvmFunction& CodegenLlvmHelpers::check_current_frame_function() const { return *check_current_frame_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:430-430
const LlvmFunction& CodegenLlvmHelpers::lookup_interface_table_record() const { return *lookup_interface_table_record_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:431-431
const LlvmFunction& CodegenLlvmHelpers::is_subtype_function() const { return *is_subtype_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:432-432
const LlvmFunction& CodegenLlvmHelpers::is_subclass_fast_function() const { return *is_subclass_fast_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:433-433
const LlvmFunction& CodegenLlvmHelpers::get_type_info() const { return *get_type_info_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:434-434
const LlvmFunction& CodegenLlvmHelpers::throw_exception_function() const { return *throw_exception_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:435-435
const LlvmFunction& CodegenLlvmHelpers::append_to_initalizers_tail() const { return *append_to_initalizers_tail_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:436-436
const LlvmFunction& CodegenLlvmHelpers::call_init_global_possibly_lock() const { return *call_init_global_possibly_lock_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:437-437
const LlvmFunction& CodegenLlvmHelpers::call_init_thread_local() const { return *call_init_thread_local_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:438-438
const LlvmFunction& CodegenLlvmHelpers::add_tls_record() const { return *add_tls_record_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:439-439
const LlvmFunction& CodegenLlvmHelpers::lookup_tls() const { return *lookup_tls_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:440-440
const LlvmFunction& CodegenLlvmHelpers::init_runtime_if_needed() const { return *init_runtime_if_needed_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:441-441
const LlvmFunction& CodegenLlvmHelpers::kotlin_get_exception_object() const { return *kotlin_get_exception_object_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:445-445
const LlvmFunction& CodegenLlvmHelpers::kotlin_mm_create_retained_external_rc_ref() const {
    std::call_once(lazy_runtime_functions_->kotlin_mm_create_retained_external_rc_ref_once, [this] {
        lazy_runtime_functions_->kotlin_mm_create_retained_external_rc_ref = import_rt_function("Kotlin_mm_createRetainedExternalRCRef", false);
    });
    return *lazy_runtime_functions_->kotlin_mm_create_retained_external_rc_ref;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:446-446
const LlvmFunction& CodegenLlvmHelpers::kotlin_mm_release_external_rc_ref() const {
    std::call_once(lazy_runtime_functions_->kotlin_mm_release_external_rc_ref_once, [this] {
        lazy_runtime_functions_->kotlin_mm_release_external_rc_ref = import_rt_function("Kotlin_mm_releaseExternalRCRef", false);
    });
    return *lazy_runtime_functions_->kotlin_mm_release_external_rc_ref;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:447-447
const LlvmFunction& CodegenLlvmHelpers::kotlin_mm_dispose_external_rc_ref() const {
    std::call_once(lazy_runtime_functions_->kotlin_mm_dispose_external_rc_ref_once, [this] {
        lazy_runtime_functions_->kotlin_mm_dispose_external_rc_ref = import_rt_function("Kotlin_mm_disposeExternalRCRef", false);
    });
    return *lazy_runtime_functions_->kotlin_mm_dispose_external_rc_ref;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:449-449
const LlvmFunction& CodegenLlvmHelpers::create_kotlin_obj_c_class() const {
    std::call_once(lazy_runtime_functions_->create_kotlin_obj_c_class_once, [this] {
        lazy_runtime_functions_->create_kotlin_obj_c_class = import_rt_function("CreateKotlinObjCClass", false);
    });
    return *lazy_runtime_functions_->create_kotlin_obj_c_class;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:450-450
const LlvmFunction& CodegenLlvmHelpers::get_obj_c_kotlin_type_info() const {
    std::call_once(lazy_runtime_functions_->get_obj_c_kotlin_type_info_once, [this] {
        lazy_runtime_functions_->get_obj_c_kotlin_type_info = import_rt_function("GetObjCKotlinTypeInfo", false);
    });
    return *lazy_runtime_functions_->get_obj_c_kotlin_type_info;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:451-451
const LlvmFunction& CodegenLlvmHelpers::missing_init_imp() const {
    std::call_once(lazy_runtime_functions_->missing_init_imp_once, [this] {
        lazy_runtime_functions_->missing_init_imp = import_rt_function("MissingInitImp", false);
    });
    return *lazy_runtime_functions_->missing_init_imp;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:453-458
const LlvmFunction& CodegenLlvmHelpers::kotlin_mm_switch_thread_state_native() const {
    std::call_once(lazy_runtime_functions_->kotlin_mm_switch_thread_state_native_once, [this] {
        lazy_runtime_functions_->kotlin_mm_switch_thread_state_native = import_rt_function(should_optimize_ ? "Kotlin_mm_switchThreadStateNative" : "Kotlin_mm_switchThreadStateNative_debug", false);
    });
    return *lazy_runtime_functions_->kotlin_mm_switch_thread_state_native;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:459-464
const LlvmFunction& CodegenLlvmHelpers::kotlin_mm_switch_thread_state_runnable() const {
    std::call_once(lazy_runtime_functions_->kotlin_mm_switch_thread_state_runnable_once, [this] {
        lazy_runtime_functions_->kotlin_mm_switch_thread_state_runnable = import_rt_function(should_optimize_ ? "Kotlin_mm_switchThreadStateRunnable" : "Kotlin_mm_switchThreadStateRunnable_debug", false);
    });
    return *lazy_runtime_functions_->kotlin_mm_switch_thread_state_runnable;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:466-466
const LlvmFunction& CodegenLlvmHelpers::kotlin_interop_does_object_conform_to_protocol() const {
    std::call_once(lazy_runtime_functions_->kotlin_interop_does_object_conform_to_protocol_once, [this] {
        lazy_runtime_functions_->kotlin_interop_does_object_conform_to_protocol = import_rt_function("Kotlin_Interop_DoesObjectConformToProtocol", false);
    });
    return *lazy_runtime_functions_->kotlin_interop_does_object_conform_to_protocol;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:467-467
const LlvmFunction& CodegenLlvmHelpers::kotlin_interop_does_object_conform_to_protocol_by_name() const {
    std::call_once(lazy_runtime_functions_->kotlin_interop_does_object_conform_to_protocol_by_name_once, [this] {
        lazy_runtime_functions_->kotlin_interop_does_object_conform_to_protocol_by_name = import_rt_function("Kotlin_Interop_DoesObjectConformToProtocolByName", false);
    });
    return *lazy_runtime_functions_->kotlin_interop_does_object_conform_to_protocol_by_name;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:468-468
const LlvmFunction& CodegenLlvmHelpers::kotlin_interop_is_object_kind_of_class() const {
    std::call_once(lazy_runtime_functions_->kotlin_interop_is_object_kind_of_class_once, [this] {
        lazy_runtime_functions_->kotlin_interop_is_object_kind_of_class = import_rt_function("Kotlin_Interop_IsObjectKindOfClass", false);
    });
    return *lazy_runtime_functions_->kotlin_interop_is_object_kind_of_class;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:470-470
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_ref_to_local_obj_c() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_ref_to_local_obj_c_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_ref_to_local_obj_c = import_rt_function("Kotlin_ObjCExport_refToLocalObjC", false);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_ref_to_local_obj_c;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:471-471
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_ref_to_retained_obj_c() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_ref_to_retained_obj_c_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_ref_to_retained_obj_c = import_rt_function("Kotlin_ObjCExport_refToRetainedObjC", false);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_ref_to_retained_obj_c;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:472-472
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_ref_from_obj_c() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_ref_from_obj_c_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_ref_from_obj_c = import_rt_function("Kotlin_ObjCExport_refFromObjC", true);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_ref_from_obj_c;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:473-473
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_create_retained_ns_string_from_k_string() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_create_retained_ns_string_from_k_string_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_create_retained_ns_string_from_k_string = import_rt_function("Kotlin_ObjCExport_CreateRetainedNSStringFromKString", false);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_create_retained_ns_string_from_k_string;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:474-474
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_convert_unit_to_retained() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_convert_unit_to_retained_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_convert_unit_to_retained = import_rt_function("Kotlin_ObjCExport_convertUnitToRetained", false);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_convert_unit_to_retained;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:475-475
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_get_associated_object() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_get_associated_object_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_get_associated_object = import_rt_function("Kotlin_ObjCExport_GetAssociatedObject", false);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_get_associated_object;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:476-476
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_abstract_method_called() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_abstract_method_called_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_abstract_method_called = import_rt_function("Kotlin_ObjCExport_AbstractMethodCalled", false);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_abstract_method_called;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:477-477
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_abstract_class_constructor_called() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_abstract_class_constructor_called_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_abstract_class_constructor_called = import_rt_function("Kotlin_ObjCExport_AbstractClassConstructorCalled", false);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_abstract_class_constructor_called;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:478-478
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_rethrow_exception_as_ns_error() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_rethrow_exception_as_ns_error_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_rethrow_exception_as_ns_error = import_rt_function("Kotlin_ObjCExport_RethrowExceptionAsNSError", false);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_rethrow_exception_as_ns_error;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:479-479
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_wrap_exception_to_ns_error() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_wrap_exception_to_ns_error_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_wrap_exception_to_ns_error = import_rt_function("Kotlin_ObjCExport_WrapExceptionToNSError", false);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_wrap_exception_to_ns_error;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:480-480
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_ns_error_as_exception() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_ns_error_as_exception_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_ns_error_as_exception = import_rt_function("Kotlin_ObjCExport_NSErrorAsException", true);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_ns_error_as_exception;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:481-481
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_alloc_instance_with_associated_object() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_alloc_instance_with_associated_object_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_alloc_instance_with_associated_object = import_rt_function("Kotlin_ObjCExport_AllocInstanceWithAssociatedObject", true);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_alloc_instance_with_associated_object;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:482-482
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_create_continuation_argument() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_create_continuation_argument_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_create_continuation_argument = import_rt_function("Kotlin_ObjCExport_createContinuationArgument", true);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_create_continuation_argument;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:483-483
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_create_unit_continuation_argument() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_create_unit_continuation_argument_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_create_unit_continuation_argument = import_rt_function("Kotlin_ObjCExport_createUnitContinuationArgument", true);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_create_unit_continuation_argument;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:484-484
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_resume_continuation() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_resume_continuation_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_resume_continuation = import_rt_function("Kotlin_ObjCExport_resumeContinuation", false);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_resume_continuation;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:486-486
const LlvmFunction& CodegenLlvmHelpers::kotlin_obj_c_export_ns_integer_type_provider() const {
    std::call_once(lazy_runtime_functions_->kotlin_obj_c_export_ns_integer_type_provider_once, [this] {
        lazy_runtime_functions_->kotlin_obj_c_export_ns_integer_type_provider = import_rt_function("Kotlin_ObjCExport_NSIntegerTypeProvider", false);
    });
    return *lazy_runtime_functions_->kotlin_obj_c_export_ns_integer_type_provider;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:487-487
const LlvmFunction& CodegenLlvmHelpers::kotlin_long_type_provider() const {
    std::call_once(lazy_runtime_functions_->kotlin_long_type_provider_once, [this] {
        lazy_runtime_functions_->kotlin_long_type_provider = import_rt_function("Kotlin_longTypeProvider", false);
    });
    return *lazy_runtime_functions_->kotlin_long_type_provider;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:489-489
const LlvmFunction& CodegenLlvmHelpers::kotlin_mm_safe_point_function_prologue() const {
    std::call_once(lazy_runtime_functions_->kotlin_mm_safe_point_function_prologue_once, [this] {
        lazy_runtime_functions_->kotlin_mm_safe_point_function_prologue = import_rt_function("Kotlin_mm_safePointFunctionPrologue", false);
    });
    return *lazy_runtime_functions_->kotlin_mm_safe_point_function_prologue;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:490-490
const LlvmFunction& CodegenLlvmHelpers::kotlin_mm_safe_point_while_loop_body() const {
    std::call_once(lazy_runtime_functions_->kotlin_mm_safe_point_while_loop_body_once, [this] {
        lazy_runtime_functions_->kotlin_mm_safe_point_while_loop_body = import_rt_function("Kotlin_mm_safePointWhileLoopBody", false);
    });
    return *lazy_runtime_functions_->kotlin_mm_safe_point_while_loop_body;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:492-492
const LlvmFunction& CodegenLlvmHelpers::kotlin_process_object_in_mark() const {
    std::call_once(lazy_runtime_functions_->kotlin_process_object_in_mark_once, [this] {
        lazy_runtime_functions_->kotlin_process_object_in_mark = import_rt_function("Kotlin_processObjectInMark", false);
    });
    return *lazy_runtime_functions_->kotlin_process_object_in_mark;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:493-493
const LlvmFunction& CodegenLlvmHelpers::kotlin_process_array_in_mark() const {
    std::call_once(lazy_runtime_functions_->kotlin_process_array_in_mark_once, [this] {
        lazy_runtime_functions_->kotlin_process_array_in_mark = import_rt_function("Kotlin_processArrayInMark", false);
    });
    return *lazy_runtime_functions_->kotlin_process_array_in_mark;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:494-494
const LlvmFunction& CodegenLlvmHelpers::kotlin_process_empty_object_in_mark() const {
    std::call_once(lazy_runtime_functions_->kotlin_process_empty_object_in_mark_once, [this] {
        lazy_runtime_functions_->kotlin_process_empty_object_in_mark = import_rt_function("Kotlin_processEmptyObjectInMark", false);
    });
    return *lazy_runtime_functions_->kotlin_process_empty_object_in_mark;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:496-496
const LlvmFunction& CodegenLlvmHelpers::update_volatile_heap_ref() const {
    std::call_once(lazy_runtime_functions_->update_volatile_heap_ref_once, [this] {
        lazy_runtime_functions_->update_volatile_heap_ref = import_rt_function("UpdateVolatileHeapRef", false);
    });
    return *lazy_runtime_functions_->update_volatile_heap_ref;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:497-497
const LlvmFunction& CodegenLlvmHelpers::compare_and_set_volatile_heap_ref() const {
    std::call_once(lazy_runtime_functions_->compare_and_set_volatile_heap_ref_once, [this] {
        lazy_runtime_functions_->compare_and_set_volatile_heap_ref = import_rt_function("CompareAndSetVolatileHeapRef", false);
    });
    return *lazy_runtime_functions_->compare_and_set_volatile_heap_ref;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:498-498
const LlvmFunction& CodegenLlvmHelpers::compare_and_swap_volatile_heap_ref() const {
    std::call_once(lazy_runtime_functions_->compare_and_swap_volatile_heap_ref_once, [this] {
        lazy_runtime_functions_->compare_and_swap_volatile_heap_ref = import_rt_function("CompareAndSwapVolatileHeapRef", true);
    });
    return *lazy_runtime_functions_->compare_and_swap_volatile_heap_ref;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:499-499
const LlvmFunction& CodegenLlvmHelpers::get_and_set_volatile_heap_ref() const {
    std::call_once(lazy_runtime_functions_->get_and_set_volatile_heap_ref_once, [this] {
        lazy_runtime_functions_->get_and_set_volatile_heap_ref = import_rt_function("GetAndSetVolatileHeapRef", true);
    });
    return *lazy_runtime_functions_->get_and_set_volatile_heap_ref;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:502-502
const LlvmFunction& CodegenLlvmHelpers::kotlin_array_get_element_address() const {
    std::call_once(lazy_runtime_functions_->kotlin_array_get_element_address_once, [this] {
        lazy_runtime_functions_->kotlin_array_get_element_address = import_rt_function("Kotlin_arrayGetElementAddress", false);
    });
    return *lazy_runtime_functions_->kotlin_array_get_element_address;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:503-503
const LlvmFunction& CodegenLlvmHelpers::kotlin_int_array_get_element_address() const {
    std::call_once(lazy_runtime_functions_->kotlin_int_array_get_element_address_once, [this] {
        lazy_runtime_functions_->kotlin_int_array_get_element_address = import_rt_function("Kotlin_intArrayGetElementAddress", false);
    });
    return *lazy_runtime_functions_->kotlin_int_array_get_element_address;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:504-504
const LlvmFunction& CodegenLlvmHelpers::kotlin_long_array_get_element_address() const {
    std::call_once(lazy_runtime_functions_->kotlin_long_array_get_element_address_once, [this] {
        lazy_runtime_functions_->kotlin_long_array_get_element_address = import_rt_function("Kotlin_longArrayGetElementAddress", false);
    });
    return *lazy_runtime_functions_->kotlin_long_array_get_element_address;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:514-514
LLVMTypeRef CodegenLlvmHelpers::int1_type() const { return int1_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:515-515
LLVMTypeRef CodegenLlvmHelpers::int8_type() const { return int8_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:516-516
LLVMTypeRef CodegenLlvmHelpers::int16_type() const { return int16_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:517-517
LLVMTypeRef CodegenLlvmHelpers::int32_type() const { return int32_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:518-518
LLVMTypeRef CodegenLlvmHelpers::int64_type() const { return int64_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:519-519
LLVMTypeRef CodegenLlvmHelpers::intptr_type() const { return intptr_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:520-520
LLVMTypeRef CodegenLlvmHelpers::float_type() const { return float_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:521-521
LLVMTypeRef CodegenLlvmHelpers::double_type() const { return double_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:522-522
LLVMTypeRef CodegenLlvmHelpers::vector128_type() const { return vector128_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:523-523
LLVMTypeRef CodegenLlvmHelpers::void_type() const { return void_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:524-524
LLVMTypeRef CodegenLlvmHelpers::pointer_type() const { return pointer_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:526-526
LLVMTypeRef CodegenLlvmHelpers::struct_type(const std::vector<LLVMTypeRef>& types) const { return struct_type(types, false); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:531-532
LLVMTypeRef CodegenLlvmHelpers::struct_type(const std::vector<LLVMTypeRef>& types, bool packed) const {
    auto fields = types;
    return LLVMStructTypeInContext(llvm_context(), fields.data(), static_cast<unsigned>(fields.size()), packed);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:528-529
Struct CodegenLlvmHelpers::struct_(const std::vector<std::shared_ptr<const ConstValue>>& elements, bool packed) const {
    std::vector<LLVMTypeRef> types;
    types.reserve(elements.size());
    for (const auto& element : elements) types.push_back(llvm_type(*element));
    return Struct(struct_type(types, packed), elements);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:534-541
LLVMTypeRef CodegenLlvmHelpers::struct_type_with_flexible_array(LLVMTypeRef original, int new_size) const {
    // NOTE(port): Source assertions follow the C++ NDEBUG policy; failures become compiler exceptions.
#ifndef NDEBUG
    if (LLVMGetTypeKind(original) != LLVMStructTypeKind) throw std::logic_error("not a struct");
#endif
    std::vector<LLVMTypeRef> types(LLVMCountStructElementTypes(original));
    LLVMGetStructElementTypes(original, types.data());
    const auto array = types.at(types.size() - 1);
#ifndef NDEBUG
    if (LLVMGetTypeKind(array) != LLVMArrayTypeKind || LLVMGetArrayLength(array) != 0)
        throw std::logic_error("not a flexible array");
#endif
    types.back() = LLVMArrayType(LLVMGetElementType(array), new_size);
    return LLVMStructTypeInContext(llvm_context(), types.data(), static_cast<unsigned>(types.size()), LLVMIsPackedStruct(original));
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:543-543
ConstInt1 CodegenLlvmHelpers::const_int1(bool value) const { return ConstInt1(*this, value); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:553-553
LLVMValueRef CodegenLlvmHelpers::int1(bool value) const { return const_int1(value).llvm(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:544-544
ConstInt8 CodegenLlvmHelpers::const_int8(std::int8_t value) const { return ConstInt8(*this, value); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:554-554
LLVMValueRef CodegenLlvmHelpers::int8(std::int8_t value) const { return const_int8(value).llvm(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:545-545
ConstUInt8 CodegenLlvmHelpers::const_uint8(std::uint8_t value) const { return ConstUInt8(*this, value); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:546-546
ConstInt16 CodegenLlvmHelpers::const_int16(std::int16_t value) const { return ConstInt16(*this, value); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:555-555
LLVMValueRef CodegenLlvmHelpers::int16(std::int16_t value) const { return const_int16(value).llvm(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:547-547
ConstChar16 CodegenLlvmHelpers::const_char16(char16_t value) const { return ConstChar16(*this, value); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:556-556
LLVMValueRef CodegenLlvmHelpers::char16(char16_t value) const { return const_char16(value).llvm(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:548-548
ConstInt32 CodegenLlvmHelpers::const_int32(std::int32_t value) const { return ConstInt32(*this, value); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:557-557
LLVMValueRef CodegenLlvmHelpers::int32(std::int32_t value) const { return const_int32(value).llvm(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:549-549
ConstInt64 CodegenLlvmHelpers::const_int64(std::int64_t value) const { return ConstInt64(*this, value); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:558-558
LLVMValueRef CodegenLlvmHelpers::int64(std::int64_t value) const { return const_int64(value).llvm(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:550-550
ConstFloat32 CodegenLlvmHelpers::const_float32(float value) const { return ConstFloat32(*this, value); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:560-560
LLVMValueRef CodegenLlvmHelpers::float32(float value) const { return const_float32(value).llvm(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:551-551
ConstFloat64 CodegenLlvmHelpers::const_float64(double value) const { return ConstFloat64(*this, value); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:561-561
LLVMValueRef CodegenLlvmHelpers::float64(double value) const { return const_float64(value).llvm(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:559-559
LLVMValueRef CodegenLlvmHelpers::intptr(std::int32_t value) const { return LLVMConstInt(intptr_type_, static_cast<std::uint64_t>(static_cast<std::int64_t>(value)), 1); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:563-563
LLVMValueRef CodegenLlvmHelpers::null_constant() const { return null_constant_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:564-564
LLVMValueRef CodegenLlvmHelpers::imm_int32_zero() const {
    std::call_once(lazy_runtime_functions_->imm_int32_zero_once, [this] { lazy_runtime_functions_->imm_int32_zero = int32(0); });
    return lazy_runtime_functions_->imm_int32_zero;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:565-565
LLVMValueRef CodegenLlvmHelpers::imm_int32_one() const {
    std::call_once(lazy_runtime_functions_->imm_int32_one_once, [this] { lazy_runtime_functions_->imm_int32_one = int32(1); });
    return lazy_runtime_functions_->imm_int32_one;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:566-566
LLVMValueRef CodegenLlvmHelpers::true_constant() const {
    std::call_once(lazy_runtime_functions_->true_constant_once, [this] { lazy_runtime_functions_->true_constant = int1(true); });
    return lazy_runtime_functions_->true_constant;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:567-567
LLVMValueRef CodegenLlvmHelpers::false_constant() const {
    std::call_once(lazy_runtime_functions_->false_constant_once, [this] { lazy_runtime_functions_->false_constant = int1(false); });
    return lazy_runtime_functions_->false_constant;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:569-571
const std::shared_ptr<ConstPointer>& CodegenLlvmHelpers::null_pointer() const { return null_pointer_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:573-573
const LlvmCallable& CodegenLlvmHelpers::memset_function() const { return *memset_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:575-579
const LlvmFunction::Declaration& CodegenLlvmHelpers::llvm_trap() const { return *llvm_trap_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:332-347
std::unique_ptr<LlvmFunction::Declaration> CodegenLlvmHelpers::import_function(const std::string& name,
    LLVMModuleRef other_module, bool returns_object_type) const {
    if (LLVMGetNamedFunction(module(), name.c_str()))
        throw std::invalid_argument("function " + name + " already exists");
    const auto external_function = LLVMGetNamedFunction(other_module, name.c_str());
    if (!external_function) throw std::runtime_error("function " + name + " not found");
    const auto attributes_copier = LlvmFunctionAttributeProvider::copy_from_external(external_function);
    const auto type = get_global_function_type(external_function);
    const auto function = LLVMAddFunction(module(), name.c_str(), type);
    attributes_copier->add_function_attributes(function);
    return std::make_unique<LlvmFunction::Declaration>(type, returns_object_type, function, attributes_copier);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:349-356
std::unique_ptr<LlvmFunction::Declaration> CodegenLlvmHelpers::import_memset() const {
    const auto context = llvm_context();
    const auto type = function_type(LLVMVoidTypeInContext(context), false,
        {LLVMPointerTypeInContext(context, 0), LLVMInt8TypeInContext(context),
         LLVMInt32TypeInContext(context), LLVMInt1TypeInContext(context)});
    return llvm_intrinsic(use_llvm_opaque_pointers_ ? "llvm.memset.p0.i32" : "llvm.memset.p0i8.i32", type);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:358-365
std::unique_ptr<LlvmFunction::Declaration> CodegenLlvmHelpers::llvm_intrinsic(const std::string& name,
    LLVMTypeRef type, const std::vector<std::string>& attributes) const {
    const auto result = LLVMAddFunction(module(), name.c_str(), type);
    for (const auto& attribute : attributes)
        add_llvm_function_enum_attribute(result, get_llvm_attribute_kind_id(attribute));
    return std::make_unique<LlvmFunction::Declaration>(type, false, result,
        LlvmFunctionAttributeProvider::copy_from_external(result));
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:416-416
std::unique_ptr<LlvmFunction::Declaration> CodegenLlvmHelpers::import_rt_function(const std::string& name,
    bool returns_object_type) const {
    return import_function(name, runtime().llvm_module(), returns_object_type);
}

}
