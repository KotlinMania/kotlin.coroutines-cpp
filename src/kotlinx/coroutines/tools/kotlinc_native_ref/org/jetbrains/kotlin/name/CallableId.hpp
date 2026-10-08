// port-lint: source core/names/src/org/jetbrains/kotlin/name/CallableId.kt
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:45-141
#pragma once
#include "ClassId.hpp"
namespace org::jetbrains::kotlin::name {
/** Identifies a callable; overloaded declarations may share the same ID. */
// NOTE(port): ClassIdBasedLocality opt-in metadata remains a compiler dependency.
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:9-52
class CallableId final {
public:
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:69-71
    CallableId(FqName package_name, std::optional<FqName> class_name, Name callable_name);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:73-77
    CallableId(FqName package_name, std::optional<FqName> class_name, Name callable_name, std::optional<FqName> path_to_local);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:79-80
    CallableId(ClassId class_id, Name callable_name);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:82-83
    CallableId(FqName package_name, Name callable_name);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:85-89
    CallableId(Name callable_name, std::optional<FqName> path_to_local);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:91-91
    explicit CallableId(Name callable_name);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:46-46
    const FqName& package_name() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:47-47
    const std::optional<FqName>& class_name() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:48-48
    const Name& callable_name() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:49-49
    const std::optional<ClassId>& class_id() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:55-55
    static const FqName& package_fq_name_for_local();
    /** True for a local callable or a member of a local class. */
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:61-67
    bool is_local() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:93-96
    FqName as_fq_name_for_debug_info() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:98-100
    FqName as_single_fq_name() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:102-102
    CallableId copy(Name callable_name) const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:104-111
    bool equals(const std::any& other) const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:113-119
    std::int32_t hash_code() const;
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:121-131
    std::u16string to_string() const;
private:
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:45-52
    CallableId(FqName package_name, std::optional<FqName> class_name, Name callable_name,
        std::optional<ClassId> class_id, std::optional<FqName> path_to_local);
    const FqName package_name_;
    const std::optional<FqName> class_name_;
    const Name callable_name_;
    const std::optional<ClassId> class_id_;
    const std::optional<FqName> path_to_local_;
};
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:134-136
CallableId with_class_id(const CallableId& callable_id, ClassId class_id);
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:138-138
FqName package_name(const CallableId* callable_id);
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:140-141
bool is_local(const CallableId* callable_id);
}
