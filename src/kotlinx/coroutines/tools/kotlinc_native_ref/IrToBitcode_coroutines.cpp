// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:895-939,2281-2340
#include "IrToBitcode_coroutines.hpp"
#include "CodeGenerator.hpp"

namespace org::jetbrains::kotlin::backend::konan::llvm {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:905-907
LLVMValueRef ContinuationBlock::value(LLVMValueRef unit_instance) const {
    return value_phi ? value_phi : unit_instance;
}
// NOTE(port): Kotlin's constructor binds the supplied mutable resume-point list.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2281
SuspendableExpressionScope::SuspendableExpressionScope(std::vector<LLVMBasicBlockRef>& resume_points)
    : resume_points_(resume_points) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2282-2286
int SuspendableExpressionScope::add_resume_point(LLVMBasicBlockRef bb_label) {
    const auto result = static_cast<int>(resume_points_.size());
    resume_points_.push_back(bb_label);
    return result;
}
namespace {
// Jump to target, passing value to its phi.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:909-918
void jump(FunctionGenerationContext& generation, const ContinuationBlock& target, LLVMValueRef value) {
    generation.br(target.block);
    if (target.value_phi) generation.assign_phis({{target.value_phi, value}});
}
// Creates a new ContinuationBlock receiving a value of the given lowered Kotlin
// type and generates code starting from its beginning.
// NOTE(port): LocationInfo and IrType lowering remain separate dependencies.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:923-939
ContinuationBlock continuation_block(FunctionGenerationContext& generation, LLVMTypeRef type, bool is_unit,
    const std::function<void(const ContinuationBlock&)>& code = [](const ContinuationBlock&) {}) {
    const auto entry = generation.basic_block("continuation_block");
    return generation.appending_to<ContinuationBlock>(entry, [&](FunctionGenerationContext& context) {
        const auto value_phi = is_unit ? nullptr : context.phi(type);
        const auto result = ContinuationBlock{entry, value_phi};
        code(result);
        return result;
    });
}
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2289-2306
LLVMValueRef evaluate_suspendable_expression(FunctionGenerationContext& generation, LLVMValueRef suspension_point_id,
    LLVMValueRef result_slot, std::vector<LLVMBasicBlockRef>& resume_points,
    const std::function<LLVMValueRef(LLVMValueRef)>& evaluate_result) {
    const auto bb_start = generation.basic_block("start");
    const auto bb_dispatch = generation.basic_block("dispatch");
    generation.cond_br(generation.icmp_eq(suspension_point_id,
        LLVMConstNull(LLVMTypeOf(suspension_point_id))), bb_start, bb_dispatch);
    generation.position_at_end(bb_start);
    const auto result = evaluate_result(result_slot);
    generation.appending_to<void>(bb_dispatch, [&](FunctionGenerationContext& context) {
        context.indirect_br(suspension_point_id, resume_points);
    });
    return result;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2320-2340
LLVMValueRef evaluate_suspension_point(FunctionGenerationContext& generation, LLVMTypeRef result_type,
    bool is_unit, LLVMValueRef unit_instance, SuspendableExpressionScope& scope,
    const std::function<LLVMValueRef(LLVMValueRef)>& evaluate_normal,
    const std::function<LLVMValueRef()>& evaluate_resume) {
    const auto bb_resume = generation.basic_block("resume");
    const auto id = scope.add_resume_point(bb_resume);
    // NOTE(port): Kotlin retains this ID in SuspensionPointScope; its current
    // genGetValue uses the block address, and does not read the numeric ID.
    (void)id;
    const auto target = continuation_block(generation, result_type, is_unit);
    const auto normal_result = evaluate_normal(generation.block_address(bb_resume));
    jump(generation, target, normal_result);
    generation.position_at_end(bb_resume);
    const auto resume_result = evaluate_resume();
    jump(generation, target, resume_result);
    generation.position_at_end(target.block);
    return target.value(unit_instance);
}
}
