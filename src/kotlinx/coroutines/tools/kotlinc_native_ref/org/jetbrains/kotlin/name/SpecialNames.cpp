// port-lint: source core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:18-119
#include "SpecialNames.hpp"
#include <stdexcept>
namespace org::jetbrains::kotlin::name {
namespace {
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:94-94
constexpr std::u16string_view ANONYMOUS_PARAMETER_NAME_PREFIX = u"anonymous parameter";
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:20-20
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::no_name_provided() {
    static const Name VALUE = Name::special(u"<no name provided>");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:23-23
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::root_package() {
    static const Name VALUE = Name::special(u"<root package>");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:26-26
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::default_name_for_companion_object() {
    static const Name VALUE = Name::identifier(u"Companion");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:33-33
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::safe_identifier_for_no_name() {
    static const Name VALUE = Name::identifier(u"no_name_in_PSI_3d19d79d_1ba9_4cd0_b7f5_b46aa3cd5d40");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:38-38
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::anonymous() {
    static const Name VALUE = Name::special(std::u16string(ANONYMOUS_STRING));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:41-41
// NOTE(port): Retained object property, initialized on first access.
const FqName& SpecialNames::anonymous_fq_name() {
    static const FqName VALUE = FqName::top_level(Name::special(std::u16string(ANONYMOUS_STRING)));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:44-44
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::unary() {
    static const Name VALUE = Name::special(u"<unary>");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:47-47
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::this_name() {
    static const Name VALUE = Name::special(u"<this>");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:50-50
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::init() {
    static const Name VALUE = Name::special(u"<init>");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:53-53
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::when_subject() {
    static const Name VALUE = Name::special(u"<when-subject>");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:56-56
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::iterator() {
    static const Name VALUE = Name::special(u"<iterator>");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:59-59
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::destruct() {
    static const Name VALUE = Name::special(u"<destruct>");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:62-62
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::local() {
    static const Name VALUE = Name::special(u"<local>");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:65-65
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::underscore_for_unused_var() {
    static const Name VALUE = Name::special(u"<unused var>");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:68-68
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::implicit_set_parameter() {
    static const Name VALUE = Name::special(u"<set-?>");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:71-71
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::array() {
    static const Name VALUE = Name::special(u"<array>");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:74-74
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::receiver() {
    static const Name VALUE = Name::special(u"<receiver>");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:80-80
// NOTE(port): Retained object property, initialized on first access.
const Name& SpecialNames::enum_get_entries() {
    static const Name VALUE = Name::special(u"<get-entries>");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:88-92
Name SpecialNames::subscribe_operator_index(std::int32_t index) {
    if (index < 0) throw std::invalid_argument("Index should be non-negative, but was " + std::to_string(index));
    const auto text = std::to_string(index);
    return Name::special(u"<index_" + std::u16string(text.begin(), text.end()) + u">");
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:97-99
Name SpecialNames::anonymous_parameter_name(std::int32_t index) {
    const auto text = std::to_string(index);
    return Name::special(u"<" + std::u16string(ANONYMOUS_PARAMETER_NAME_PREFIX) + u" " + std::u16string(text.begin(), text.end()) + u">");
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:102-104
bool SpecialNames::is_anonymous_parameter_name(const Name& name) {
    return name.is_special() && name.as_string_strip_special_markers().starts_with(ANONYMOUS_PARAMETER_NAME_PREFIX);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:107-109
Name SpecialNames::safe_identifier(const Name* name) {
    return name && !name->is_special() ? *name : safe_identifier_for_no_name();
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:112-114
Name SpecialNames::safe_identifier(const std::u16string* name) {
    if (!name) return safe_identifier(static_cast<const Name*>(nullptr));
    const auto identifier = Name::identifier(*name);
    return safe_identifier(&identifier);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/SpecialNames.kt:116-118
bool SpecialNames::is_safe_identifier(const Name& name) {
    return !name.as_string().empty() && !name.is_special();
}
}
