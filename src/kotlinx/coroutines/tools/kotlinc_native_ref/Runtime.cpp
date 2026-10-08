// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:14-122
#include "Runtime.hpp"
#include <mutex>
#include <stdexcept>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// NOTE(port): Synchronized source lazy properties and ownership of the created LLVM target data.
struct Runtime::LazyProperties {
    explicit LazyProperties(const std::string& layout) : target_data(LLVMCreateTargetData(layout.c_str())) {}
    ~LazyProperties() { LLVMDisposeTargetData(target_data); }
    const LLVMTargetDataRef target_data;
    std::once_flag kotlin_obj_c_class_data_once;
    LLVMTypeRef kotlin_obj_c_class_data = nullptr;
    std::once_flag kotlin_obj_c_class_info_once;
    LLVMTypeRef kotlin_obj_c_class_info = nullptr;
    std::once_flag obj_c_method_description_once;
    LLVMTypeRef obj_c_method_description = nullptr;
    std::once_flag obj_c_type_adapter_once;
    LLVMTypeRef obj_c_type_adapter = nullptr;
    std::once_flag obj_c_to_kotlin_method_adapter_once;
    LLVMTypeRef obj_c_to_kotlin_method_adapter = nullptr;
    std::once_flag kotlin_to_obj_c_method_adapter_once;
    LLVMTypeRef kotlin_to_obj_c_method_adapter = nullptr;
    std::once_flag type_info_obj_c_export_addition_once;
    LLVMTypeRef type_info_obj_c_export_addition = nullptr;
    std::once_flag block_literal_type_once;
    LLVMTypeRef block_literal_type = nullptr;
    std::once_flag block_descriptor_type_once;
    LLVMTypeRef block_descriptor_type = nullptr;
    std::once_flag obj_c_class_object_type_once;
    LLVMTypeRef obj_c_class_object_type = nullptr;
    std::once_flag obj_c_cache_once;
    LLVMTypeRef obj_c_cache = nullptr;
    std::once_flag obj_c_class_ro_type_once;
    LLVMTypeRef obj_c_class_ro_type = nullptr;
    std::once_flag obj_c_method_type_once;
    LLVMTypeRef obj_c_method_type = nullptr;
    std::once_flag pointer_size_once;
    int pointer_size{};
    std::once_flag pointer_alignment_once;
    int pointer_alignment{};
    std::once_flag string_header_extra_size_once;
    int string_header_extra_size{};
    std::once_flag is_big_endian_once;
    bool is_big_endian{};
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:18-23,43-62,102-102
Runtime::Runtime(LLVMContextRef llvm_context, LLVMModuleRef llvm_module)
    : llvm_context_(llvm_context), llvm_module_(llvm_module),
      pointer_type_(LLVMPointerTypeInContext(llvm_context, 0)),
      type_info_type_(get_struct_type("TypeInfo")),
      extended_type_info_type_(get_struct_type("ExtendedTypeInfo")),
      writable_type_info_type_(get_struct_type_or_null("WritableTypeInfo")),
      interface_table_record_type_(get_struct_type("InterfaceTableRecord")),
      associated_object_table_record_type_(get_struct_type("AssociatedObjectTableRecord")),
      obj_header_type_(get_struct_type("ObjHeader")),
      array_header_type_(get_struct_type("ArrayHeader")),
      string_header_type_(get_struct_type("StringHeader")),
      frame_overlay_type_(get_struct_type("FrameOverlay")),
      init_node_type_(get_struct_type("InitNode")),
      target_(LLVMGetTarget(llvm_module)), data_layout_(LLVMGetDataLayoutStr(llvm_module)),
      lazy_(std::make_unique<LazyProperties>(data_layout_)), i32_(LLVMInt32TypeInContext(llvm_context)) {}
// NOTE(port): Dispose owned target data without disposing the borrowed LLVM module/context.
Runtime::~Runtime() = default;
// NOTE(port): Polymorphic C++ interface lifetime.
RuntimeAware::~RuntimeAware() = default;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:23-23
LLVMModuleRef Runtime::llvm_module() const { return llvm_module_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:43-43
LLVMTypeRef Runtime::pointer_type() const { return pointer_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:44-44
LLVMTypeRef Runtime::type_info_type() const { return type_info_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:45-45
LLVMTypeRef Runtime::extended_type_info_type() const { return extended_type_info_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:46-46
LLVMTypeRef Runtime::writable_type_info_type() const { return writable_type_info_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:47-47
LLVMTypeRef Runtime::interface_table_record_type() const { return interface_table_record_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:48-48
LLVMTypeRef Runtime::associated_object_table_record_type() const { return associated_object_table_record_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:50-50
LLVMTypeRef Runtime::obj_header_type() const { return obj_header_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:51-51
LLVMTypeRef Runtime::array_header_type() const { return array_header_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:52-52
LLVMTypeRef Runtime::string_header_type() const { return string_header_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:54-54
LLVMTypeRef Runtime::frame_overlay_type() const { return frame_overlay_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:56-56
LLVMTypeRef Runtime::init_node_type() const { return init_node_type_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:58-58
const std::string& Runtime::target() const { return target_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:60-60
const std::string& Runtime::data_layout() const { return data_layout_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:62-62
LLVMTargetDataRef Runtime::target_data() const { return lazy_->target_data; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:64-64
LLVMTypeRef Runtime::kotlin_obj_c_class_data() const {
    std::call_once(lazy_->kotlin_obj_c_class_data_once, [this] { lazy_->kotlin_obj_c_class_data = get_struct_type("KotlinObjCClassData"); });
    return lazy_->kotlin_obj_c_class_data;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:65-65
LLVMTypeRef Runtime::kotlin_obj_c_class_info() const {
    std::call_once(lazy_->kotlin_obj_c_class_info_once, [this] { lazy_->kotlin_obj_c_class_info = get_struct_type("KotlinObjCClassInfo"); });
    return lazy_->kotlin_obj_c_class_info;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:66-66
LLVMTypeRef Runtime::obj_c_method_description() const {
    std::call_once(lazy_->obj_c_method_description_once, [this] { lazy_->obj_c_method_description = get_struct_type("ObjCMethodDescription"); });
    return lazy_->obj_c_method_description;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:67-67
LLVMTypeRef Runtime::obj_c_type_adapter() const {
    std::call_once(lazy_->obj_c_type_adapter_once, [this] { lazy_->obj_c_type_adapter = get_struct_type("ObjCTypeAdapter"); });
    return lazy_->obj_c_type_adapter;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:68-68
LLVMTypeRef Runtime::obj_c_to_kotlin_method_adapter() const {
    std::call_once(lazy_->obj_c_to_kotlin_method_adapter_once, [this] { lazy_->obj_c_to_kotlin_method_adapter = get_struct_type("ObjCToKotlinMethodAdapter"); });
    return lazy_->obj_c_to_kotlin_method_adapter;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:69-69
LLVMTypeRef Runtime::kotlin_to_obj_c_method_adapter() const {
    std::call_once(lazy_->kotlin_to_obj_c_method_adapter_once, [this] { lazy_->kotlin_to_obj_c_method_adapter = get_struct_type("KotlinToObjCMethodAdapter"); });
    return lazy_->kotlin_to_obj_c_method_adapter;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:70-70
LLVMTypeRef Runtime::type_info_obj_c_export_addition() const {
    std::call_once(lazy_->type_info_obj_c_export_addition_once, [this] { lazy_->type_info_obj_c_export_addition = get_struct_type("TypeInfoObjCExportAddition"); });
    return lazy_->type_info_obj_c_export_addition;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:104-104
LLVMTypeRef Runtime::block_literal_type() const {
    std::call_once(lazy_->block_literal_type_once, [this] { lazy_->block_literal_type = get_struct_type("Block_literal_1"); });
    return lazy_->block_literal_type;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:105-105
LLVMTypeRef Runtime::block_descriptor_type() const {
    std::call_once(lazy_->block_descriptor_type_once, [this] { lazy_->block_descriptor_type = get_struct_type("Block_descriptor_1"); });
    return lazy_->block_descriptor_type;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:72-81
LLVMTypeRef Runtime::obj_c_class_object_type() const {
    std::call_once(lazy_->obj_c_class_object_type_once, [this] { lazy_->obj_c_class_object_type = create_struct_type("_class_t", {pointer_type_, pointer_type_, pointer_type_, pointer_type_, pointer_type_}); });
    return lazy_->obj_c_class_object_type;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:82-82
LLVMTypeRef Runtime::obj_c_cache() const {
    std::call_once(lazy_->obj_c_cache_once, [this] { lazy_->obj_c_cache = create_opaque_struct_type("_objc_cache"); });
    return lazy_->obj_c_cache;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:83-97
LLVMTypeRef Runtime::obj_c_class_ro_type() const {
    std::call_once(lazy_->obj_c_class_ro_type_once, [this] { lazy_->obj_c_class_ro_type = create_struct_type("_class_ro_t", {i32_, i32_, i32_, pointer_type_, pointer_type_, pointer_type_, pointer_type_, pointer_type_, pointer_type_, pointer_type_}); });
    return lazy_->obj_c_class_ro_type;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:98-100
LLVMTypeRef Runtime::obj_c_method_type() const {
    std::call_once(lazy_->obj_c_method_type_once, [this] { lazy_->obj_c_method_type = create_struct_type("_objc_method", {pointer_type_, pointer_type_, pointer_type_}); });
    return lazy_->obj_c_method_type;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:111-111
int Runtime::pointer_size() const {
    std::call_once(lazy_->pointer_size_once, [this] { lazy_->pointer_size = size_of(pointer_type_); });
    return lazy_->pointer_size;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:112-112
int Runtime::pointer_alignment() const {
    std::call_once(lazy_->pointer_alignment_once, [this] { lazy_->pointer_alignment = align_of(pointer_type_); });
    return lazy_->pointer_alignment;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:114-116
int Runtime::string_header_extra_size() const {
    std::call_once(lazy_->string_header_extra_size_once, [this] { lazy_->string_header_extra_size = offset_of(string_header_type_, LLVMCountStructElementTypes(string_header_type_) - 1) - size_of(array_header_type_); });
    return lazy_->string_header_extra_size;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:121-121
bool Runtime::is_big_endian() const {
    std::call_once(lazy_->is_big_endian_once, [this] { lazy_->is_big_endian = LLVMByteOrder(target_data()) == LLVMBigEndian; });
    return lazy_->is_big_endian;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:107-107
int Runtime::size_of(LLVMTypeRef type) const { return static_cast<int>(LLVMABISizeOfType(target_data(), type)); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:108-108
int Runtime::align_of(LLVMTypeRef type) const { return static_cast<int>(LLVMABIAlignmentOfType(target_data(), type)); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:109-109
int Runtime::offset_of(LLVMTypeRef type, int index) const { return static_cast<int>(LLVMOffsetOfElement(target_data(), type, index)); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:118-119
int Runtime::object_alignment() const { return 8; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:27-29
LLVMTypeRef Runtime::get_struct_type_or_null(const std::string& name, bool is_class) const {
    const auto type = LLVMGetTypeByName(llvm_module_, ((is_class ? "class." : "struct.") + name).c_str());
    if (type) return type;
    const auto global = LLVMGetNamedGlobal(llvm_module_, ("touch" + name).c_str());
    return global ? LLVMGlobalGetValueType(global) : nullptr;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:31-32
LLVMTypeRef Runtime::get_struct_type(const std::string& name, bool is_class) const {
    const auto result = get_struct_type_or_null(name, is_class);
    if (!result) throw std::runtime_error("type " + name + " is not found in the Runtime module.");
    return result;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:34-38
LLVMTypeRef Runtime::create_struct_type(const std::string& name, const std::vector<LLVMTypeRef>& field_types) const {
    const auto result = LLVMStructCreateNamed(llvm_context_, name.c_str());
    if (!result) throw std::runtime_error("failed to create struct " + name);
    auto fields = field_types;
    LLVMStructSetBody(result, fields.data(), static_cast<unsigned>(fields.size()), 0);
    return result;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:40-41
LLVMTypeRef Runtime::create_opaque_struct_type(const std::string& name) const {
    const auto result = LLVMStructCreateNamed(llvm_context_, name.c_str());
    if (!result) throw std::runtime_error("failed to create struct " + name);
    return result;
}
}
