// port-lint: source core/names/src/org/jetbrains/kotlin/name/ClassId.kt
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-147
#pragma once
#include "FqName.hpp"
#include <any>
#include <optional>

namespace org::jetbrains::kotlin::name {
/**
 * Uniquely identifies a Kotlin class. A local ID may also denote an inner class
 * of a local class; its relative name includes the callable/class containers.
 */
// NOTE(port): Kotlin's ClassIdBasedLocality compiler opt-in diagnostic remains
// a compiler-metadata dependency; the underlying locality value is preserved.
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:19-28
class ClassId final {
public:
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-33
    ClassId(FqName package_fq_name, FqName relative_class_name, bool is_local);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:29-29
    ClassId(FqName package_fq_name, const Name& top_level_name);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
    const FqName& package_fq_name() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
    const FqName& relative_class_name() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
    bool is_local() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:36-45
    [[deprecated("Use outer_class_id instead. It is semantically equivalent.")]]
    std::optional<ClassId> parent_class_id() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:47-48
    Name short_class_name() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:50-55
    std::optional<ClassId> outer_class_id() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:57-64
    ClassId outermost_class_id() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:66-67
    bool is_nested_class() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:69-71
    ClassId create_nested_class_id(const Name& name) const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:73-75
    FqName as_single_fq_name() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:77-79
    bool starts_with(const Name& segment) const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:84-102
    /** Packages use '/', classes use '.', e.g. kotlin/Map.Entry. */
    std::u16string as_string() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:104-114
    std::u16string as_fq_name_string() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:116-118
    std::u16string to_string() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:122-124
    static ClassId top_level(const FqName& top_level_fq_name);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:132-145
    /** Class names containing slashes may be quoted, e.g. package/`test/test`. */
    static ClassId from_string(const std::u16string& string, bool is_local = false);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
    bool equals(const std::any& other) const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
    std::int32_t hash_code() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
    FqName component1() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
    FqName component2() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
    bool component3() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
    ClassId copy(std::optional<FqName> package_fq_name = std::nullopt,
        std::optional<FqName> relative_class_name = std::nullopt,
        std::optional<bool> is_local = std::nullopt) const;
private:
    const FqName package_fq_name_;
    const FqName relative_class_name_;
    const bool is_local_;
};
}
