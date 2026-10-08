// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:390-393,616-621,673-678,720-732,754-758,890-907,948-964,1001-1011,1208-1262,1264-1276,1342-1347,1461-1463,1505-1558,1566-1599
#pragma once
#include "LocationInfo.hpp"
#include <llvm-c/Core.h>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <utility>
#include <optional>
namespace org::jetbrains::kotlin::backend::konan::llvm {
// NOTE(port): The LLVM boundary retains native values and source locations.
// Runtime frame generation and the complete enclosing compiler context are
// separate translated dependencies.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:591-615
class FunctionGenerationContext {
public:
    // NOTE(port): Bind Kotlin's function definition to the compiler-owned LLVM
    // value. Full CodeGenerator and runtime-frame initialization remain unported.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:591-600
    explicit FunctionGenerationContext(LLVMValueRef function);
    // NOTE(port): The caller supplies the owning compiler context's
    // shouldContainLocationDebugInfo result at this LLVM boundary.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:591-615
    FunctionGenerationContext(LLVMValueRef function, bool contain_location_debug_info);
    // NOTE(port): Expose that supplied policy to the typed IR expression boundary.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/ConfigChecks.kt:30-30
    bool should_contain_location_debug_info() const;
    ~FunctionGenerationContext();
    FunctionGenerationContext(const FunctionGenerationContext&) = delete;
    FunctionGenerationContext& operator=(const FunctionGenerationContext&) = delete;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:592
    LLVMValueRef function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1577-1578
    LLVMBuilderRef builder();
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1568-1569
    LLVMBasicBlockRef current_block() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1566
    bool is_after_terminator() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1580
    void position_at_end(LLVMBasicBlockRef block);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1582
    void position_before(LLVMValueRef instruction);
    // NOTE(port): The LLVM operand adapter has no source location argument.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:673-678
    LLVMBasicBlockRef basic_block(const std::string& name = "label_");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:673-678
    LLVMBasicBlockRef basic_block(const std::string& name, LocationInfo* start_location);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:673-678
    LLVMBasicBlockRef basic_block(const std::string& name, LocationInfo* start_location, LocationInfo* end_location);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1252-1262
    void debug_location(LocationInfo& start_location, LocationInfo* end_location);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1342-1345
    void reset_debug_location();
    // NOTE(port): Retain the returned range's identity when the map replaces it.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1347-1347
    std::shared_ptr<LocationInfoRange> position() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:720-720
    LLVMValueRef param(int index) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:728-732
    LLVMValueRef load(LLVMTypeRef type, LLVMValueRef address, const std::string& name = "",
        std::optional<LLVMAtomicOrdering> memory_order = std::nullopt,
        std::optional<int> alignment = std::nullopt);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:754-758
    void store(LLVMValueRef value, LLVMValueRef pointer,
        std::optional<LLVMAtomicOrdering> memory_order = std::nullopt,
        std::optional<int> alignment = std::nullopt);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:890-892
    LLVMValueRef phi(LLVMTypeRef type, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:894-901
    void add_phi_incoming(LLVMValueRef phi, const std::vector<std::pair<LLVMBasicBlockRef, LLVMValueRef>>& incoming);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:903-907
    void assign_phis(const std::vector<std::pair<LLVMValueRef, LLVMValueRef>>& phi_to_value);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:949-954
    LLVMValueRef br(LLVMBasicBlockRef block);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:956-960
    LLVMValueRef cond_br(LLVMValueRef condition, LLVMBasicBlockRef bb_true, LLVMBasicBlockRef bb_false);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:962-964
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmCallable.kt:98-99
    LLVMValueRef block_address(LLVMBasicBlockRef block);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1001
    LLVMValueRef icmp_eq(LLVMValueRef left, LLVMValueRef right, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1003
    LLVMValueRef icmp_gt(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1004
    LLVMValueRef icmp_ge(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1005
    LLVMValueRef icmp_lt(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1006
    LLVMValueRef icmp_le(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1007
    LLVMValueRef icmp_ne(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1008
    LLVMValueRef icmp_u_lt(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1009
    LLVMValueRef icmp_u_le(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1010
    LLVMValueRef icmp_u_gt(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1011
    LLVMValueRef icmp_u_ge(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1208-1235
    LLVMValueRef if_then_else(LLVMValueRef condition, LLVMValueRef then_value,
        const std::function<LLVMValueRef()>& else_block);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1237-1250
    void if_then(LLVMValueRef condition, const std::function<void()>& then_block);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1264-1269
    LLVMValueRef indirect_br(LLVMValueRef address, const std::vector<LLVMBasicBlockRef>& destinations);
    // NOTE(port): Escape Kotlin's switch method name with a trailing underscore,
    // since switch is a C++ keyword.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1271-1276
    LLVMValueRef switch_(LLVMValueRef value,
        const std::vector<std::pair<LLVMValueRef, LLVMBasicBlockRef>>& cases, LLVMBasicBlockRef else_block);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1596-1599
    template <typename R>
    R appending_to(LLVMBasicBlockRef block, const std::function<R(FunctionGenerationContext&)>& code);
protected:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1461-1463
    LLVMValueRef raw_ret(LLVMValueRef value);
private:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1584-1594
    template <typename R>
    R preserving_position(const std::function<R()>& code);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:722-726
    LLVMValueRef apply_memory_order_and_alignment(LLVMValueRef value,
        std::optional<LLVMAtomicOrdering> memory_order, std::optional<int> alignment);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:618-621
    void update(LLVMBasicBlockRef block, LocationInfo* start_location, LocationInfo* end_location);
    LLVMValueRef function_;
    LLVMContextRef context_;
    const bool contain_location_debug_info_;
    class DebugLocationState;
    std::unique_ptr<DebugLocationState> debug_locations_;
    class PositionHolder;
    std::unique_ptr<PositionHolder> current_position_holder_;
};
}
