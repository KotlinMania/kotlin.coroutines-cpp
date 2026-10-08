// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:390-393,616-621,667-678,720-732,754-758,87-92,639-650,850-907,948-1042,1208-1262,1264-1276,1342-1347,1461-1463,1505-1558,1566-1599
#pragma once
#include "LocationInfo.hpp"
#include "LlvmCallable.hpp"
#include <llvm-c/Core.h>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <utility>
#include <optional>
namespace org::jetbrains::kotlin::backend::konan::llvm {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:87-92
class ExceptionHandler {
public:
    virtual ~ExceptionHandler();
    class None;
    class Caller;
    class Local;
    static const None NONE;
    static const Caller CALLER;
private:
    ExceptionHandler();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:88-88
class ExceptionHandler::None final : public ExceptionHandler {
private:
    friend class ExceptionHandler;
    None();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:89-89
class ExceptionHandler::Caller final : public ExceptionHandler {
private:
    friend class ExceptionHandler;
    Caller();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:90-92
class ExceptionHandler::Local : public ExceptionHandler {
public:
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:91-91
    virtual LLVMBasicBlockRef unwind() const = 0;
protected:
    Local();
};
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
    // NOTE(port): The enclosing compiler supplies its actual optional cleanup
    // landingpad here; Native frame/prologue construction remains separate.
    // NOTE(port): The caller supplies the owning compiler context's
    // shouldContainLocationDebugInfo result at this LLVM boundary.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:591-615
    FunctionGenerationContext(LLVMValueRef function, bool contain_location_debug_info,
        LLVMBasicBlockRef cleanup_landingpad = nullptr);
    // NOTE(port): The owning compiler retains this actual definition; this
    // context borrows it for code generation without changing LLVM ownership.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:591-600
    FunctionGenerationContext(const LlvmFunction::Definition& function, bool contain_location_debug_info,
        LLVMBasicBlockRef cleanup_landingpad = nullptr);
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
    // NOTE(port): Trailing underscores escape C++ operator keywords.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:966-966
    LLVMValueRef not_(LLVMValueRef arg, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:967-967
    LLVMValueRef and_(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:968-968
    LLVMValueRef or_(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:969-969
    LLVMValueRef xor_(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:971-972
    LLVMValueRef zext(LLVMValueRef arg, LLVMTypeRef type);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:974-975
    LLVMValueRef sext(LLVMValueRef arg, LLVMTypeRef type);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:977-982
    LLVMValueRef ext(LLVMValueRef arg, LLVMTypeRef type, bool is_signed);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:984-985
    LLVMValueRef trunc(LLVMValueRef arg, LLVMTypeRef type);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:994-994
    LLVMValueRef shl(LLVMValueRef arg, int amount);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:996-998
    LLVMValueRef shr(LLVMValueRef arg, int amount, bool is_signed);
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
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1014-1014
    LLVMValueRef fcmp_eq(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1015-1015
    LLVMValueRef fcmp_gt(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1016-1016
    LLVMValueRef fcmp_ge(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1017-1017
    LLVMValueRef fcmp_lt(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1018-1018
    LLVMValueRef fcmp_le(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1020-1020
    LLVMValueRef sub(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1021-1021
    LLVMValueRef add(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1023-1023
    LLVMValueRef fsub(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1024-1024
    LLVMValueRef fadd(LLVMValueRef arg0, LLVMValueRef arg1, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1025-1025
    LLVMValueRef fneg(LLVMValueRef arg, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1027-1028
    LLVMValueRef select(LLVMValueRef if_value, LLVMValueRef then_value, LLVMValueRef else_value, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1030-1030
    LLVMValueRef bitcast(LLVMTypeRef type, LLVMValueRef value, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1032-1032
    LLVMValueRef int_to_ptr(LLVMValueRef value, LLVMTypeRef dest_type, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1033-1033
    LLVMValueRef ptr_to_int(LLVMValueRef value, LLVMTypeRef dest_type, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1035-1036
    LLVMValueRef gep(LLVMTypeRef type, LLVMValueRef base, LLVMValueRef index, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1038-1039
    LLVMValueRef struct_gep(LLVMTypeRef type, LLVMValueRef base, int index, const std::string& name = "");
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1041-1042
    LLVMValueRef extract_value(LLVMValueRef aggregate, int index, const std::string& name = "");
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
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:667-671
    LLVMBasicBlockRef basic_block_in_function(const std::string& name, LocationInfo* location_info);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1461-1463
    LLVMValueRef raw_ret(LLVMValueRef value);
private:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:850-886
    LLVMValueRef call_raw(const LlvmCallable& llvm_callable, const std::vector<LLVMValueRef>& args,
        const ExceptionHandler& exception_handler);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:987-992
    LLVMValueRef shift(LLVMOpcode op, LLVMValueRef arg, int amount);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:650-650
    bool cleanup_landingpad_is_used_ = false;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:639-639
    const LLVMBasicBlockRef cleanup_landingpad_;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:1584-1594
    template <typename R>
    R preserving_position(const std::function<R()>& code);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:722-726
    LLVMValueRef apply_memory_order_and_alignment(LLVMValueRef value,
        std::optional<LLVMAtomicOrdering> memory_order, std::optional<int> alignment);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:618-621
    void update(LLVMBasicBlockRef block, LocationInfo* start_location, LocationInfo* end_location);
    const LlvmFunction::Definition* definition_ = nullptr;
    LLVMValueRef function_;
    LLVMContextRef context_;
    const bool contain_location_debug_info_;
    class DebugLocationState;
    std::unique_ptr<DebugLocationState> debug_locations_;
    class PositionHolder;
    std::unique_ptr<PositionHolder> current_position_holder_;
};
}
