// port-lint: source kotlin-native/llvmDebugInfoC/src/main/cpp/DebugInfoC.cpp
// Transliterated from: kotlin-native/llvmDebugInfoC/src/main/cpp/DebugInfoC.cpp:253-286
// Declarations from: kotlin-native/llvmDebugInfoC/src/main/include/DebugInfoC.h:106-109
#pragma once

#include <llvm-c/Core.h>
#include <llvm-c/DebugInfo.h>
#include <cstdint>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// NOTE(port): Use the selected LLVM package's metadata and builder references.
// All references are borrowed; the LLVM context and DIBuilder own their objects.
// NOTE(port): Source C exports become snake_case compiler-namespace functions;
// the expression pointer is read-only, as in the source body.
// Transliterated from: kotlin-native/llvmDebugInfoC/src/main/cpp/DebugInfoC.cpp:253-260
LLVMMetadataRef di_create_auto_variable(LLVMDIBuilderRef builder,
    LLVMMetadataRef scope, const char* name, LLVMMetadataRef file,
    unsigned line, LLVMMetadataRef type);

// Transliterated from: kotlin-native/llvmDebugInfoC/src/main/cpp/DebugInfoC.cpp:262-270
LLVMMetadataRef di_create_parameter_variable(LLVMDIBuilderRef builder,
    LLVMMetadataRef scope, const char* name, unsigned arg_no,
    LLVMMetadataRef file, unsigned line, LLVMMetadataRef type);

// Transliterated from: kotlin-native/llvmDebugInfoC/src/main/cpp/DebugInfoC.cpp:272-274
LLVMMetadataRef di_create_empty_expression(LLVMDIBuilderRef builder);

// Transliterated from: kotlin-native/llvmDebugInfoC/src/main/cpp/DebugInfoC.cpp:276-286
void di_insert_declaration(LLVMDIBuilderRef builder, LLVMValueRef value,
    LLVMMetadataRef local_variable, LLVMMetadataRef location,
    LLVMBasicBlockRef block, const std::int64_t* expression,
    std::uint64_t expression_count);
}
