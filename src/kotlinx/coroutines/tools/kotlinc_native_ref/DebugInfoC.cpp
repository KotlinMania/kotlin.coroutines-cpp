// port-lint: source kotlin-native/llvmDebugInfoC/src/main/cpp/DebugInfoC.cpp
// Transliterated from: kotlin-native/llvmDebugInfoC/src/main/cpp/DebugInfoC.cpp:253-286
// Copyright 2010-2017 JetBrains s.r.o. Licensed under Apache-2.0.
#include "DebugInfoC.hpp"

#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>
#include <vector>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// Transliterated from: kotlin-native/llvmDebugInfoC/src/main/cpp/DebugInfoC.cpp:253-260
LLVMMetadataRef di_create_auto_variable(LLVMDIBuilderRef builder,
    LLVMMetadataRef scope, const char* name, LLVMMetadataRef file,
    unsigned line, LLVMMetadataRef type) {
    return ::llvm::wrap(::llvm::unwrap(builder)->createAutoVariable(
        ::llvm::unwrap<::llvm::DIScope>(scope), name,
        ::llvm::unwrap<::llvm::DIFile>(file), line,
        ::llvm::unwrap<::llvm::DIType>(type)));
}

// Transliterated from: kotlin-native/llvmDebugInfoC/src/main/cpp/DebugInfoC.cpp:262-270
LLVMMetadataRef di_create_parameter_variable(LLVMDIBuilderRef builder,
    LLVMMetadataRef scope, const char* name, unsigned arg_no,
    LLVMMetadataRef file, unsigned line, LLVMMetadataRef type) {
    return ::llvm::wrap(::llvm::unwrap(builder)->createParameterVariable(
        ::llvm::unwrap<::llvm::DIScope>(scope), name, arg_no,
        ::llvm::unwrap<::llvm::DIFile>(file), line,
        ::llvm::unwrap<::llvm::DIType>(type)));
}

// Transliterated from: kotlin-native/llvmDebugInfoC/src/main/cpp/DebugInfoC.cpp:272-274
LLVMMetadataRef di_create_empty_expression(LLVMDIBuilderRef builder) {
    return ::llvm::wrap(::llvm::unwrap(builder)->createExpression());
}

// Transliterated from: kotlin-native/llvmDebugInfoC/src/main/cpp/DebugInfoC.cpp:276-286
void di_insert_declaration(LLVMDIBuilderRef builder, LLVMValueRef value,
    LLVMMetadataRef local_variable, LLVMMetadataRef location,
    LLVMBasicBlockRef block, const std::int64_t* expression,
    std::uint64_t expression_count) {
    auto* di_builder = ::llvm::unwrap(builder);
    std::vector<std::uint64_t> operations;
    for (std::uint64_t i = 0; i < expression_count; ++i)
        operations.push_back(static_cast<std::uint64_t>(expression[i]));
    di_builder->insertDeclare(::llvm::unwrap(value),
        ::llvm::unwrap<::llvm::DILocalVariable>(local_variable),
        di_builder->createExpression(operations),
        ::llvm::unwrap<::llvm::DILocation>(location), ::llvm::unwrap(block));
}
}
