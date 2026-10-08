// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:21-130,300-365,411-441,573-579
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

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:329-330,411-441
CodegenLlvmHelpers::CodegenLlvmHelpers(LLVMContextRef llvm_context, LLVMModuleRef module,
    bool use_llvm_opaque_pointers, LLVMModuleRef runtime_module)
    : BasicLlvmHelpers(llvm_context, module, use_llvm_opaque_pointers),
      runtime_module_(runtime_module), use_llvm_opaque_pointers_(use_llvm_opaque_pointers),
      alloc_instance_function_((LLVMSetDataLayout(module, LLVMGetDataLayoutStr(runtime_module)),
          LLVMSetTarget(module, LLVMGetTarget(runtime_module)), import_rt_function("AllocInstance", true))),
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
      memset_function_(import_memset()),
      llvm_trap_(llvm_intrinsic("llvm.trap", function_type(LLVMVoidTypeInContext(llvm_context)),
          {"cold", "noreturn", "nounwind"})) {}
// NOTE(port): Own compiler-side callable descriptors; LLVM modules stay borrowed.
CodegenLlvmHelpers::~CodegenLlvmHelpers() = default;
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
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:573-573
const LlvmCallable& CodegenLlvmHelpers::memset_function() const { return *memset_function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:575-579
const LlvmFunction::Declaration& CodegenLlvmHelpers::llvm_trap() const { return *llvm_trap_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:332-347
std::unique_ptr<LlvmFunction::Declaration> CodegenLlvmHelpers::import_function(const std::string& name,
    LLVMModuleRef other_module, bool returns_object_type) {
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
std::unique_ptr<LlvmFunction::Declaration> CodegenLlvmHelpers::import_memset() {
    const auto context = llvm_context();
    const auto type = function_type(LLVMVoidTypeInContext(context), false,
        {LLVMPointerTypeInContext(context, 0), LLVMInt8TypeInContext(context),
         LLVMInt32TypeInContext(context), LLVMInt1TypeInContext(context)});
    return llvm_intrinsic(use_llvm_opaque_pointers_ ? "llvm.memset.p0.i32" : "llvm.memset.p0i8.i32", type);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:358-365
std::unique_ptr<LlvmFunction::Declaration> CodegenLlvmHelpers::llvm_intrinsic(const std::string& name,
    LLVMTypeRef type, const std::vector<std::string>& attributes) {
    const auto result = LLVMAddFunction(module(), name.c_str(), type);
    for (const auto& attribute : attributes)
        add_llvm_function_enum_attribute(result, get_llvm_attribute_kind_id(attribute));
    return std::make_unique<LlvmFunction::Declaration>(type, false, result,
        LlvmFunctionAttributeProvider::copy_from_external(result));
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:416-416
std::unique_ptr<LlvmFunction::Declaration> CodegenLlvmHelpers::import_rt_function(const std::string& name,
    bool returns_object_type) {
    return import_function(name, runtime_module_, returns_object_type);
}

}
