// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:11-103
#include "LlvmCallable.hpp"
#include <llvm-c/DebugInfo.h>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:18-21
class LlvmCallable::LazyProperties {
public:
    std::once_flag name_once;
    std::once_flag return_type_once;
    std::once_flag num_params_once;
    std::once_flag is_constant_once;
    std::optional<std::string> name;
    LLVMTypeRef return_type = nullptr;
    int num_params = 0;
    bool is_constant = false;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:55-60
class LlvmFunction::LazyProperties {
public:
    std::once_flag no_unwind_once;
    bool is_no_unwind = false;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:11-16
LlvmCallable::LlvmCallable(LLVMTypeRef function_type, bool returns_object_type, LLVMValueRef llvm_value,
    std::shared_ptr<LlvmFunctionAttributeProvider> attribute_provider)
    : llvm_value_(llvm_value), attribute_provider_(std::move(attribute_provider)), function_type_(function_type),
      returns_object_type_(returns_object_type), lazy_(std::make_unique<LazyProperties>()) {}
// NOTE(port): C++ owns the lazy property records; LLVM operands stay borrowed.
LlvmCallable::~LlvmCallable() = default;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:12-12
LLVMTypeRef LlvmCallable::function_type() const { return function_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:13-13
bool LlvmCallable::returns_object_type() const { return returns_object_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:18-18
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:256-257
const std::optional<std::string>& LlvmCallable::name() const {
    std::call_once(lazy_->name_once, [&] {
        if (const auto value_name = LLVMGetValueName(llvm_value_)) lazy_->name = value_name;
    });
    return lazy_->name;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:19-19
LLVMTypeRef LlvmCallable::return_type() const {
    std::call_once(lazy_->return_type_once, [&] { lazy_->return_type = LLVMGetReturnType(function_type_); });
    return lazy_->return_type;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:20-20
int LlvmCallable::num_params() const {
    std::call_once(lazy_->num_params_once, [&] { lazy_->num_params = LLVMCountParamTypes(function_type_); });
    return lazy_->num_params;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:21-21
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:259-260
bool LlvmCallable::is_constant() const {
    std::call_once(lazy_->is_constant_once, [&] { lazy_->is_constant = LLVMIsConstant(llvm_value_) == 1; });
    return lazy_->is_constant;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:23-26
LLVMValueRef LlvmCallable::build_call(LLVMBuilderRef builder, const std::vector<LLVMValueRef>& args, const std::string& name) const {
    // NOTE(port): LLVM-C's array parameter is mutable; copy only the borrowed
    // operand handles to retain the source's immutable argument-list contract.
    auto operands = args;
    const auto call = LLVMBuildCall2(builder, function_type_, llvm_value_, operands.data(), operands.size(), name.c_str());
    attribute_provider_->add_call_site_attributes(call);
    return call;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:28-31
LLVMValueRef LlvmCallable::build_invoke(LLVMBuilderRef builder, const std::vector<LLVMValueRef>& args,
    LLVMBasicBlockRef success, LLVMBasicBlockRef catch_block, const std::string& name) const {
    auto operands = args;
    const auto invoke = LLVMBuildInvoke2(builder, function_type_, llvm_value_, operands.data(), operands.size(),
        success, catch_block, name.c_str());
    attribute_provider_->add_call_site_attributes(invoke);
    return invoke;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:33-33
std::shared_ptr<ConstPointer> LlvmCallable::to_const_pointer() const { return const_pointer(llvm_value_); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:35-35
LLVMValueRef LlvmCallable::as_callback() const { return llvm_value_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:38-43
LlvmFunctionPointer::LlvmFunctionPointer(LLVMTypeRef function_type, bool returns_object_type, LLVMValueRef llvm_value,
    std::shared_ptr<LlvmFunctionAttributeProvider> attribute_provider)
    : LlvmCallable(function_type, returns_object_type, llvm_value, std::move(attribute_provider)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:48-53
LlvmFunction::LlvmFunction(LLVMTypeRef function_type, bool returns_object_type, LLVMValueRef llvm_value,
    std::shared_ptr<LlvmFunctionAttributeProvider> attribute_provider)
    : LlvmCallable(function_type, returns_object_type, llvm_value, std::move(attribute_provider)),
      lazy_(std::make_unique<LazyProperties>()) {}
// NOTE(port): Dispose the source object's cached properties at lifetime end.
LlvmFunction::~LlvmFunction() = default;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:55-60
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmUtils.kt:307-319
bool LlvmFunction::is_no_unwind() const {
    std::call_once(lazy_->no_unwind_once, [&] {
        if (!LLVMIsAFunction(llvm_value_)) {
            const auto value_name = LLVMGetValueName(llvm_value_);
            throw std::invalid_argument("The LLVM value '" + std::string(value_name ? value_name : "null") +
                "' is not a function. Supposed to be a function named '" + name().value_or("null") + "'.");
        }
        // NOTE(port): The source's NoUnwind attribute resolves to this actual
        // LLVM kind ID; retain its required failure when the kind is absent.
        const auto kind = LLVMGetEnumAttributeKindForName("nounwind", 8);
        if (kind == 0) throw std::runtime_error("Unable to find 'nounwind' attribute kind id");
        lazy_->is_no_unwind = LLVMGetEnumAttributeAtIndex(llvm_value_, LLVMAttributeFunctionIndex, kind) != nullptr;
    });
    return lazy_->is_no_unwind;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:62-67
LLVMValueRef LlvmFunction::param(int index) const {
    const auto count = num_params();
    if (index < 0 || index >= count)
        throw std::invalid_argument("Requested index " + std::to_string(index) + " but function '" +
            name().value_or("null") + "' got only " + std::to_string(count) + " params.");
    return LLVMGetParam(llvm_value_, index);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:69-70
LLVMValueRef LlvmFunction::build_landingpad(LLVMBuilderRef builder, LLVMTypeRef landingpad_type,
    int num_clauses, const std::string& name) const {
    return LLVMBuildLandingPad(builder, landingpad_type, llvm_value_, num_clauses, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:75-80
LlvmFunction::Declaration::Declaration(LLVMTypeRef function_type, bool returns_object_type, LLVMValueRef llvm_value,
    std::shared_ptr<LlvmFunctionAttributeProvider> attribute_provider)
    : LlvmFunction(function_type, returns_object_type, llvm_value, std::move(attribute_provider)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:85-90
LlvmFunction::Definition::Definition(LLVMTypeRef function_type, bool returns_object_type, LLVMValueRef llvm_value,
    std::shared_ptr<LlvmFunctionAttributeProvider> attribute_provider)
    : LlvmFunction(function_type, returns_object_type, llvm_value, std::move(attribute_provider)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:95-96
LLVMBasicBlockRef LlvmFunction::Definition::add_basic_block(LLVMContextRef context, const std::string& name) const {
    return LLVMAppendBasicBlockInContext(context, llvm_value_, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:98-99
LLVMValueRef LlvmFunction::Definition::block_address(LLVMBasicBlockRef label) const {
    return LLVMBlockAddress(llvm_value_, label);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:101-103
// NOTE(port): Native's DIFunctionAddSubprogram binds to the same actual LLVM
// subprogram attachment through the standard LLVM-C metadata API.
void LlvmFunction::Definition::add_debug_info_subprogram(LLVMMetadataRef subprogram) const {
    LLVMSetSubprogram(llvm_value_, subprogram);
}
}
