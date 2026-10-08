// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:14-122
#pragma once
#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <memory>
#include <string>
#include <vector>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:18-122
class Runtime {
public:
    // NOTE(port): Explicit compiler-module entry; phase-context bitcode loading is not yet translated.
    // The actual LLVM module/context stay borrowed and must outlive this metadata.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:18-23,43-62,102-102
    Runtime(LLVMContextRef llvm_context, LLVMModuleRef llvm_module);
    ~Runtime();
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:23-23
    LLVMModuleRef llvm_module() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:43-43
    LLVMTypeRef pointer_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:44-44
    LLVMTypeRef type_info_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:45-45
    LLVMTypeRef extended_type_info_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:46-46
    LLVMTypeRef writable_type_info_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:47-47
    LLVMTypeRef interface_table_record_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:48-48
    LLVMTypeRef associated_object_table_record_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:50-50
    LLVMTypeRef obj_header_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:51-51
    LLVMTypeRef array_header_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:52-52
    LLVMTypeRef string_header_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:54-54
    LLVMTypeRef frame_overlay_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:56-56
    LLVMTypeRef init_node_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:58-58
    const std::string& target() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:60-60
    const std::string& data_layout() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:62-62
    LLVMTargetDataRef target_data() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:64-64
    LLVMTypeRef kotlin_obj_c_class_data() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:65-65
    LLVMTypeRef kotlin_obj_c_class_info() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:66-66
    LLVMTypeRef obj_c_method_description() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:67-67
    LLVMTypeRef obj_c_type_adapter() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:68-68
    LLVMTypeRef obj_c_to_kotlin_method_adapter() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:69-69
    LLVMTypeRef kotlin_to_obj_c_method_adapter() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:70-70
    LLVMTypeRef type_info_obj_c_export_addition() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:104-104
    LLVMTypeRef block_literal_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:105-105
    LLVMTypeRef block_descriptor_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:72-81
    LLVMTypeRef obj_c_class_object_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:82-82
    LLVMTypeRef obj_c_cache() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:83-97
    LLVMTypeRef obj_c_class_ro_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:98-100
    LLVMTypeRef obj_c_method_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:111-111
    int pointer_size() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:112-112
    int pointer_alignment() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:114-116
    int string_header_extra_size() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:121-121
    bool is_big_endian() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:107-107
    int size_of(LLVMTypeRef type) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:108-108
    int align_of(LLVMTypeRef type) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:109-109
    int offset_of(LLVMTypeRef type, int index) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:118-119
    int object_alignment() const;
private:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:27-29
    LLVMTypeRef get_struct_type_or_null(const std::string& name, bool is_class = false) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:31-32
    LLVMTypeRef get_struct_type(const std::string& name, bool is_class = false) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:34-38
    LLVMTypeRef create_struct_type(const std::string& name, const std::vector<LLVMTypeRef>& field_types) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:40-41
    LLVMTypeRef create_opaque_struct_type(const std::string& name) const;
    const LLVMContextRef llvm_context_;
    const LLVMModuleRef llvm_module_;
    const LLVMTypeRef pointer_type_;
    const LLVMTypeRef type_info_type_;
    const LLVMTypeRef extended_type_info_type_;
    const LLVMTypeRef writable_type_info_type_;
    const LLVMTypeRef interface_table_record_type_;
    const LLVMTypeRef associated_object_table_record_type_;
    const LLVMTypeRef obj_header_type_;
    const LLVMTypeRef array_header_type_;
    const LLVMTypeRef string_header_type_;
    const LLVMTypeRef frame_overlay_type_;
    const LLVMTypeRef init_node_type_;
    const std::string target_;
    const std::string data_layout_;
    struct LazyProperties;
    const std::unique_ptr<LazyProperties> lazy_;
    const LLVMTypeRef i32_;
};

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:14-16
class RuntimeAware {
public:
    virtual ~RuntimeAware();
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/Runtime.kt:15-15
    virtual const Runtime& runtime() const = 0;
};
}
