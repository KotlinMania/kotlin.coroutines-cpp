// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:21-130,264-365,411-441,445-571,573-579
#pragma once
#include <llvm-c/Core.h>
#include "LlvmCallable.hpp"
#include "Runtime.hpp"
#include "LlvmUtils.hpp"
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
class CodegenLlvmHelpers;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:264-266
class ConstInt1 : public ConstValue {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:264-265
    ConstInt1(const CodegenLlvmHelpers& llvm, bool value);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:264-264
    bool value() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:265-265
    LLVMValueRef llvm() const override;
private:
    const bool value_;
    const LLVMValueRef llvm_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:268-270
class ConstInt8 : public ConstValue {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:268-269
    ConstInt8(const CodegenLlvmHelpers& llvm, std::int8_t value);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:268-268
    std::int8_t value() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:269-269
    LLVMValueRef llvm() const override;
private:
    const std::int8_t value_;
    const LLVMValueRef llvm_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:272-274
class ConstUInt8 : public ConstValue {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:272-273
    ConstUInt8(const CodegenLlvmHelpers& llvm, std::uint8_t value);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:272-272
    std::uint8_t value() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:273-273
    LLVMValueRef llvm() const override;
private:
    const std::uint8_t value_;
    const LLVMValueRef llvm_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:276-278
class ConstInt16 : public ConstValue {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:276-277
    ConstInt16(const CodegenLlvmHelpers& llvm, std::int16_t value);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:276-276
    std::int16_t value() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:277-277
    LLVMValueRef llvm() const override;
private:
    const std::int16_t value_;
    const LLVMValueRef llvm_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:280-282
class ConstChar16 : public ConstValue {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:280-281
    ConstChar16(const CodegenLlvmHelpers& llvm, char16_t value);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:280-280
    char16_t value() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:281-281
    LLVMValueRef llvm() const override;
private:
    const char16_t value_;
    const LLVMValueRef llvm_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:284-286
class ConstInt32 : public ConstValue {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:284-285
    ConstInt32(const CodegenLlvmHelpers& llvm, std::int32_t value);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:284-284
    std::int32_t value() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:285-285
    LLVMValueRef llvm() const override;
private:
    const std::int32_t value_;
    const LLVMValueRef llvm_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:288-290
class ConstInt64 : public ConstValue {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:288-289
    ConstInt64(const CodegenLlvmHelpers& llvm, std::int64_t value);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:288-288
    std::int64_t value() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:289-289
    LLVMValueRef llvm() const override;
private:
    const std::int64_t value_;
    const LLVMValueRef llvm_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:292-294
class ConstFloat32 : public ConstValue {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:292-293
    ConstFloat32(const CodegenLlvmHelpers& llvm, float value);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:292-292
    float value() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:293-293
    LLVMValueRef llvm() const override;
private:
    const float value_;
    const LLVMValueRef llvm_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:296-298
class ConstFloat64 : public ConstValue {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:296-297
    ConstFloat64(const CodegenLlvmHelpers& llvm, double value);
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:296-296
    double value() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:297-297
    LLVMValueRef llvm() const override;
private:
    const double value_;
    const LLVMValueRef llvm_;
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

// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:329-363,411-441,445-571,573-579
class CodegenLlvmHelpers : public BasicLlvmHelpers, public RuntimeAware {
public:
    // NOTE(port): Bind actual compiler LLVM handles and Runtime metadata directly;
    // their owners outlive this helper and its attribute providers.
    // NOTE(port): should_optimize is the enclosing generationState.shouldOptimize() policy.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:329-330,411-441,453-464
    CodegenLlvmHelpers(LLVMContextRef llvm_context, LLVMModuleRef module,
        bool use_llvm_opaque_pointers, const Runtime& runtime, bool should_optimize);
    ~CodegenLlvmHelpers() override;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:409-409
    const Runtime& runtime() const override;
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
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:445-445
    const LlvmFunction& kotlin_mm_create_retained_external_rc_ref() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:446-446
    const LlvmFunction& kotlin_mm_release_external_rc_ref() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:447-447
    const LlvmFunction& kotlin_mm_dispose_external_rc_ref() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:449-449
    const LlvmFunction& create_kotlin_obj_c_class() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:450-450
    const LlvmFunction& get_obj_c_kotlin_type_info() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:451-451
    const LlvmFunction& missing_init_imp() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:453-458
    const LlvmFunction& kotlin_mm_switch_thread_state_native() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:459-464
    const LlvmFunction& kotlin_mm_switch_thread_state_runnable() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:466-466
    const LlvmFunction& kotlin_interop_does_object_conform_to_protocol() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:467-467
    const LlvmFunction& kotlin_interop_does_object_conform_to_protocol_by_name() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:468-468
    const LlvmFunction& kotlin_interop_is_object_kind_of_class() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:470-470
    const LlvmFunction& kotlin_obj_c_export_ref_to_local_obj_c() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:471-471
    const LlvmFunction& kotlin_obj_c_export_ref_to_retained_obj_c() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:472-472
    const LlvmFunction& kotlin_obj_c_export_ref_from_obj_c() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:473-473
    const LlvmFunction& kotlin_obj_c_export_create_retained_ns_string_from_k_string() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:474-474
    const LlvmFunction& kotlin_obj_c_export_convert_unit_to_retained() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:475-475
    const LlvmFunction& kotlin_obj_c_export_get_associated_object() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:476-476
    const LlvmFunction& kotlin_obj_c_export_abstract_method_called() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:477-477
    const LlvmFunction& kotlin_obj_c_export_abstract_class_constructor_called() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:478-478
    const LlvmFunction& kotlin_obj_c_export_rethrow_exception_as_ns_error() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:479-479
    const LlvmFunction& kotlin_obj_c_export_wrap_exception_to_ns_error() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:480-480
    const LlvmFunction& kotlin_obj_c_export_ns_error_as_exception() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:481-481
    const LlvmFunction& kotlin_obj_c_export_alloc_instance_with_associated_object() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:482-482
    const LlvmFunction& kotlin_obj_c_export_create_continuation_argument() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:483-483
    const LlvmFunction& kotlin_obj_c_export_create_unit_continuation_argument() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:484-484
    const LlvmFunction& kotlin_obj_c_export_resume_continuation() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:489-489
    const LlvmFunction& kotlin_mm_safe_point_function_prologue() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:490-490
    const LlvmFunction& kotlin_mm_safe_point_while_loop_body() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:492-492
    const LlvmFunction& kotlin_process_object_in_mark() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:493-493
    const LlvmFunction& kotlin_process_array_in_mark() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:494-494
    const LlvmFunction& kotlin_process_empty_object_in_mark() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:496-496
    const LlvmFunction& update_volatile_heap_ref() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:497-497
    const LlvmFunction& compare_and_set_volatile_heap_ref() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:498-498
    const LlvmFunction& compare_and_swap_volatile_heap_ref() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:499-499
    const LlvmFunction& get_and_set_volatile_heap_ref() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:502-502
    const LlvmFunction& kotlin_array_get_element_address() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:503-503
    const LlvmFunction& kotlin_int_array_get_element_address() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:504-504
    const LlvmFunction& kotlin_long_array_get_element_address() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:514-514
    LLVMTypeRef int1_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:515-515
    LLVMTypeRef int8_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:516-516
    LLVMTypeRef int16_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:517-517
    LLVMTypeRef int32_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:518-518
    LLVMTypeRef int64_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:519-519
    LLVMTypeRef intptr_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:520-520
    LLVMTypeRef float_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:521-521
    LLVMTypeRef double_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:522-522
    LLVMTypeRef vector128_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:523-523
    LLVMTypeRef void_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:524-524
    LLVMTypeRef pointer_type() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:526-526
    LLVMTypeRef struct_type(const std::vector<LLVMTypeRef>& types = std::vector<LLVMTypeRef>{}) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:528-529
    Struct struct_(const std::vector<std::shared_ptr<const ConstValue>>& elements = std::vector<std::shared_ptr<const ConstValue>>{}, bool packed = false) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:534-541
    LLVMTypeRef struct_type_with_flexible_array(LLVMTypeRef original, int new_size) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:543-543
    ConstInt1 const_int1(bool value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:553-553
    LLVMValueRef int1(bool value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:544-544
    ConstInt8 const_int8(std::int8_t value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:554-554
    LLVMValueRef int8(std::int8_t value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:545-545
    ConstUInt8 const_uint8(std::uint8_t value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:546-546
    ConstInt16 const_int16(std::int16_t value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:555-555
    LLVMValueRef int16(std::int16_t value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:547-547
    ConstChar16 const_char16(char16_t value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:556-556
    LLVMValueRef char16(char16_t value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:548-548
    ConstInt32 const_int32(std::int32_t value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:557-557
    LLVMValueRef int32(std::int32_t value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:549-549
    ConstInt64 const_int64(std::int64_t value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:558-558
    LLVMValueRef int64(std::int64_t value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:550-550
    ConstFloat32 const_float32(float value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:560-560
    LLVMValueRef float32(float value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:551-551
    ConstFloat64 const_float64(double value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:561-561
    LLVMValueRef float64(double value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:559-559
    LLVMValueRef intptr(std::int32_t value) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:563-563
    LLVMValueRef null_constant() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:564-564
    LLVMValueRef imm_int32_zero() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:565-565
    LLVMValueRef imm_int32_one() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:566-566
    LLVMValueRef true_constant() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:567-567
    LLVMValueRef false_constant() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:569-571
    const std::shared_ptr<ConstPointer>& null_pointer() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:573-573
    const LlvmCallable& memset_function() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:575-579
    const LlvmFunction::Declaration& llvm_trap() const;
private:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:531-532
    LLVMTypeRef struct_type(const std::vector<LLVMTypeRef>& types, bool packed) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:486-486
    const LlvmFunction& kotlin_obj_c_export_ns_integer_type_provider() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:487-487
    const LlvmFunction& kotlin_long_type_provider() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:332-347
    std::unique_ptr<LlvmFunction::Declaration> import_function(const std::string& name,
        LLVMModuleRef other_module, bool returns_object_type) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:349-356
    std::unique_ptr<LlvmFunction::Declaration> import_memset() const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:358-365
    std::unique_ptr<LlvmFunction::Declaration> llvm_intrinsic(const std::string& name,
        LLVMTypeRef type, const std::vector<std::string>& attributes = std::vector<std::string>{}) const;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:416-416
    std::unique_ptr<LlvmFunction::Declaration> import_rt_function(const std::string& name, bool returns_object_type) const;
    struct LazyRuntimeFunctions;
    const std::unique_ptr<LazyRuntimeFunctions> lazy_runtime_functions_;
    const bool should_optimize_;
    const Runtime& runtime_;
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
    const LLVMTypeRef int1_type_;
    const LLVMTypeRef int8_type_;
    const LLVMTypeRef int16_type_;
    const LLVMTypeRef int32_type_;
    const LLVMTypeRef int64_type_;
    const LLVMTypeRef intptr_type_;
    const LLVMTypeRef float_type_;
    const LLVMTypeRef double_type_;
    const LLVMTypeRef vector128_type_;
    const LLVMTypeRef void_type_;
    const LLVMTypeRef pointer_type_;
    const LLVMValueRef null_constant_;
    const std::shared_ptr<ConstPointer> null_pointer_;
    const std::unique_ptr<LlvmFunction::Declaration> memset_function_;
    const std::unique_ptr<LlvmFunction::Declaration> llvm_trap_;
};

}
