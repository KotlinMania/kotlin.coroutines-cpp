// port-lint: source core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt
#pragma once
#include "Name.hpp"
#include <any>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace org::jetbrains::kotlin::name {
class FqName;
/** Like FqName, but allows '<' and '>' characters in the name. */
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:12-187
class FqNameUnsafe final {
public:
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:30-32
    explicit FqNameUnsafe(std::u16string fq_name);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:67-69
    const std::u16string& as_string() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:71-71
    bool is_safe() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:73-74
    FqName to_safe() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:76-77
    bool is_root() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:79-89
    FqNameUnsafe parent() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:91-98
    FqNameUnsafe child(const Name& name) const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:100-110
    Name short_name() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:112-118
    Name short_name_or_special() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:134-142
    bool starts_with(const Name& segment) const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:144-153
    bool starts_with(const FqNameUnsafe& other) const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:155-157
    std::u16string to_string() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:159-166
    bool equals(const std::any& other) const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:168-170
    std::int32_t hash_code() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:177-180
    static bool is_valid(const std::optional<std::u16string>& qualified_name);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:183-185
    static FqNameUnsafe top_level(const Name& short_name);
private:
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:13-23
    struct State;
    std::shared_ptr<State> state_;
    friend class FqName;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:25-28
    FqNameUnsafe(std::u16string fq_name, bool safe_view);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:34-38
    FqNameUnsafe(std::u16string fq_name, const FqNameUnsafe& parent, const Name& short_name);
    explicit FqNameUnsafe(std::shared_ptr<State> state);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:40-49
    void compute() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:51-65
    static std::u16string::size_type index_of_last_dot_with_backticks_support(const std::u16string& fq_name);
};
}
