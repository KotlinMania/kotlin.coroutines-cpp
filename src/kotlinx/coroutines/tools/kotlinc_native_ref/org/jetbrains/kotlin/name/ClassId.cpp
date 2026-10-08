// port-lint: source core/names/src/org/jetbrains/kotlin/name/ClassId.kt
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-147
#include "ClassId.hpp"
#include <algorithm>
#include <bit>
#include <stdexcept>
#include <utility>

namespace org::jetbrains::kotlin::name {
namespace {
#ifndef NDEBUG
// NOTE(port): Encode source UTF-16 diagnostic text as UTF-8/WTF-8, following
// Name.cpp's exception boundary without changing the underlying code units.
std::string diagnostic_string(const std::u16string& text) {
    std::string result;
    for (std::size_t i = 0; i < text.size(); ++i) {
        std::uint32_t ch = text[i];
        if (ch >= 0xd800 && ch <= 0xdbff && i + 1 < text.size() &&
            text[i + 1] >= 0xdc00 && text[i + 1] <= 0xdfff)
            ch = 0x10000 + ((ch - 0xd800) << 10) + (text[++i] - 0xdc00);
        if (ch < 0x80) result.push_back(static_cast<char>(ch));
        else if (ch < 0x800) {
            result.push_back(static_cast<char>(0xc0 | (ch >> 6)));
            result.push_back(static_cast<char>(0x80 | (ch & 0x3f)));
        } else if (ch < 0x10000) {
            result.push_back(static_cast<char>(0xe0 | (ch >> 12)));
            result.push_back(static_cast<char>(0x80 | ((ch >> 6) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | (ch & 0x3f)));
        } else {
            result.push_back(static_cast<char>(0xf0 | (ch >> 18)));
            result.push_back(static_cast<char>(0x80 | ((ch >> 12) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | ((ch >> 6) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | (ch & 0x3f)));
        }
    }
    return result;
}
#endif
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:85-91
std::u16string escape_slashes(const FqName& name) {
    return name.as_string().find(u'/') == std::u16string::npos ? name.as_string() : u"`" + name.as_string() + u"`";
}
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-33
ClassId::ClassId(FqName package_fq_name, FqName relative_class_name, bool is_local)
    : package_fq_name_(std::move(package_fq_name)), relative_class_name_(std::move(relative_class_name)), is_local_(is_local) {
#ifndef NDEBUG
    // NOTE(port): Map Kotlin assertions to NDEBUG and compiler logic_error.
    if (relative_class_name_.is_root())
        throw std::logic_error("Class name must not be root: " + diagnostic_string(package_fq_name_.to_string()) + (is_local_ ? " (local)" : ""));
#endif
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:29-29
ClassId::ClassId(FqName package_fq_name, const Name& top_level_name)
    : ClassId(std::move(package_fq_name), FqName::top_level(top_level_name), false) {}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
const FqName& ClassId::package_fq_name() const { return package_fq_name_; }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
const FqName& ClassId::relative_class_name() const { return relative_class_name_; }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
bool ClassId::is_local() const { return is_local_; }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:41-45
std::optional<ClassId> ClassId::parent_class_id() const {
    if (!is_nested_class()) return std::nullopt;
    return ClassId(package_fq_name_, relative_class_name_.parent(), is_local_);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:47-48
Name ClassId::short_class_name() const { return relative_class_name_.short_name(); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:50-55
std::optional<ClassId> ClassId::outer_class_id() const {
    auto parent = relative_class_name_.parent();
    if (parent.is_root()) return std::nullopt;
    return ClassId(package_fq_name_, std::move(parent), is_local_);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:57-64
ClassId ClassId::outermost_class_id() const {
    auto name = relative_class_name_;
    while (!name.parent().is_root()) name = name.parent();
    return ClassId(package_fq_name_, std::move(name), false);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:66-67
bool ClassId::is_nested_class() const { return !relative_class_name_.parent().is_root(); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:69-71
ClassId ClassId::create_nested_class_id(const Name& name) const {
    return ClassId(package_fq_name_, relative_class_name_.child(name), is_local_);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:73-75
FqName ClassId::as_single_fq_name() const {
    return package_fq_name_.is_root() ? relative_class_name_ : FqName(package_fq_name_.as_string() + u"." + relative_class_name_.as_string());
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:77-79
bool ClassId::starts_with(const Name& segment) const { return package_fq_name_.starts_with(segment); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:84-102
std::u16string ClassId::as_string() const {
    if (package_fq_name_.is_root()) return escape_slashes(relative_class_name_);
    auto package = package_fq_name_.as_string();
    std::replace(package.begin(), package.end(), u'.', u'/');
    return package + u"/" + escape_slashes(relative_class_name_);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:104-114
std::u16string ClassId::as_fq_name_string() const {
    return package_fq_name_.is_root() ? relative_class_name_.as_string() : package_fq_name_.as_string() + u"." + relative_class_name_.as_string();
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:116-118
std::u16string ClassId::to_string() const { return package_fq_name_.is_root() ? u"/" + as_string() : as_string(); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:122-124
ClassId ClassId::top_level(const FqName& top_level_fq_name) {
    return ClassId(top_level_fq_name.parent(), top_level_fq_name.short_name());
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:132-145
ClassId ClassId::from_string(const std::u16string& string, bool is_local) {
    const auto tick_index = string.find(u'`');
    const auto last_slash_index = string.rfind(u'/', tick_index == std::u16string::npos ? string.size() : tick_index);
    std::u16string package_name;
    auto class_name = string;
    if (last_slash_index != std::u16string::npos) {
        package_name = string.substr(0, last_slash_index);
        std::replace(package_name.begin(), package_name.end(), u'/', u'.');
        class_name = string.substr(last_slash_index + 1);
    }
    std::erase(class_name, u'`');
    return ClassId(FqName(std::move(package_name)), FqName(std::move(class_name)), is_local);
}
// NOTE(port): Kotlin data-class generated members use the source field order.
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/DataClassMembersGenerator.kt:133-176
bool ClassId::equals(const std::any& other) const {
    const auto* value = std::any_cast<ClassId>(&other);
    return value && package_fq_name_.equals(value->package_fq_name_) &&
        relative_class_name_.equals(value->relative_class_name_) && is_local_ == value->is_local_;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/DataClassMembersGenerator.kt:178-221,269-307; kotlin-native/runtime/src/main/kotlin/kotlin/Boolean.kt:66-67
std::int32_t ClassId::hash_code() const {
    auto value = static_cast<std::uint32_t>(package_fq_name_.hash_code());
    value = value * 31U + static_cast<std::uint32_t>(relative_class_name_.hash_code());
    value = value * 31U + (is_local_ ? 1231U : 1237U);
    return std::bit_cast<std::int32_t>(value);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/DataClassMembersGenerator.kt:106-108
FqName ClassId::component1() const { return package_fq_name_; }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/DataClassMembersGenerator.kt:106-108
FqName ClassId::component2() const { return relative_class_name_; }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/DataClassMembersGenerator.kt:106-108
bool ClassId::component3() const { return is_local_; }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/ClassId.kt:28-28
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/DataClassMembersGenerator.kt:110-125
ClassId ClassId::copy(std::optional<FqName> package_fq_name, std::optional<FqName> relative_class_name, std::optional<bool> is_local) const {
    return ClassId(package_fq_name.value_or(package_fq_name_), relative_class_name.value_or(relative_class_name_), is_local.value_or(is_local_));
}
}
