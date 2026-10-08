// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:74-78,390-393,616-621,667-678,720-732,754-758,890-907,948-964,1001-1042,1208-1262,1264-1276,1342-1347,1461-1463,1505-1558,1566-1599
#include "CodeGenerator.hpp"
#include <llvm-c/DebugInfo.h>
#include <map>
#include "IrToBitcode_coroutines.hpp"
#include <type_traits>
#include <utility>
namespace org::jetbrains::kotlin::backend::konan::llvm {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2884-2887
LocationInfo::LocationInfo(LLVMMetadataRef scope, int line, int column, LocationInfo* inlined_at)
    : scope(scope), line(line), column(column), inlined_at(inlined_at) {}

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:616-621
// NOTE(port): Each map update owns its range object. Shared handles preserve
// Kotlin's returned range identity across later replacement of the map entry.
class FunctionGenerationContext::DebugLocationState {
public:
    std::map<LLVMBasicBlockRef, std::shared_ptr<LocationInfoRange>> locations;
};
namespace {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:74-78
// NOTE(port): LLVM-C represents both Native location constructors with the same
// metadata API. The enclosing compiler's LLVM context is supplied explicitly.
LLVMMetadataRef generate_location_info(LLVMContextRef context, const LocationInfo& location) {
    const auto inlined_at = location.inlined_at ? generate_location_info(context, *location.inlined_at) : nullptr;
    return LLVMDIBuilderCreateDebugLocation(context, location.line, location.column, location.scope, inlined_at);
}
}

// Represents the mutable position of instructions being inserted, including
// Kotlin's handling of code generation after a terminator.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1500-1547
class FunctionGenerationContext::PositionHolder {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1505-1506
    explicit PositionHolder(FunctionGenerationContext& generation);
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
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1548-1551
    void reset_builder_debug_location();
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1553-1556
    void set_builder_debug_location(LLVMMetadataRef location);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1558-1558
    bool has_debug_location() const;
private:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1544-1546
    void dispose();
    FunctionGenerationContext& generation_;
    LLVMContextRef context_;
    LLVMBuilderRef builder_;
    bool is_after_terminator_ = false;
};

// NOTE(port): The supplied LLVM value is Kotlin's function definition at this
// compiler boundary; derive its module context rather than guess from a block.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:591-600
FunctionGenerationContext::FunctionGenerationContext(LLVMValueRef function)
    : FunctionGenerationContext(function, false) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:591-615
FunctionGenerationContext::FunctionGenerationContext(LLVMValueRef function, bool contain_location_debug_info)
    : function_(function), context_(LLVMGetModuleContext(LLVMGetGlobalParent(function))),
      contain_location_debug_info_(contain_location_debug_info), debug_locations_(std::make_unique<DebugLocationState>()),
      current_position_holder_(std::make_unique<PositionHolder>(*this)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:591-600
FunctionGenerationContext::FunctionGenerationContext(const LlvmFunction::Definition& function, bool contain_location_debug_info)
    : FunctionGenerationContext(function.as_callback(), contain_location_debug_info) {
    definition_ = &function;
}
// NOTE(port): Supplied compiler policy, rather than inferred LLVM metadata.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/ConfigChecks.kt:30-30
bool FunctionGenerationContext::should_contain_location_debug_info() const { return contain_location_debug_info_; }
// NOTE(port): Keep the compiler-owned position holder private and dispose its
// LLVM builder when this context leaves scope.
FunctionGenerationContext::~FunctionGenerationContext() = default;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1505-1506
// NOTE(port): Kotlin's inner holder reads its enclosing LLVM context.
FunctionGenerationContext::PositionHolder::PositionHolder(FunctionGenerationContext& generation)
    : generation_(generation), context_(generation.context_), builder_(LLVMCreateBuilderInContext(context_)) {}
// NOTE(port): Dispose Kotlin's owned builder at the C++ object's lifetime end.
FunctionGenerationContext::PositionHolder::~PositionHolder() { dispose(); }
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1509-1516,673-678
LLVMBuilderRef FunctionGenerationContext::PositionHolder::get_builder() {
    if (is_after_terminator_) {
        const auto position = generation_.position();
        const auto block = generation_.basic_block("unreachable",
            position ? position->start : nullptr, position ? position->end : nullptr);
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
    auto found = generation_.debug_locations_->locations.find(block);
    if (found != generation_.debug_locations_->locations.end())
        generation_.debug_location(*found->second->start, found->second->end);
    const auto last_instruction = LLVMGetLastInstruction(block);
    is_after_terminator_ = last_instruction && LLVMIsATerminatorInst(last_instruction);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1538-1542
void FunctionGenerationContext::PositionHolder::position_before(LLVMValueRef instruction) {
    LLVMPositionBuilderBefore(builder_, instruction);
    const auto previous_instruction = LLVMGetPreviousInstruction(instruction);
    is_after_terminator_ = previous_instruction && LLVMIsATerminatorInst(previous_instruction);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1548-1551
void FunctionGenerationContext::PositionHolder::reset_builder_debug_location() {
    if (generation_.contain_location_debug_info_) LLVMSetCurrentDebugLocation2(builder_, nullptr);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1553-1556
void FunctionGenerationContext::PositionHolder::set_builder_debug_location(LLVMMetadataRef location) {
    if (generation_.contain_location_debug_info_) LLVMSetCurrentDebugLocation2(builder_, location);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1558-1558
bool FunctionGenerationContext::PositionHolder::has_debug_location() const {
    return generation_.contain_location_debug_info_ && LLVMGetCurrentDebugLocation2(builder_) != nullptr;
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
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:618-621
void FunctionGenerationContext::update(LLVMBasicBlockRef block, LocationInfo* start_location, LocationInfo* end_location) {
    if (!start_location) return;
    debug_locations_->locations[block] = std::make_shared<LocationInfoRange>(LocationInfoRange{start_location, end_location});
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:667-671
LLVMBasicBlockRef FunctionGenerationContext::basic_block_in_function(const std::string& name, LocationInfo* location_info) {
    const auto block = definition_ ? definition_->add_basic_block(context_, name)
        : LLVMAppendBasicBlockInContext(context_, function_, name.c_str());
    update(block, location_info, location_info);
    return block;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:673-678
LLVMBasicBlockRef FunctionGenerationContext::basic_block(const std::string& name) {
    return basic_block(name, nullptr, nullptr);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:673-678
LLVMBasicBlockRef FunctionGenerationContext::basic_block(const std::string& name, LocationInfo* start_location) {
    return basic_block(name, start_location, start_location);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:673-678
LLVMBasicBlockRef FunctionGenerationContext::basic_block(const std::string& name, LocationInfo* start_location, LocationInfo* end_location) {
    const auto result = LLVMInsertBasicBlockInContext(context_, current_block(), name.c_str());
    update(result, start_location, end_location);
    LLVMMoveBasicBlockAfter(result, current_block());
    return result;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1252-1262
void FunctionGenerationContext::debug_location(LocationInfo& start_location, LocationInfo* end_location) {
    if (!contain_location_debug_info_) return;
    if (start_location.line != 0 || debug_locations_->locations.find(current_block()) == debug_locations_->locations.end())
        update(current_block(), &start_location, end_location);
    if (start_location.line != 0 || !current_position_holder_->has_debug_location())
        current_position_holder_->set_builder_debug_location(generate_location_info(context_, start_location));
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1342-1345
void FunctionGenerationContext::reset_debug_location() {
    if (!contain_location_debug_info_) return;
    current_position_holder_->reset_builder_debug_location();
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1347-1347
std::shared_ptr<LocationInfoRange> FunctionGenerationContext::position() const {
    const auto found = debug_locations_->locations.find(current_block());
    return found == debug_locations_->locations.end() ? nullptr : found->second;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:720-720
LLVMValueRef FunctionGenerationContext::param(int index) const {
    if (definition_) return definition_->param(index);
    return LLVMGetParam(function_, index);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:722-726
LLVMValueRef FunctionGenerationContext::apply_memory_order_and_alignment(LLVMValueRef value,
    std::optional<LLVMAtomicOrdering> memory_order, std::optional<int> alignment) {
    if (memory_order) LLVMSetOrdering(value, *memory_order);
    if (alignment) LLVMSetAlignment(value, *alignment);
    return value;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:728-732
LLVMValueRef FunctionGenerationContext::load(LLVMTypeRef type, LLVMValueRef address, const std::string& name,
    std::optional<LLVMAtomicOrdering> memory_order, std::optional<int> alignment) {
    return apply_memory_order_and_alignment(LLVMBuildLoad2(builder(), type, address, name.c_str()), memory_order, alignment);
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:754-758
void FunctionGenerationContext::store(LLVMValueRef value, LLVMValueRef pointer,
    std::optional<LLVMAtomicOrdering> memory_order, std::optional<int> alignment) {
    const auto instruction = LLVMBuildStore(builder(), value, pointer);
    if (memory_order) LLVMSetOrdering(instruction, *memory_order);
    if (alignment) LLVMSetAlignment(instruction, *alignment);
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
// NOTE(port): The LLVM operand adapter retains its existing function binding;
// typed compiler callers bind the actual translated LlvmFunction.Definition.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:962-964
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:98-99
LLVMValueRef FunctionGenerationContext::block_address(LLVMBasicBlockRef block) {
    if (definition_) return definition_->block_address(block);
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
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1014-1014
LLVMValueRef FunctionGenerationContext::fcmp_eq(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildFCmp(builder(), LLVMRealOEQ, arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1015-1015
LLVMValueRef FunctionGenerationContext::fcmp_gt(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildFCmp(builder(), LLVMRealOGT, arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1016-1016
LLVMValueRef FunctionGenerationContext::fcmp_ge(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildFCmp(builder(), LLVMRealOGE, arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1017-1017
LLVMValueRef FunctionGenerationContext::fcmp_lt(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildFCmp(builder(), LLVMRealOLT, arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1018-1018
LLVMValueRef FunctionGenerationContext::fcmp_le(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildFCmp(builder(), LLVMRealOLE, arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1020-1020
LLVMValueRef FunctionGenerationContext::sub(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildSub(builder(), arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1021-1021
LLVMValueRef FunctionGenerationContext::add(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildAdd(builder(), arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1023-1023
LLVMValueRef FunctionGenerationContext::fsub(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildFSub(builder(), arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1024-1024
LLVMValueRef FunctionGenerationContext::fadd(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name) {
    return LLVMBuildFAdd(builder(), arg0, arg1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1025-1025
LLVMValueRef FunctionGenerationContext::fneg(LLVMValueRef arg, const std::string& name) {
    return LLVMBuildFNeg(builder(), arg, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1027-1028
LLVMValueRef FunctionGenerationContext::select(LLVMValueRef if_value, LLVMValueRef then_value, LLVMValueRef else_value, const std::string& name) {
    return LLVMBuildSelect(builder(), if_value, then_value, else_value, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1030-1030
LLVMValueRef FunctionGenerationContext::bitcast(LLVMTypeRef type, LLVMValueRef value, const std::string& name) {
    return LLVMBuildBitCast(builder(), value, type, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1032-1032
LLVMValueRef FunctionGenerationContext::int_to_ptr(LLVMValueRef value, LLVMTypeRef dest_type, const std::string& name) {
    return LLVMBuildIntToPtr(builder(), value, dest_type, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1033-1033
LLVMValueRef FunctionGenerationContext::ptr_to_int(LLVMValueRef value, LLVMTypeRef dest_type, const std::string& name) {
    return LLVMBuildPtrToInt(builder(), value, dest_type, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1035-1036
LLVMValueRef FunctionGenerationContext::gep(LLVMTypeRef type, LLVMValueRef base, LLVMValueRef index, const std::string& name) {
    return LLVMBuildGEP2(builder(), type, base, &index, 1, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1038-1039
LLVMValueRef FunctionGenerationContext::struct_gep(LLVMTypeRef type, LLVMValueRef base, int index, const std::string& name) {
    return LLVMBuildStructGEP2(builder(), type, base, index, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1041-1042
LLVMValueRef FunctionGenerationContext::extract_value(LLVMValueRef aggregate, int index, const std::string& name) {
    return LLVMBuildExtractValue(builder(), aggregate, index, name.c_str());
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1208-1235
LLVMValueRef FunctionGenerationContext::if_then_else(LLVMValueRef condition, LLVMValueRef then_value,
    const std::function<LLVMValueRef()>& else_block) {
    const auto result_type = LLVMTypeOf(then_value);
    const auto source_position = position();
    const auto current_position = position();
    const auto end_position = current_position ? current_position->end : nullptr;
    const auto bb_exit = basic_block("label_", end_position);
    const auto result_phi = appending_to<LLVMValueRef>(bb_exit, [&](FunctionGenerationContext& context) {
        return context.phi(result_type);
    });
    const auto bb_else = basic_block("label_", source_position ? source_position->start : nullptr, end_position);
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
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1237-1250
void FunctionGenerationContext::if_then(LLVMValueRef condition, const std::function<void()>& then_block) {
    const auto source_position = position();
    const auto end_position = source_position ? source_position->end : nullptr;
    const auto bb_exit = basic_block("label_", end_position);
    const auto bb_then = basic_block("label_", end_position);
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
    auto new_position_holder = std::make_unique<PositionHolder>(*this);
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
