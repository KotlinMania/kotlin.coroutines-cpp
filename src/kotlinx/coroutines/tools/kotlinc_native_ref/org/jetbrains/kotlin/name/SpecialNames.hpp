// port-lint: source core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:18-119
#pragma once
#include "FqName.hpp"
#include <string_view>
namespace org::jetbrains::kotlin::name {
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:18-119
class SpecialNames final {
public:
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:35-35
    static constexpr std::u16string_view ANONYMOUS_STRING = u"<anonymous>";
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:20-20
    static const Name& no_name_provided();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:23-23
    static const Name& root_package();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:26-26
    static const Name& default_name_for_companion_object();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:33-33
    static const Name& safe_identifier_for_no_name();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:38-38
    static const Name& anonymous();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:41-41
    static const FqName& anonymous_fq_name();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:44-44
    static const Name& unary();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:47-47
    // NOTE(port): The property name THIS maps to this_name, escaping C++ this.
    static const Name& this_name();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:50-50
    static const Name& init();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:53-53
    static const Name& when_subject();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:56-56
    static const Name& iterator();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:59-59
    static const Name& destruct();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:62-62
    static const Name& local();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:65-65
    static const Name& underscore_for_unused_var();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:68-68
    static const Name& implicit_set_parameter();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:71-71
    static const Name& array();
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:74-74
    static const Name& receiver();
    /** Name of the generated read-only enum entries property. */
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:80-80
    static const Name& enum_get_entries();
    /** Special name for an index expression in a subscription operator. */
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:88-92
    static Name subscribe_operator_index(std::int32_t index);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:97-99
    static Name anonymous_parameter_name(std::int32_t index);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:102-104
    static bool is_anonymous_parameter_name(const Name& name);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:107-109
    static Name safe_identifier(const Name* name);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:112-114
    static Name safe_identifier(const std::u16string* name);
    // Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:116-118
    static bool is_safe_identifier(const Name& name);
};
}
