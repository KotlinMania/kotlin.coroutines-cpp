// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:67-105
#pragma once
#include "LlvmUtils.hpp"
#include <string>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:67-69
class LlvmAttribute {
public:
    virtual ~LlvmAttribute() = default;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:68-68
    virtual LLVMAttributeKindId as_attribute_kind_id() const = 0;
};
// Preserve the source's class representation for attributes with parameters.
// NOTE(port): Source singleton attributes have static identities and cannot be
// copied. Attribute lists borrow those singleton objects.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:71-85
class LlvmParameterAttribute : public LlvmAttribute {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:75-77
    LLVMAttributeKindId as_attribute_kind_id() const override;
    LlvmParameterAttribute(const LlvmParameterAttribute&) = delete;
    LlvmParameterAttribute& operator=(const LlvmParameterAttribute&) = delete;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:83-83
    class SignExt;
    static const SignExt SIGN_EXT;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:84-84
    class ZeroExt;
    static const ZeroExt ZERO_EXT;
private:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:73-73
    explicit LlvmParameterAttribute(std::string llvm_attribute_name);
    const std::string llvm_attribute_name_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:87-105
class LlvmFunctionAttribute : public LlvmAttribute {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:89-91
    LLVMAttributeKindId as_attribute_kind_id() const override;
    LlvmFunctionAttribute(const LlvmFunctionAttribute&) = delete;
    LlvmFunctionAttribute& operator=(const LlvmFunctionAttribute&) = delete;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:97-97
    class NoUnwind;
    static const NoUnwind NO_UNWIND;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:98-98
    class NoReturn;
    static const NoReturn NO_RETURN;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:99-99
    class NoInline;
    static const NoInline NO_INLINE;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:100-100
    class AlwaysInline;
    static const AlwaysInline ALWAYS_INLINE;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:101-101
    class SanitizeThread;
    static const SanitizeThread SANITIZE_THREAD;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:102-102
    class Ssp;
    static const Ssp SSP;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:103-103
    class SspStrong;
    static const SspStrong SSP_STRONG;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:104-104
    class SspReq;
    static const SspReq SSP_REQ;
private:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:87-87
    explicit LlvmFunctionAttribute(std::string llvm_attribute_name);
    const std::string llvm_attribute_name_;
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:83-83
class LlvmParameterAttribute::SignExt final : public LlvmParameterAttribute {
private:
    friend class LlvmParameterAttribute;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:83-83
    SignExt();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:84-84
class LlvmParameterAttribute::ZeroExt final : public LlvmParameterAttribute {
private:
    friend class LlvmParameterAttribute;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:84-84
    ZeroExt();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:97-97
class LlvmFunctionAttribute::NoUnwind final : public LlvmFunctionAttribute {
private:
    friend class LlvmFunctionAttribute;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:97-97
    NoUnwind();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:98-98
class LlvmFunctionAttribute::NoReturn final : public LlvmFunctionAttribute {
private:
    friend class LlvmFunctionAttribute;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:98-98
    NoReturn();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:99-99
class LlvmFunctionAttribute::NoInline final : public LlvmFunctionAttribute {
private:
    friend class LlvmFunctionAttribute;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:99-99
    NoInline();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:100-100
class LlvmFunctionAttribute::AlwaysInline final : public LlvmFunctionAttribute {
private:
    friend class LlvmFunctionAttribute;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:100-100
    AlwaysInline();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:101-101
class LlvmFunctionAttribute::SanitizeThread final : public LlvmFunctionAttribute {
private:
    friend class LlvmFunctionAttribute;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:101-101
    SanitizeThread();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:102-102
class LlvmFunctionAttribute::Ssp final : public LlvmFunctionAttribute {
private:
    friend class LlvmFunctionAttribute;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:102-102
    Ssp();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:103-103
class LlvmFunctionAttribute::SspStrong final : public LlvmFunctionAttribute {
private:
    friend class LlvmFunctionAttribute;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:103-103
    SspStrong();
};
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:104-104
class LlvmFunctionAttribute::SspReq final : public LlvmFunctionAttribute {
private:
    friend class LlvmFunctionAttribute;
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:104-104
    SspReq();
};
}
