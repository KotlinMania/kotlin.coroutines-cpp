// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:120-212,299-326,895-939,2152-2154,2281-2340
#include "IrToBitcode_coroutines.hpp"
#include "CodeGenerator.hpp"
#include "org/jetbrains/kotlin/ir/declarations/IrVariable.hpp"
#include "org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.hpp"
#include "org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.hpp"

namespace org::jetbrains::kotlin::backend::konan::llvm {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:198-201
void CodeContext::on_enter() {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:203-206
void CodeContext::on_exit() {}

namespace {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:300-303
// NOTE(port): C++ spells Kotlin interface delegation as concrete forwarding
// methods. The outer context owns its declarations and LLVM generation state.
class InnerScope : public CodeContext {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    explicit InnerScope(CodeContext& outer_context) : outer_context_(outer_context) {}
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    void gen_return(ir::declarations::IrSymbolOwner& target, LLVMValueRef value) override { outer_context_.gen_return(target, value); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    LLVMValueRef get_return_slot(ir::declarations::IrSymbolOwner& target) override { return outer_context_.get_return_slot(target); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    void gen_break(ir::expressions::IrBreak& destination) override { outer_context_.gen_break(destination); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    void gen_continue(ir::expressions::IrContinue& destination) override { outer_context_.gen_continue(destination); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    ExceptionHandler& exception_handler() const override { return outer_context_.exception_handler(); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    int gen_declare_variable(ir::declarations::IrVariable& variable, LLVMValueRef value, VariableDebugLocation* variable_location) override { return outer_context_.gen_declare_variable(variable, value, variable_location); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    int get_declared_value(ir::declarations::IrValueDeclaration& value) override { return outer_context_.get_declared_value(value); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    LLVMValueRef gen_get_value(ir::declarations::IrValueDeclaration& value, LLVMValueRef result_slot) override { return outer_context_.gen_get_value(value, result_slot); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    CodeContext* function_scope() override { return outer_context_.function_scope(); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    CodeContext* file_scope() override { return outer_context_.file_scope(); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    CodeContext* class_scope() override { return outer_context_.class_scope(); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    int add_resume_point(LLVMBasicBlockRef block) override { return outer_context_.add_resume_point(block); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    CodeContext* returnable_block_scope() override { return outer_context_.returnable_block_scope(); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    LocationInfo* location(int offset) override { return outer_context_.location(offset); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    LLVMMetadataRef scope() override { return outer_context_.scope(); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    std::exception_ptr wrap_exception(std::exception_ptr error) override { return outer_context_.wrap_exception(std::move(error)); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    void on_enter() override { outer_context_.on_enter(); }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:303-303
    void on_exit() override { outer_context_.on_exit(); }
private:
    CodeContext& outer_context_;
};

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2281-2287
// NOTE(port): The existing LLVM operand adapter owns the same resume-point list.
// This scope adds actual CodeContext delegation for normalized IR evaluation.
class SuspendableExpressionCodeScope final : public InnerScope {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2281-2281
    SuspendableExpressionCodeScope(CodeContext& outer_context, SuspendableExpressionScope& points)
        : InnerScope(outer_context), points_(points) {}
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2282-2286
    int add_resume_point(LLVMBasicBlockRef block) override { return points_.add_resume_point(block); }
private:
    SuspendableExpressionScope& points_;
};

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2309-2318
class SuspensionPointScope final : public InnerScope {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2309-2310
    SuspensionPointScope(CodeContext& outer_context, FunctionGenerationContext& generation,
        ir::declarations::IrVariable& suspension_point_id, LLVMBasicBlockRef resume, int resume_id)
        : InnerScope(outer_context), generation_(generation), suspension_point_id_(suspension_point_id),
          resume_(resume), bb_resume_id_(resume_id) {}
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2311-2317
    LLVMValueRef gen_get_value(ir::declarations::IrValueDeclaration& value, LLVMValueRef result_slot) override {
        if (&value == &static_cast<ir::declarations::IrValueDeclaration&>(suspension_point_id_))
            return generation_.block_address(resume_);
        return InnerScope::gen_get_value(value, result_slot);
    }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2310-2310
    int bb_resume_id() const { return bb_resume_id_; }
private:
    FunctionGenerationContext& generation_;
    ir::declarations::IrVariable& suspension_point_id_;
    LLVMBasicBlockRef resume_;
    // NOTE(port): Source code-generation metadata; runtime dispatch uses the
    // block address, never this index. The constructor val has its source getter.
    const int bb_resume_id_;
};

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:309-326
// NOTE(port): Pass the selected lexical context explicitly to the IR evaluator;
// no global context pointer is changed. Enter/exit and exception wrapping retain
// Kotlin's using-scope order, including an exception thrown by wrap_exception.
LLVMValueRef using_context(CodeContext& context, const std::function<LLVMValueRef(CodeContext&)>& code) {
    context.on_enter();
    LLVMValueRef result;
    try {
        result = code(context);
    } catch (...) {
        std::exception_ptr error;
        try { error = context.wrap_exception(std::current_exception()); }
        catch (...) { error = std::current_exception(); }
        context.on_exit();
        std::rethrow_exception(error);
    }
    context.on_exit();
    return result;
}
}

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
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2152-2154
// NOTE(port): Read the owning compiler's supplied location-debug policy.
LocationInfo* start_location(FunctionGenerationContext& generation, CodeContext& context, const ir::IrElement& element) {
    if (!generation.should_contain_location_debug_info()) return nullptr;
    return context.location(element.start_offset());
}
// Jump to target, passing value to its phi.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:909-918
void jump(FunctionGenerationContext& generation, const ContinuationBlock& target, LLVMValueRef value) {
    generation.br(target.block);
    if (target.value_phi) generation.assign_phis({{target.value_phi, value}});
}
// Creates a new ContinuationBlock receiving a value of the given lowered Kotlin
// type and generates code starting from its beginning.
// NOTE(port): The owning emitter supplies the lowered IrType and isUnit result.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:923-939
ContinuationBlock continuation_block(FunctionGenerationContext& generation, LLVMTypeRef type, bool is_unit,
    LocationInfo* location_info = nullptr,
    const std::function<void(const ContinuationBlock&)>& code = [](const ContinuationBlock&) {}) {
    const auto entry = generation.basic_block("continuation_block", location_info);
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
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2289-2306
LLVMValueRef evaluate_suspendable_expression(FunctionGenerationContext& generation,
    ir::expressions::IrSuspendableExpression& expression, LLVMValueRef result_slot,
    CodeContext& outer_context, const IrExpressionEvaluator& evaluate_expression) {
    const auto suspension_point_id = evaluate_expression(expression.suspension_point_id(), outer_context, nullptr);
    const auto start = generation.basic_block("start", start_location(generation, outer_context, expression.result()));
    const auto dispatch = generation.basic_block("dispatch", start_location(generation, outer_context, expression.suspension_point_id()));
    std::vector<LLVMBasicBlockRef> resume_points;
    SuspendableExpressionScope points(resume_points);
    SuspendableExpressionCodeScope scope(outer_context, points);
    return using_context(scope, [&](CodeContext& context) {
        generation.cond_br(generation.icmp_eq(suspension_point_id,
            LLVMConstNull(LLVMTypeOf(suspension_point_id))), start, dispatch);
        generation.position_at_end(start);
        const auto result = evaluate_expression(expression.result(), context, result_slot);
        generation.appending_to<void>(dispatch, [&](FunctionGenerationContext& appended) {
            appended.indirect_br(suspension_point_id, resume_points);
        });
        return result;
    });
}

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2320-2340
LLVMValueRef evaluate_suspension_point(FunctionGenerationContext& generation,
    ir::expressions::IrSuspensionPoint& expression, LLVMTypeRef result_type,
    bool is_unit, LLVMValueRef unit_instance, CodeContext& outer_context,
    const IrExpressionEvaluator& evaluate_expression) {
    const auto resume = generation.basic_block("resume", start_location(generation, outer_context, expression.resume_result()));
    const auto id = outer_context.add_resume_point(resume);
    SuspensionPointScope scope(outer_context, generation, expression.suspension_point_id_parameter(), resume, id);
    return using_context(scope, [&](CodeContext& context) {
        const auto target = continuation_block(generation, result_type, is_unit,
            start_location(generation, context, expression.result()));
        const auto normal_result = evaluate_expression(expression.result(), context, nullptr);
        jump(generation, target, normal_result);
        generation.position_at_end(resume);
        const auto resume_result = evaluate_expression(expression.resume_result(), context, nullptr);
        jump(generation, target, resume_result);
        generation.position_at_end(target.block);
        return target.value(unit_instance);
    });
}

}
