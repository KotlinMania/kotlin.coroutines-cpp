// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/ContextUtils.kt:21-130
#pragma once
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
}
