// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:120-212,895-939,2281-2340
#pragma once
#include "CodeContext.hpp"
#include <llvm-c/Core.h>
#include <functional>
#include <vector>
namespace org::jetbrains::kotlin::ir::expressions {
class IrExpression;
class IrSuspendableExpression;
class IrSuspensionPoint;
}
namespace org::jetbrains::kotlin::backend::konan::llvm {
class FunctionGenerationContext;
// Represents a basic block that may expect a value. A jump to this block must
// provide that value, which is accessible inside the block through value_phi.
// Used to generate expressions that have a value and require branching.
// A null value_phi means that a Unit value is passed.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:895-907
struct ContinuationBlock {
    LLVMBasicBlockRef block;
    LLVMValueRef value_phi;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:905-907
    LLVMValueRef value(LLVMValueRef unit_instance) const;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2281-2287
class SuspendableExpressionScope {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2281
    explicit SuspendableExpressionScope(std::vector<LLVMBasicBlockRef>& resume_points);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2282-2286
    int add_resume_point(LLVMBasicBlockRef bb_label);
private:
    std::vector<LLVMBasicBlockRef>& resume_points_;
};
// NOTE(port): The frontend provides lowered LLVM operands and expression emitters
// at this boundary. These callbacks emit IR; they do not execute coroutine code.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2289-2306
LLVMValueRef evaluate_suspendable_expression(FunctionGenerationContext& generation, LLVMValueRef suspension_point_id,
    LLVMValueRef result_slot, std::vector<LLVMBasicBlockRef>& resume_points,
    const std::function<LLVMValueRef(LLVMValueRef)>& evaluate_result);
// NOTE(port): evaluate_normal receives the real resume block address that Kotlin's
// SuspensionPointScope supplies when reading the suspension-point ID variable.
// Source location metadata and general IrValueDeclaration resolution are separate
// compiler dependencies, not supplied by this LLVM operand boundary.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2320-2340
LLVMValueRef evaluate_suspension_point(FunctionGenerationContext& generation, LLVMTypeRef result_type,
    bool is_unit, LLVMValueRef unit_instance, SuspendableExpressionScope& scope,
    const std::function<LLVMValueRef(LLVMValueRef)>& evaluate_normal,
    const std::function<LLVMValueRef()>& evaluate_resume);
// NOTE(port): A normalized IR emitter supplies general expression/type lowering.
// This callback emits LLVM in the actual lexical CodeContext; it runs no coroutine.
using IrExpressionEvaluator = std::function<LLVMValueRef(
    ir::expressions::IrExpression&, CodeContext&, LLVMValueRef)>;

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2289-2306
LLVMValueRef evaluate_suspendable_expression(FunctionGenerationContext& generation,
    ir::expressions::IrSuspendableExpression& expression, LLVMValueRef result_slot,
    CodeContext& outer_context, const IrExpressionEvaluator& evaluate_expression);
// NOTE(port): result_type and Unit are lowered LLVM values supplied by the same
// frontend emitter. Declaration lookup uses the actual IR suspension-point variable.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2320-2340
LLVMValueRef evaluate_suspension_point(FunctionGenerationContext& generation,
    ir::expressions::IrSuspensionPoint& expression, LLVMTypeRef result_type,
    bool is_unit, LLVMValueRef unit_instance, CodeContext& outer_context,
    const IrExpressionEvaluator& evaluate_expression);

}
