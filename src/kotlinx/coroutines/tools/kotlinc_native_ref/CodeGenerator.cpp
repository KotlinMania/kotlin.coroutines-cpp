// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:673-678,890-907,948-964,1001-1011,1208-1250,1264-1276,1461-1463,1505-1547,1566-1599
#include "CodeGenerator.hpp"
#include "IrToBitcode_coroutines.hpp"
#include <type_traits>
#include <utility>
namespace org::jetbrains::kotlin::backend::konan::llvm {
// Represents the mutable position of instructions being inserted, including
// Kotlin's handling of code generation after a terminator.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1500-1547
class FunctionGenerationContext::PositionHolder {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1505-1506
    explicit PositionHolder(LLVMContextRef context);
    ~PositionHolder();
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1509-1516,673-678
    LLVMBuilderRef get_builder();
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1528-1529
    LLVMBasicBlockRef current_block() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1518-1522
    bool is_after_terminator() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1524-1526
    void set_after_terminator();
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1531-1536
    void position_at_end(LLVMBasicBlockRef block);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1538-1542
    void position_before(LLVMValueRef instruction);
private:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1544-1546
    void dispose();
    LLVMContextRef context_;
    LLVMBuilderRef builder_;
    bool is_after_terminator_ = false;
};

// NOTE(port): The supplied LLVM value is Kotlin's function definition at this
// compiler boundary; derive its module context rather than guess from a block.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:591-600
FunctionGenerationContext::FunctionGenerationContext(LLVMValueRef function)
    : function_(function), context_(LLVMGetModuleContext(LLVMGetGlobalParent(function))),
      current_position_holder_(std::make_unique<PositionHolder>(context_)) {}
// NOTE(port): Keep the compiler-owned position holder private and dispose its
// LLVM builder when this context leaves scope.
FunctionGenerationContext::~FunctionGenerationContext() = default;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1505-1506
// NOTE(port): Kotlin's inner holder reads its enclosing LLVM context.
FunctionGenerationContext::PositionHolder::PositionHolder(LLVMContextRef context)
    : context_(context), builder_(LLVMCreateBuilderInContext(context)) {}
// NOTE(port): Dispose Kotlin's owned builder at the C++ object's lifetime end.
FunctionGenerationContext::PositionHolder::~PositionHolder() { dispose(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1509-1516,673-678
LLVMBuilderRef FunctionGenerationContext::PositionHolder::get_builder() {
    if (is_after_terminator_) {
        // NOTE(port): LocationInfo start/end maps are not supplied by this boundary.
        const auto block = LLVMInsertBasicBlockInContext(context_, current_block(), "unreachable");
        LLVMMoveBasicBlockAfter(block, current_block());
        position_at_end(block);
    }
    return builder_;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1518-1522
bool FunctionGenerationContext::PositionHolder::is_after_terminator() const { return is_after_terminator_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1524-1526
void FunctionGenerationContext::PositionHolder::set_after_terminator() { is_after_terminator_ = true; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1528-1529
LLVMBasicBlockRef FunctionGenerationContext::PositionHolder::current_block() const { return LLVMGetInsertBlock(builder_); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1531-1536
void FunctionGenerationContext::PositionHolder::position_at_end(LLVMBasicBlockRef block) {
    LLVMPositionBuilderAtEnd(builder_, block);
    // NOTE(port): Kotlin's source-location map/debugLocation dependency is unported.
    const auto last_instruction = LLVMGetLastInstruction(block);
    is_after_terminator_ = last_instruction && LLVMIsATerminatorInst(last_instruction);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1538-1542
void FunctionGenerationContext::PositionHolder::position_before(LLVMValueRef instruction) {
    LLVMPositionBuilderBefore(builder_, instruction);
    const auto previous_instruction = LLVMGetPreviousInstruction(instruction);
    is_after_terminator_ = previous_instruction && LLVMIsATerminatorInst(previous_instruction);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1544-1546
void FunctionGenerationContext::PositionHolder::dispose() { LLVMDisposeBuilder(builder_); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:592
LLVMValueRef FunctionGenerationContext::function() const { return function_; }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1577-1578
LLVMBuilderRef FunctionGenerationContext::builder() { return current_position_holder_->get_builder(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1568-1569
LLVMBasicBlockRef FunctionGenerationContext::current_block() const { return current_position_holder_->current_block(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1566
bool FunctionGenerationContext::is_after_terminator() const { return current_position_holder_->is_after_terminator(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1580
void FunctionGenerationContext::position_at_end(LLVMBasicBlockRef block) { current_position_holder_->position_at_end(block); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1582
void FunctionGenerationContext::position_before(LLVMValueRef instruction) { current_position_holder_->position_before(instruction); }
// NOTE(port): Source LocationInfo parameters and update map remain unported.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:673-678
LLVMBasicBlockRef FunctionGenerationContext::basic_block(const std::string& name) {
    const auto result = LLVMInsertBasicBlockInContext(context_, current_block(), name.c_str());
    LLVMMoveBasicBlockAfter(result, current_block());
    return result;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:890-892
LLVMValueRef FunctionGenerationContext::phi(LLVMTypeRef type, const std::string& name) {
    return LLVMBuildPhi(builder(), type, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:894-901
void FunctionGenerationContext::add_phi_incoming(LLVMValueRef phi,
    const std::vector<std::pair<LLVMBasicBlockRef, LLVMValueRef>>& incoming) {
    std::vector<LLVMValueRef> incoming_values;
    std::vector<LLVMBasicBlockRef> incoming_blocks;
    for (const auto& item : incoming) incoming_values.push_back(item.second);
    for (const auto& item : incoming) incoming_blocks.push_back(item.first);
    LLVMAddIncoming(phi, incoming_values.data(), incoming_blocks.data(), incoming.size());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:903-907
void FunctionGenerationContext::assign_phis(const std::vector<std::pair<LLVMValueRef, LLVMValueRef>>& phi_to_value) {
    for (const auto& item : phi_to_value) {
        add_phi_incoming(item.first, {{current_block(), item.second}});
    }
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:949-954
LLVMValueRef FunctionGenerationContext::br(LLVMBasicBlockRef block) {
    const auto result = LLVMBuildBr(builder(), block);
    current_position_holder_->set_after_terminator();
    return result;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:956-960
LLVMValueRef FunctionGenerationContext::cond_br(LLVMValueRef condition, LLVMBasicBlockRef bb_true, LLVMBasicBlockRef bb_false) {
    const auto result = LLVMBuildCondBr(builder(), condition, bb_true, bb_false);
    current_position_holder_->set_after_terminator();
    return result;
}
// NOTE(port): Inline LlvmFunction.Definition's LLVM value operation at this
// boundary; preserve the function binding instead of deriving it from the label.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:962-964
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:98-99
LLVMValueRef FunctionGenerationContext::block_address(LLVMBasicBlockRef block) {
    return LLVMBlockAddress(function_, block);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1001
LLVMValueRef FunctionGenerationContext::icmp_eq(LLVMValueRef left, LLVMValueRef right, const std::string& name) {
    return LLVMBuildICmp(builder(), LLVMIntEQ, left, right, name.c_str());
}
// Integer comparisons.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1003
LLVMValueRef FunctionGenerationContext::icmp_gt(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildICmp(builder(), LLVMIntSGT, arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1004
LLVMValueRef FunctionGenerationContext::icmp_ge(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildICmp(builder(), LLVMIntSGE, arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1005
LLVMValueRef FunctionGenerationContext::icmp_lt(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildICmp(builder(), LLVMIntSLT, arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1006
LLVMValueRef FunctionGenerationContext::icmp_le(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildICmp(builder(), LLVMIntSLE, arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1007
LLVMValueRef FunctionGenerationContext::icmp_ne(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildICmp(builder(), LLVMIntNE, arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1008
LLVMValueRef FunctionGenerationContext::icmp_u_lt(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildICmp(builder(), LLVMIntULT, arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1009
LLVMValueRef FunctionGenerationContext::icmp_u_le(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildICmp(builder(), LLVMIntULE, arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1010
LLVMValueRef FunctionGenerationContext::icmp_u_gt(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildICmp(builder(), LLVMIntUGT, arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1011
LLVMValueRef FunctionGenerationContext::icmp_u_ge(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildICmp(builder(), LLVMIntUGE, arg0, arg1, name.c_str());
}
// NOTE(port): Source LocationInfo maps remain unported at this LLVM boundary.
// Block creation retains the bound function and Kotlin's default block name.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1208-1235
LLVMValueRef FunctionGenerationContext::if_then_else(LLVMValueRef condition, LLVMValueRef then_value,
    const std::function<LLVMValueRef()>& else_block) {
    const auto result_type = LLVMTypeOf(then_value);
    const auto bb_exit = basic_block();
    const auto result_phi = appending_to<LLVMValueRef>(bb_exit, [&](FunctionGenerationContext& context) {
        return context.phi(result_type);
    });
    const auto bb_else = basic_block();
    cond_br(condition, bb_exit, bb_else);
    assign_phis({{result_phi, then_value}});
    appending_to<void>(bb_else, [&](FunctionGenerationContext&) {
        const auto else_value = else_block();
        br(bb_exit);
        assign_phis({{result_phi, else_value}});
    });
    position_at_end(bb_exit);
    return result_phi;
}
// NOTE(port): LocationInfo maps are not supplied by this LLVM operand boundary.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1237-1250
void FunctionGenerationContext::if_then(LLVMValueRef condition, const std::function<void()>& then_block) {
    const auto bb_exit = basic_block();
    const auto bb_then = basic_block();
    cond_br(condition, bb_then, bb_exit);
    appending_to<void>(bb_then, [&](FunctionGenerationContext&) {
        then_block();
        if (!is_after_terminator()) br(bb_exit);
    });
    position_at_end(bb_exit);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1264-1269
LLVMValueRef FunctionGenerationContext::indirect_br(LLVMValueRef address, const std::vector<LLVMBasicBlockRef>& destinations) {
    const auto branch = LLVMBuildIndirectBr(builder(), address, destinations.size());
    for (const auto destination : destinations) LLVMAddDestination(branch, destination);
    current_position_holder_->set_after_terminator();
    return branch;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1271-1276
LLVMValueRef FunctionGenerationContext::switch_(LLVMValueRef value,
    const std::vector<std::pair<LLVMValueRef, LLVMBasicBlockRef>>& cases, LLVMBasicBlockRef else_block) {
    const auto result = LLVMBuildSwitch(builder(), value, else_block, cases.size());
    for (const auto& item : cases) LLVMAddCase(result, item.first, item.second);
    current_position_holder_->set_after_terminator();
    return result;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1461-1463
LLVMValueRef FunctionGenerationContext::raw_ret(LLVMValueRef value) {
    const auto result = LLVMBuildRet(builder(), value);
    current_position_holder_->set_after_terminator();
    return result;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1584-1594
template <typename R>
R FunctionGenerationContext::preserving_position(const std::function<R()>& code) {
    auto new_position_holder = std::make_unique<PositionHolder>(context_);
    auto old_position_holder = std::move(current_position_holder_);
    current_position_holder_ = std::move(new_position_holder);
    try {
        if constexpr (std::is_void_v<R>) {
            code();
            new_position_holder = std::move(current_position_holder_);
            current_position_holder_ = std::move(old_position_holder);
        } else {
            auto result = code();
            new_position_holder = std::move(current_position_holder_);
            current_position_holder_ = std::move(old_position_holder);
            return result;
        }
    } catch (...) {
        new_position_holder = std::move(current_position_holder_);
        current_position_holder_ = std::move(old_position_holder);
        throw;
    }
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1596-1599
template <typename R>
R FunctionGenerationContext::appending_to(LLVMBasicBlockRef block,
    const std::function<R(FunctionGenerationContext&)>& code) {
    return preserving_position<R>([&]() -> R {
        position_at_end(block);
        return code(*this);
    });
}
// NOTE(port): Explicitly instantiate every return type used at this LLVM boundary;
// keep the generic Kotlin operation's implementation out of the public header.
template void FunctionGenerationContext::appending_to<void>(LLVMBasicBlockRef,
    const std::function<void(FunctionGenerationContext&)>&);
template LLVMValueRef FunctionGenerationContext::appending_to<LLVMValueRef>(LLVMBasicBlockRef,
    const std::function<LLVMValueRef(FunctionGenerationContext&)>&);
template ContinuationBlock FunctionGenerationContext::appending_to<ContinuationBlock>(LLVMBasicBlockRef,
    const std::function<ContinuationBlock(FunctionGenerationContext&)>&);
}
