// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:120-212
#pragma once
#include <llvm-c/Core.h>
#include <exception>

namespace org::jetbrains::kotlin::ir::declarations {
class IrSymbolOwner;
class IrVariable;
class IrValueDeclaration;
}
namespace org::jetbrains::kotlin::ir::expressions {
class IrBreak;
class IrContinue;
}
namespace org::jetbrains::kotlin::backend::konan::llvm {
class ExceptionHandler;
struct VariableDebugLocation;
class LocationInfo;

// Defines how to generate context-dependent operations.
// NOTE(port): IR declarations, debug locations and outer contexts are borrowed.
// LLVMMetadataRef carries the compiler-owned DIScope metadata at the LLVM-C boundary.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:120-212
class CodeContext {
public:
    // NOTE(port): C++ interface lifetime boundary.
    virtual ~CodeContext() = default;
    // Generates a return; value may be null only for a Unit target.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:130-130
    virtual void gen_return(ir::declarations::IrSymbolOwner& target, LLVMValueRef value) = 0;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:132-132
    virtual LLVMValueRef get_return_slot(ir::declarations::IrSymbolOwner& target) = 0;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:134-134
    virtual void gen_break(ir::expressions::IrBreak& destination) = 0;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:136-136
    virtual void gen_continue(ir::expressions::IrContinue& destination) = 0;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:138-138
    virtual ExceptionHandler& exception_handler() const = 0;
    // Declares the variable and returns its index.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:144-144
    virtual int gen_declare_variable(ir::declarations::IrVariable& variable, LLVMValueRef value, VariableDebugLocation* variable_location) = 0;
    // Returns the declared index, or -1 when this value has no declaration.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:149-149
    virtual int get_declared_value(ir::declarations::IrValueDeclaration& value) = 0;
    // Generates a read of the value available in this context.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:156-156
    virtual LLVMValueRef gen_get_value(ir::declarations::IrValueDeclaration& value, LLVMValueRef result_slot) = 0;
    // Returns the owning function scope, if present.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:163-163
    virtual CodeContext* function_scope() = 0;
    // Returns the owning file scope, if present.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:170-170
    virtual CodeContext* file_scope() = 0;
    // Returns the owning class scope, if present.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:177-177
    virtual CodeContext* class_scope() = 0;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:179-179
    virtual int add_resume_point(LLVMBasicBlockRef block) = 0;
    // Returns the owning returnable-block scope, if present.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:186-186
    virtual CodeContext* returnable_block_scope() = 0;
    // Returns location information for the source offset, if present.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:191-191
    virtual LocationInfo* location(int offset) = 0;
    // Returns this scope's actual LLVM debug metadata, if present.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:196-196
    virtual LLVMMetadataRef scope() = 0;
    // Wraps the caught compiler exception that will be rethrown.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:211-211
    virtual std::exception_ptr wrap_exception(std::exception_ptr error) = 0;
    // Called when the context is pushed onto the code-generation stack.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:198-201
    virtual void on_enter();
    // Called when the context is removed from that stack.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:203-206
    virtual void on_exit();
};
}
