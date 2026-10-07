#include "IrToBitcode_coroutines.hpp"
#include "CodeGenerator.hpp"
#include <llvm-c/Analysis.h>
#include <cassert>
#include <vector>
#include <stdexcept>
using namespace org::jetbrains::kotlin::backend::konan::llvm;
// Test-only access declarations retain Kotlin's protected production method.
class TestFunctionGenerationContext final : public FunctionGenerationContext {
public:
    using FunctionGenerationContext::FunctionGenerationContext;
    using FunctionGenerationContext::raw_ret;
};
int main(int argc, char** argv) {
    assert(argc == 2);
    const auto context = LLVMContextCreate();
    const auto module = LLVMModuleCreateWithNameInContext("coroutine_codegen", context);
    {
    const auto integer = LLVMInt32TypeInContext(context);
    const auto pointer = LLVMPointerTypeInContext(context, 0);
    LLVMTypeRef arguments[] = {pointer, integer, integer};
    for (bool unit : {false, true}) {
        const auto function = LLVMAddFunction(module, unit ? "unit_frame" : "value_frame",
            LLVMFunctionType(integer, arguments, 3, false));
        TestFunctionGenerationContext generation(function);
        assert(generation.function() == function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        // Block addresses belong to the bound function even before positioning.
        const auto address_probe = LLVMAppendBasicBlockInContext(context, function, "address_probe");
        assert(generation.block_address(address_probe) == LLVMBlockAddress(function, address_probe));
        generation.appending_to<void>(address_probe, [&](FunctionGenerationContext&) {
            generation.raw_ret(LLVMConstInt(integer, 0, false));
        });
        generation.position_at_end(entry);
        const auto label_field = LLVMGetParam(function, 0);
        const auto label = LLVMBuildLoad2(generation.builder(), pointer, label_field, "label");
        std::vector<LLVMBasicBlockRef> resume_points;
        SuspendableExpressionScope scope(resume_points);
        const auto result = evaluate_suspendable_expression(generation, label, nullptr, resume_points,
            [&](LLVMValueRef) {
                return evaluate_suspension_point(generation, integer, unit, LLVMConstInt(integer, 0, false), scope,
                    [&](LLVMValueRef resume_address) {
                        LLVMBuildStore(generation.builder(), resume_address, label_field);
                        return LLVMGetParam(function, 1);
                    }, [&] { return LLVMGetParam(function, 2); });
            });
        generation.raw_ret(result);
        assert(resume_points.size() == 1);
    }
    {
        LLVMTypeRef parameters[] = {pointer, integer, integer, integer};
        const auto function = LLVMAddFunction(module, "two_point_frame",
            LLVMFunctionType(integer, parameters, 4, false));
        TestFunctionGenerationContext generation(function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        generation.position_at_end(entry);
        const auto label_field = LLVMGetParam(function, 0);
        const auto label = LLVMBuildLoad2(generation.builder(), pointer, label_field, "label");
        std::vector<LLVMBasicBlockRef> resume_points;
        SuspendableExpressionScope scope(resume_points);
        const auto result = evaluate_suspendable_expression(generation, label, nullptr, resume_points,
            [&](LLVMValueRef) {
                const auto first = evaluate_suspension_point(generation, integer, false, nullptr, scope,
                    [&](LLVMValueRef resume_address) {
                        LLVMBuildStore(generation.builder(), resume_address, label_field);
                        generation.raw_ret(LLVMGetParam(function, 1));
                        return LLVMGetParam(function, 1);
                    }, [&] { return LLVMGetParam(function, 2); });
                size_t name_length = 0;
                LLVMGetValueName2(first, &name_length);
                assert(name_length == 0);
                return evaluate_suspension_point(generation, integer, false, nullptr, scope,
                    [&](LLVMValueRef resume_address) {
                        LLVMBuildStore(generation.builder(), resume_address, label_field);
                        generation.raw_ret(first);
                        return first;
                    }, [&] { return LLVMGetParam(function, 3); });
            });
        generation.raw_ret(result);
        assert(resume_points.size() == 2 && resume_points[0] != resume_points[1]);
        const auto condition = LLVMGetCondition(LLVMGetBasicBlockTerminator(entry));
        size_t name_length = 0;
        LLVMGetValueName2(condition, &name_length);
        assert(name_length == 0);
        LLVMBasicBlockRef dispatch = nullptr;
        for (auto block = LLVMGetFirstBasicBlock(function); block; block = LLVMGetNextBasicBlock(block)) {
            const auto terminator = LLVMGetBasicBlockTerminator(block);
            if (terminator && LLVMGetInstructionOpcode(terminator) == LLVMIndirectBr) dispatch = block;
        }
        assert(dispatch);
        const auto branch = LLVMGetBasicBlockTerminator(dispatch);
        assert(LLVMGetNumSuccessors(branch) == resume_points.size());
        for (unsigned index = 0; index < resume_points.size(); ++index)
            assert(LLVMGetSuccessor(branch, index) == resume_points[index]);
    }
    {
        const auto function = LLVMAddFunction(module, "terminated_normal_frame",
            LLVMFunctionType(integer, arguments, 3, false));
        TestFunctionGenerationContext generation(function);
        assert(generation.function() == function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        const auto early_exit = LLVMAppendBasicBlockInContext(context, function, "early_exit");
        generation.position_at_end(entry);
        generation.appending_to<void>(early_exit, [&](FunctionGenerationContext&) {
            generation.raw_ret(LLVMConstInt(integer, 13, false));
        });
        const auto label_field = LLVMGetParam(function, 0);
        const auto label = LLVMBuildLoad2(generation.builder(), pointer, label_field, "label");
        std::vector<LLVMBasicBlockRef> resume_points;
        SuspendableExpressionScope scope(resume_points);
        const auto result = evaluate_suspendable_expression(generation, label, nullptr, resume_points,
            [&](LLVMValueRef) {
                return evaluate_suspension_point(generation, integer, false, nullptr, scope,
                    [&](LLVMValueRef resume_address) {
                        LLVMBuildStore(generation.builder(), resume_address, label_field);
                        generation.br(early_exit);
                        return LLVMGetParam(function, 1);
                    }, [&] { return LLVMGetParam(function, 2); });
            });
        assert(LLVMCountIncoming(result) == 2);
        const auto normal_predecessor = LLVMGetIncomingBlock(result, 0);
        const auto normal_terminator = LLVMGetBasicBlockTerminator(normal_predecessor);
        assert(LLVMGetSuccessor(normal_terminator, 0) == LLVMGetInstructionParent(result));
        assert(normal_predecessor != entry && normal_predecessor != early_exit);
        generation.raw_ret(result);
    }
    {
        const auto function = LLVMAddFunction(module, "position_state", LLVMFunctionType(integer, nullptr, 0, false));
        TestFunctionGenerationContext generation(function);
        assert(generation.function() == function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        const auto next = LLVMAppendBasicBlockInContext(context, function, "next");
        generation.position_at_end(entry);
        generation.br(next);
        assert(generation.is_after_terminator() && generation.current_block() == entry);
        generation.position_at_end(next);
        assert(!generation.is_after_terminator());
        const auto slot = LLVMBuildAlloca(generation.builder(), integer, "slot");
        const auto returned = generation.raw_ret(LLVMConstInt(integer, 22, false));
        assert(generation.is_after_terminator());
        generation.position_at_end(next);
        assert(generation.is_after_terminator());
        generation.position_before(returned);
        assert(!generation.is_after_terminator());
        const auto original_builder = generation.builder();
        const auto side = LLVMAppendBasicBlockInContext(context, function, "side");
        const auto scoped_value = generation.appending_to<LLVMValueRef>(side, [&](FunctionGenerationContext& scoped) {
            assert(scoped.builder() != original_builder);
            return generation.raw_ret(LLVMConstInt(integer, 7, false));
        });
        assert(LLVMIsAReturnInst(scoped_value));
        assert(generation.builder() == original_builder && generation.current_block() == next);
        assert(!generation.is_after_terminator());
        const auto throwing = LLVMAppendBasicBlockInContext(context, function, "throwing");
        bool caught = false;
        try {
            generation.appending_to<void>(throwing, [&](FunctionGenerationContext& scoped) {
                scoped.br(side);
                throw std::runtime_error("generation interrupted");
            });
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught && generation.builder() == original_builder && generation.current_block() == next);
        assert(!generation.is_after_terminator());
        LLVMBuildStore(generation.builder(), LLVMConstInt(integer, 22, false), slot);
        generation.position_at_end(next);
        assert(generation.is_after_terminator());
        generation.builder();
        assert(generation.current_block() != next && !generation.is_after_terminator());
        generation.raw_ret(LLVMConstInt(integer, 0, false));
    }
    {
        LLVMTypeRef parameters[] = {integer, integer, integer, pointer};
        const auto function = LLVMAddFunction(module, "branching_value",
            LLVMFunctionType(integer, parameters, 4, false));
        TestFunctionGenerationContext generation(function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        const auto anchor = LLVMAppendBasicBlockInContext(context, function, "anchor");
        generation.position_at_end(entry);
        generation.appending_to<void>(anchor, [&](FunctionGenerationContext&) {
            generation.raw_ret(LLVMConstInt(integer, 0, false));
        });
        const auto condition = generation.icmp_eq(LLVMGetParam(function, 0), LLVMConstInt(integer, 1, false));
        const auto result = generation.if_then_else(condition, LLVMGetParam(function, 1), [&] {
            const auto effects = LLVMGetParam(function, 3);
            const auto count = LLVMBuildLoad2(generation.builder(), integer, effects, "count");
            const auto incremented = LLVMBuildAdd(generation.builder(), count, LLVMConstInt(integer, 1, false), "incremented");
            LLVMBuildStore(generation.builder(), incremented, effects);
            return LLVMGetParam(function, 2);
        });
        const auto exit = LLVMGetInstructionParent(result);
        const auto alternative = LLVMGetNextBasicBlock(entry);
        assert(LLVMGetNextBasicBlock(alternative) == exit && LLVMGetNextBasicBlock(exit) == anchor);
        assert(LLVMCountIncoming(result) == 2);
        generation.raw_ret(result);
    }
    for (int termination : {0, 1, 2}) {
        LLVMTypeRef parameters[] = {integer, pointer};
        const auto function = LLVMAddFunction(module, termination == 2 ? "branching_returned_effect" :
            termination == 1 ? "branching_terminated_effect" : "branching_effect",
            LLVMFunctionType(integer, parameters, 2, false));
        TestFunctionGenerationContext generation(function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        const auto early_exit = LLVMAppendBasicBlockInContext(context, function, "early_exit");
        generation.position_at_end(entry);
        generation.appending_to<void>(early_exit, [&](FunctionGenerationContext&) {
            generation.raw_ret(LLVMConstInt(integer, 29, false));
        });
        const auto condition = generation.icmp_eq(LLVMGetParam(function, 0), LLVMConstInt(integer, 1, false));
        generation.if_then(condition, [&] {
            LLVMBuildStore(generation.builder(), LLVMConstInt(integer, 17, false), LLVMGetParam(function, 1));
            if (termination == 1) generation.br(early_exit);
            if (termination == 2) generation.raw_ret(LLVMConstInt(integer, 29, false));
        });
        generation.raw_ret(LLVMConstInt(integer, 9, false));
    }
    {
        LLVMTypeRef parameters[] = {integer};
        const auto function = LLVMAddFunction(module, "initializer_switch_dispatch",
            LLVMFunctionType(integer, parameters, 1, false));
        TestFunctionGenerationContext generation(function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        generation.position_at_end(entry);
        const auto init = generation.basic_block("init");
        const auto local_init = generation.basic_block("local_init");
        const auto local_alloc = generation.basic_block("local_alloc");
        const auto otherwise = generation.basic_block("default");
        const std::vector<std::pair<LLVMValueRef, LLVMBasicBlockRef>> cases = {
            {LLVMConstInt(integer, 1, false), init},
            {LLVMConstInt(integer, 2, false), local_init},
            {LLVMConstInt(integer, 0, false), local_alloc}};
        const auto dispatch = generation.switch_(LLVMGetParam(function, 0), cases, otherwise);
        assert(generation.is_after_terminator() && generation.current_block() == entry);
        assert(LLVMGetSwitchDefaultDest(dispatch) == otherwise && LLVMGetNumSuccessors(dispatch) == 4);
        for (unsigned index = 0; index < cases.size(); ++index)
            assert(LLVMGetSuccessor(dispatch, index + 1) == cases[index].second);
        // Source builder requests after dispatch create a separate unreachable block.
        generation.builder();
        assert(generation.current_block() != entry && !generation.is_after_terminator());
        assert(LLVMGetNextBasicBlock(entry) == generation.current_block());
        generation.raw_ret(LLVMConstInt(integer, 0, false));
        generation.appending_to<void>(init, [&](FunctionGenerationContext&) {
            generation.raw_ret(LLVMConstInt(integer, 17, false));
        });
        generation.appending_to<void>(local_init, [&](FunctionGenerationContext&) {
            generation.raw_ret(LLVMConstInt(integer, 19, false));
        });
        generation.appending_to<void>(local_alloc, [&](FunctionGenerationContext&) {
            generation.raw_ret(LLVMConstInt(integer, 13, false));
        });
        generation.appending_to<void>(otherwise, [&](FunctionGenerationContext& scoped) {
            LLVMBuildUnreachable(scoped.builder());
        });
    }
    {
        LLVMTypeRef parameters[] = {integer, integer};
        const auto function = LLVMAddFunction(module, "comparison_eq",
            LLVMFunctionType(integer, parameters, 2, false));
        TestFunctionGenerationContext generation(function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        generation.position_at_end(entry);
        const auto comparison = generation.icmp_eq(LLVMGetParam(function, 0), LLVMGetParam(function, 1), "comparison");
        assert(LLVMGetICmpPredicate(comparison) == LLVMIntEQ);
        assert(!generation.is_after_terminator());
        generation.raw_ret(LLVMBuildZExt(generation.builder(), comparison, integer, "result"));
    }
    {
        LLVMTypeRef parameters[] = {integer, integer};
        const auto function = LLVMAddFunction(module, "comparison_gt",
            LLVMFunctionType(integer, parameters, 2, false));
        TestFunctionGenerationContext generation(function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        generation.position_at_end(entry);
        const auto comparison = generation.icmp_gt(LLVMGetParam(function, 0), LLVMGetParam(function, 1), "comparison");
        assert(LLVMGetICmpPredicate(comparison) == LLVMIntSGT);
        assert(!generation.is_after_terminator());
        generation.raw_ret(LLVMBuildZExt(generation.builder(), comparison, integer, "result"));
    }
    {
        LLVMTypeRef parameters[] = {integer, integer};
        const auto function = LLVMAddFunction(module, "comparison_ge",
            LLVMFunctionType(integer, parameters, 2, false));
        TestFunctionGenerationContext generation(function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        generation.position_at_end(entry);
        const auto comparison = generation.icmp_ge(LLVMGetParam(function, 0), LLVMGetParam(function, 1), "comparison");
        assert(LLVMGetICmpPredicate(comparison) == LLVMIntSGE);
        assert(!generation.is_after_terminator());
        generation.raw_ret(LLVMBuildZExt(generation.builder(), comparison, integer, "result"));
    }
    {
        LLVMTypeRef parameters[] = {integer, integer};
        const auto function = LLVMAddFunction(module, "comparison_lt",
            LLVMFunctionType(integer, parameters, 2, false));
        TestFunctionGenerationContext generation(function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        generation.position_at_end(entry);
        const auto comparison = generation.icmp_lt(LLVMGetParam(function, 0), LLVMGetParam(function, 1), "comparison");
        assert(LLVMGetICmpPredicate(comparison) == LLVMIntSLT);
        assert(!generation.is_after_terminator());
        generation.raw_ret(LLVMBuildZExt(generation.builder(), comparison, integer, "result"));
    }
    {
        LLVMTypeRef parameters[] = {integer, integer};
        const auto function = LLVMAddFunction(module, "comparison_le",
            LLVMFunctionType(integer, parameters, 2, false));
        TestFunctionGenerationContext generation(function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        generation.position_at_end(entry);
        const auto comparison = generation.icmp_le(LLVMGetParam(function, 0), LLVMGetParam(function, 1), "comparison");
        assert(LLVMGetICmpPredicate(comparison) == LLVMIntSLE);
        assert(!generation.is_after_terminator());
        generation.raw_ret(LLVMBuildZExt(generation.builder(), comparison, integer, "result"));
    }
    {
        LLVMTypeRef parameters[] = {integer, integer};
        const auto function = LLVMAddFunction(module, "comparison_ne",
            LLVMFunctionType(integer, parameters, 2, false));
        TestFunctionGenerationContext generation(function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        generation.position_at_end(entry);
        const auto comparison = generation.icmp_ne(LLVMGetParam(function, 0), LLVMGetParam(function, 1), "comparison");
        assert(LLVMGetICmpPredicate(comparison) == LLVMIntNE);
        assert(!generation.is_after_terminator());
        generation.raw_ret(LLVMBuildZExt(generation.builder(), comparison, integer, "result"));
    }
    {
        LLVMTypeRef parameters[] = {integer, integer};
        const auto function = LLVMAddFunction(module, "comparison_u_lt",
            LLVMFunctionType(integer, parameters, 2, false));
        TestFunctionGenerationContext generation(function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        generation.position_at_end(entry);
        const auto comparison = generation.icmp_u_lt(LLVMGetParam(function, 0), LLVMGetParam(function, 1), "comparison");
        assert(LLVMGetICmpPredicate(comparison) == LLVMIntULT);
        assert(!generation.is_after_terminator());
        generation.raw_ret(LLVMBuildZExt(generation.builder(), comparison, integer, "result"));
    }
    {
        LLVMTypeRef parameters[] = {integer, integer};
        const auto function = LLVMAddFunction(module, "comparison_u_le",
            LLVMFunctionType(integer, parameters, 2, false));
        TestFunctionGenerationContext generation(function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        generation.position_at_end(entry);
        const auto comparison = generation.icmp_u_le(LLVMGetParam(function, 0), LLVMGetParam(function, 1), "comparison");
        assert(LLVMGetICmpPredicate(comparison) == LLVMIntULE);
        assert(!generation.is_after_terminator());
        generation.raw_ret(LLVMBuildZExt(generation.builder(), comparison, integer, "result"));
    }
    {
        LLVMTypeRef parameters[] = {integer, integer};
        const auto function = LLVMAddFunction(module, "comparison_u_gt",
            LLVMFunctionType(integer, parameters, 2, false));
        TestFunctionGenerationContext generation(function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        generation.position_at_end(entry);
        const auto comparison = generation.icmp_u_gt(LLVMGetParam(function, 0), LLVMGetParam(function, 1), "comparison");
        assert(LLVMGetICmpPredicate(comparison) == LLVMIntUGT);
        assert(!generation.is_after_terminator());
        generation.raw_ret(LLVMBuildZExt(generation.builder(), comparison, integer, "result"));
    }
    {
        LLVMTypeRef parameters[] = {integer, integer};
        const auto function = LLVMAddFunction(module, "comparison_u_ge",
            LLVMFunctionType(integer, parameters, 2, false));
        TestFunctionGenerationContext generation(function);
        const auto entry = LLVMAppendBasicBlockInContext(context, function, "entry");
        generation.position_at_end(entry);
        const auto comparison = generation.icmp_u_ge(LLVMGetParam(function, 0), LLVMGetParam(function, 1), "comparison");
        assert(LLVMGetICmpPredicate(comparison) == LLVMIntUGE);
        assert(!generation.is_after_terminator());
        generation.raw_ret(LLVMBuildZExt(generation.builder(), comparison, integer, "result"));
    }
    char* error = nullptr;
    const auto invalid = LLVMVerifyModule(module, LLVMReturnStatusAction, &error);
    LLVMDisposeMessage(error);
    assert(!invalid);
    assert(!LLVMPrintModuleToFile(module, argv[1], &error));
    }
    LLVMDisposeModule(module);
    LLVMContextDispose(context);
}
