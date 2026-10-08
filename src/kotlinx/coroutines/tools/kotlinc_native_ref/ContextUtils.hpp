// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:21-130,300-365,411-441,573-579
#pragma once
#include <llvm-c/Core.h>
#include "LlvmCallable.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:21-45
class SlotType {
public:
    virtual ~SlotType();
    SlotType(const SlotType&) = delete;
    SlotType& operator=(const SlotType&) = delete;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:23-23
    class Stack;
    static const Stack STACK;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:26-26
    class Arena;
    static const Arena ARENA;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:29-29
    class Return;
    static const Return RETURN;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:32-32
    class ReturnIfArena;
    static const ReturnIfArena RETURN_IF_ARENA;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:41-41
    class Anonymous;
    static const Anonymous ANONYMOUS;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:44-44
    class Unknown;
    static const Unknown UNKNOWN;
    class ParamIfArena;
    class ParamsIfArena;
private:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:21-21
    SlotType();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:23-23
class SlotType::Stack final : public SlotType {
private:
    friend class SlotType;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:23-23
    Stack();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:26-26
class SlotType::Arena final : public SlotType {
private:
    friend class SlotType;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:26-26
    Arena();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:29-29
class SlotType::Return final : public SlotType {
private:
    friend class SlotType;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:29-29
    Return();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:32-32
class SlotType::ReturnIfArena final : public SlotType {
private:
    friend class SlotType;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:32-32
    ReturnIfArena();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:41-41
class SlotType::Anonymous final : public SlotType {
private:
    friend class SlotType;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:41-41
    Anonymous();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:44-44
class SlotType::Unknown final : public SlotType {
private:
    friend class SlotType;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:44-44
    Unknown();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:35-35
class SlotType::ParamIfArena final : public SlotType {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:35-35
    explicit ParamIfArena(int parameter);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:35-35
    int parameter() const;
private:
    const int parameter_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:38-38
class SlotType::ParamsIfArena final : public SlotType {
public:
    // NOTE(port): Shared mutable array storage preserves Kotlin IntArray identity.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:38-38
    ParamsIfArena(std::shared_ptr<std::vector<int>> parameters, bool use_return_slot);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:38-38
    const std::shared_ptr<std::vector<int>>& parameters() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:38-38
    bool use_return_slot() const;
private:
    const std::shared_ptr<std::vector<int>> parameters_;
    const bool use_return_slot_;
};

// Lifetimes of references, computed by escape analysis.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:48-130
class Lifetime {
public:
    virtual ~Lifetime();
    Lifetime(const Lifetime&) = delete;
    Lifetime& operator=(const Lifetime&) = delete;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:48-48
    const SlotType& slot_type() const;
    // NOTE(port): Every concrete source variant overrides Kotlin's toString.
    virtual std::string to_string() const = 0;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:49-53
    class Stack;
    static const Stack STACK;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:60-64
    class Local;
    static const Local LOCAL;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:67-71
    class ReturnValue;
    static const ReturnValue RETURN_VALUE;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:74-78
    class IndirectReturnValue;
    static const IndirectReturnValue INDIRECT_RETURN_VALUE;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:96-100
    class Global;
    static const Global GLOBAL;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:103-107
    class Throw;
    static const Throw THROW;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:111-115
    class Argument;
    static const Argument ARGUMENT;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:118-122
    class Unknown;
    static const Unknown UNKNOWN;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:125-129
    class Irrelevant;
    static const Irrelevant IRRELEVANT;
    class StackArray;
    class ParameterField;
    class ParametersField;
private:
    // NOTE(port): Static slot objects stay borrowed; freshly constructed
    // parameter slot objects are owned by the lifetime that creates them.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:48-48
    explicit Lifetime(const SlotType& slot_type);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:48-48,81-81,88-89
    explicit Lifetime(std::unique_ptr<const SlotType> slot_type);
    const std::unique_ptr<const SlotType> owned_slot_type_;
    const SlotType* const slot_type_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:49-53
class Lifetime::Stack final : public Lifetime {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:49-53
    std::string to_string() const override;
private:
    friend class Lifetime;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:49-53
    Stack();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:60-64
class Lifetime::Local final : public Lifetime {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:60-64
    std::string to_string() const override;
private:
    friend class Lifetime;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:60-64
    Local();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:67-71
class Lifetime::ReturnValue final : public Lifetime {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:67-71
    std::string to_string() const override;
private:
    friend class Lifetime;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:67-71
    ReturnValue();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:74-78
class Lifetime::IndirectReturnValue final : public Lifetime {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:74-78
    std::string to_string() const override;
private:
    friend class Lifetime;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:74-78
    IndirectReturnValue();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:96-100
class Lifetime::Global final : public Lifetime {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:96-100
    std::string to_string() const override;
private:
    friend class Lifetime;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:96-100
    Global();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:103-107
class Lifetime::Throw final : public Lifetime {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:103-107
    std::string to_string() const override;
private:
    friend class Lifetime;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:103-107
    Throw();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:111-115
class Lifetime::Argument final : public Lifetime {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:111-115
    std::string to_string() const override;
private:
    friend class Lifetime;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:111-115
    Argument();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:118-122
class Lifetime::Unknown final : public Lifetime {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:118-122
    std::string to_string() const override;
private:
    friend class Lifetime;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:118-122
    Unknown();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:125-129
class Lifetime::Irrelevant final : public Lifetime {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:125-129
    std::string to_string() const override;
private:
    friend class Lifetime;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:125-129
    Irrelevant();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:55-57
class Lifetime::StackArray final : public Lifetime {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:55-57
    explicit StackArray(int size);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:55-57
    int size() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:55-57
    std::string to_string() const override;
private:
    const int size_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:81-85
class Lifetime::ParameterField final : public Lifetime {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:81-85
    explicit ParameterField(int parameter);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:81-85
    int parameter() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:81-85
    std::string to_string() const override;
private:
    const int parameter_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:88-93
class Lifetime::ParametersField final : public Lifetime {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:88-93
    ParametersField(std::shared_ptr<std::vector<int>> parameters, bool use_return_slot);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:88-93
    const std::shared_ptr<std::vector<int>>& parameters() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:88-93
    bool use_return_slot() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:88-93
    std::string to_string() const override;
private:
    const std::shared_ptr<std::vector<int>> parameters_;
    const bool use_return_slot_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:300-326
class BasicLlvmHelpers {
public:
    // NOTE(port): Supply the actual BitcodePostProcessingContext LLVM handle
    // and opaque-pointer policy at this boundary; LLVM objects remain borrowed.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:300-302
    BasicLlvmHelpers(LLVMContextRef llvm_context, LLVMModuleRef module, bool use_llvm_opaque_pointers);
    virtual ~BasicLlvmHelpers();
    BasicLlvmHelpers(const BasicLlvmHelpers&) = delete;
    BasicLlvmHelpers& operator=(const BasicLlvmHelpers&) = delete;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:300-300
    LLVMModuleRef module() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:302-302
    LLVMContextRef llvm_context() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:303-305
    const std::string& target_triple() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:307-325
    const std::map<std::string, std::vector<LLVMValueRef>>& runtime_annotation_map() const;
private:
    const LLVMContextRef llvm_context_;
    const LLVMModuleRef module_;
    const bool use_llvm_opaque_pointers_;
    class LazyProperties;
    std::unique_ptr<LazyProperties> lazy_;
};

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:329-363,411-441,573-579
class CodegenLlvmHelpers : public BasicLlvmHelpers {
public:
    // NOTE(port): Bind the actual compiler and Runtime LLVM modules directly;
    // their owners outlive this helper and its attribute providers.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:329-330,411-441
    CodegenLlvmHelpers(LLVMContextRef llvm_context, LLVMModuleRef module,
        bool use_llvm_opaque_pointers, LLVMModuleRef runtime_module);
    ~CodegenLlvmHelpers() override;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:418-418
    const LlvmFunction& alloc_instance_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:419-419
    const LlvmFunction& alloc_array_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:420-420
    const LlvmFunction& register_global_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:421-421
    const LlvmFunction& update_heap_ref_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:422-422
    const LlvmFunction& update_stack_ref_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:423-423
    const LlvmFunction& update_return_ref_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:424-424
    const LlvmFunction& zero_heap_ref_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:425-425
    const LlvmFunction& zero_array_refs_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:426-426
    const LlvmFunction& enter_frame_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:427-427
    const LlvmFunction& leave_frame_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:428-428
    const LlvmFunction& set_current_frame_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:429-429
    const LlvmFunction& check_current_frame_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:430-430
    const LlvmFunction& lookup_interface_table_record() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:431-431
    const LlvmFunction& is_subtype_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:432-432
    const LlvmFunction& is_subclass_fast_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:433-433
    const LlvmFunction& get_type_info() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:434-434
    const LlvmFunction& throw_exception_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:435-435
    const LlvmFunction& append_to_initalizers_tail() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:436-436
    const LlvmFunction& call_init_global_possibly_lock() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:437-437
    const LlvmFunction& call_init_thread_local() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:438-438
    const LlvmFunction& add_tls_record() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:439-439
    const LlvmFunction& lookup_tls() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:440-440
    const LlvmFunction& init_runtime_if_needed() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:441-441
    const LlvmFunction& kotlin_get_exception_object() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:573-573
    const LlvmCallable& memset_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:575-579
    const LlvmFunction::Declaration& llvm_trap() const;
private:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:332-347
    std::unique_ptr<LlvmFunction::Declaration> import_function(const std::string& name,
        LLVMModuleRef other_module, bool returns_object_type);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:349-356
    std::unique_ptr<LlvmFunction::Declaration> import_memset();
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:358-365
    std::unique_ptr<LlvmFunction::Declaration> llvm_intrinsic(const std::string& name,
        LLVMTypeRef type, const std::vector<std::string>& attributes = {});
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:416-416
    std::unique_ptr<LlvmFunction::Declaration> import_rt_function(const std::string& name, bool returns_object_type);
    const LLVMModuleRef runtime_module_;
    const bool use_llvm_opaque_pointers_;
    const std::unique_ptr<LlvmFunction::Declaration> alloc_instance_function_;
    const std::unique_ptr<LlvmFunction::Declaration> alloc_array_function_;
    const std::unique_ptr<LlvmFunction::Declaration> register_global_function_;
    const std::unique_ptr<LlvmFunction::Declaration> update_heap_ref_function_;
    const std::unique_ptr<LlvmFunction::Declaration> update_stack_ref_function_;
    const std::unique_ptr<LlvmFunction::Declaration> update_return_ref_function_;
    const std::unique_ptr<LlvmFunction::Declaration> zero_heap_ref_function_;
    const std::unique_ptr<LlvmFunction::Declaration> zero_array_refs_function_;
    const std::unique_ptr<LlvmFunction::Declaration> enter_frame_function_;
    const std::unique_ptr<LlvmFunction::Declaration> leave_frame_function_;
    const std::unique_ptr<LlvmFunction::Declaration> set_current_frame_function_;
    const std::unique_ptr<LlvmFunction::Declaration> check_current_frame_function_;
    const std::unique_ptr<LlvmFunction::Declaration> lookup_interface_table_record_;
    const std::unique_ptr<LlvmFunction::Declaration> is_subtype_function_;
    const std::unique_ptr<LlvmFunction::Declaration> is_subclass_fast_function_;
    const std::unique_ptr<LlvmFunction::Declaration> get_type_info_;
    const std::unique_ptr<LlvmFunction::Declaration> throw_exception_function_;
    const std::unique_ptr<LlvmFunction::Declaration> append_to_initalizers_tail_;
    const std::unique_ptr<LlvmFunction::Declaration> call_init_global_possibly_lock_;
    const std::unique_ptr<LlvmFunction::Declaration> call_init_thread_local_;
    const std::unique_ptr<LlvmFunction::Declaration> add_tls_record_;
    const std::unique_ptr<LlvmFunction::Declaration> lookup_tls_;
    const std::unique_ptr<LlvmFunction::Declaration> init_runtime_if_needed_;
    const std::unique_ptr<LlvmFunction::Declaration> kotlin_get_exception_object_;
    const std::unique_ptr<LlvmFunction::Declaration> memset_function_;
    const std::unique_ptr<LlvmFunction::Declaration> llvm_trap_;
};

}
