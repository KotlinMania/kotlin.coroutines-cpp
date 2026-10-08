// port-lint: source core/names/src/org/jetbrains/kotlin/name/FqName.kt
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:29-125
#include "FqName.hpp"
#include <stdexcept>
#include <utility>

namespace org::jetbrains::kotlin::name {
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:119-120
// NOTE(port): Companion constant getter avoids C++ cross-unit initialization order.
const FqName& FqName::root() {
    static const FqName ROOT(u"");
    return ROOT;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:36-38
FqName::FqName(std::u16string fq_name) : fq_name_(std::move(fq_name), true) {}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:40-42
FqName::FqName(FqNameUnsafe fq_name) : fq_name_(std::move(fq_name)) {}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:44-47
FqName::FqName(FqNameUnsafe fq_name, const FqName& parent)
    : fq_name_(std::move(fq_name)), parent_(std::make_shared<FqName>(parent)) {}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:49-51
const std::u16string& FqName::as_string() const { return fq_name_.as_string(); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:53-55
FqNameUnsafe FqName::to_unsafe() const { return fq_name_; }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:57-58
bool FqName::is_root() const { return fq_name_.is_root(); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:60-70
FqName FqName::parent() const {
    if (parent_) return *parent_;
    if (is_root()) throw std::logic_error("root");
    parent_ = std::make_shared<FqName>(fq_name_.parent());
    return *parent_;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:72-74
FqName FqName::child(const Name& name) const { return FqName(fq_name_.child(name), *this); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:76-78
Name FqName::short_name() const { return fq_name_.short_name(); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:80-82
Name FqName::short_name_or_special() const { return fq_name_.short_name_or_special(); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:88-90
bool FqName::starts_with(const Name& segment) const { return fq_name_.starts_with(segment); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:92-94
bool FqName::starts_with(const FqName& other) const { return fq_name_.starts_with(other.fq_name_); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:96-98
std::u16string FqName::to_string() const { return fq_name_.to_string(); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:100-107
bool FqName::equals(const std::any& other) const {
    const auto* value = std::any_cast<FqName>(&other);
    return value && fq_name_.equals(value->fq_name_);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:109-111
std::int32_t FqName::hash_code() const { return fq_name_.hash_code(); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqName.kt:123-125
FqName FqName::top_level(const Name& short_name) { return FqName(FqNameUnsafe::top_level(short_name)); }
}
