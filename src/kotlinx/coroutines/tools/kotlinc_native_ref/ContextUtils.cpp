// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:21-130,300-326
#include "ContextUtils.hpp"
#include "LlvmUtils.hpp"
#include <mutex>
#include <stdexcept>
#include <utility>

namespace org::jetbrains::kotlin::backend::konan::llvm {
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

}
