// port-lint: source core/names/src/org/jetbrains/kotlin/name/FqName.kt
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt
#pragma once
#include "FqNameUnsafe.hpp"
#include <any>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace org::jetbrains::kotlin::name {
/** Fully qualified declaration or package name, such as foo.bar.A. */
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:29-127
class FqName final {
public:
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:36-38
    explicit FqName(std::u16string fq_name);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:40-42
    explicit FqName(FqNameUnsafe fq_name);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:49-51
    const std::u16string& as_string() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:53-55
    FqNameUnsafe to_unsafe() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:57-58
    bool is_root() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:60-70
    FqName parent() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:72-74
    FqName child(const Name& name) const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:76-78
    Name short_name() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:80-82
    Name short_name_or_special() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:88-90
    bool starts_with(const Name& segment) const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:92-94
    bool starts_with(const FqName& other) const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:96-98
    std::u16string to_string() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:100-107
    bool equals(const std::any& other) const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:109-111
    std::int32_t hash_code() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:119-120
    static const FqName& root();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:123-125
    static FqName top_level(const Name& short_name);
private:
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:30-34
    FqNameUnsafe fq_name_;
    mutable std::shared_ptr<FqName> parent_;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:44-47
    FqName(FqNameUnsafe fq_name, const FqName& parent);
};
}
