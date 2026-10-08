// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:11-60
#pragma once
#include "../name/FqName.hpp"
#include <atomic>
#include <memory>
#include <string>
#include <string_view>

namespace org::jetbrains::kotlin::builtins {
// NOTE(port): Enum instances with source properties are concrete immutable
// singletons; nullable enum results are borrowed pointers to these instances.
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:11-60
class PrimitiveType final {
public:
    // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:12-19
    static const PrimitiveType BOOLEAN;
    static const PrimitiveType CHAR;
    static const PrimitiveType BYTE;
    static const PrimitiveType SHORT;
    static const PrimitiveType INT;
    static const PrimitiveType FLOAT;
    static const PrimitiveType LONG;
    static const PrimitiveType DOUBLE;
    PrimitiveType(const PrimitiveType&) = delete;
    PrimitiveType& operator=(const PrimitiveType&) = delete;
    // NOTE(port): Each instance owns its published properties and name boxes.
    // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:26-28
    ~PrimitiveType();
    // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:22-22
    const name::Name& type_name() const;
    // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:24-24
    const name::Name& array_type_name() const;
    // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:26-26
    const name::FqName& type_fq_name() const;
    // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:28-28
    const name::FqName& array_type_fq_name() const;
    // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:34-45
    static const PrimitiveType* get_by_short_name(const std::u16string& name);
    // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:47-58
    static const PrimitiveType* get_by_short_array_name(const std::u16string& name);
private:
    struct Properties;
    // NOTE(port): Constant initialization makes canonical enum instances safe
    // for compiler catalogs used before main. Owned properties are constructed
    // on their first access; their implementation remains in the .cpp file.
    // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:11-24
    constexpr explicit PrimitiveType(std::u16string_view type_name) : type_name_string_(type_name) {}
    // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:22-28
    const Properties& properties() const;
    const std::u16string_view type_name_string_;
    mutable std::atomic<const Properties*> properties_{nullptr};
};
}
