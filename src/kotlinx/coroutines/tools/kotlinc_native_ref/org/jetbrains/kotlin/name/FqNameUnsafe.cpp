// port-lint: source core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:12-185
#include "FqNameUnsafe.hpp"
#include "FqName.hpp"
#include <bit>
#include <stdexcept>
#include <utility>

namespace org::jetbrains::kotlin::name {
// NOTE(port): Like Name, immutable names use C++ value handles. Shared backing
// retains the name and lazy parent/short-name caches. The safe view shares this
// backing instead of forming a strong FqName/Unsafe ownership cycle. The internal
// source constructor's safe argument is always the same-name FqName being built.
// UTF-16 code units and source cache-dependent isSafe behavior are preserved.
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:13-38
struct FqNameUnsafe::State {
    explicit State(std::u16string fq_name, bool safe_view = false)
        : fq_name_(std::move(fq_name)), safe_view_(safe_view) {}
    const std::u16string fq_name_;
    bool safe_view_;
    std::shared_ptr<State> parent_;
    std::optional<Name> short_name_;
};
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:30-32
FqNameUnsafe::FqNameUnsafe(std::u16string fq_name)
    : state_(std::make_shared<State>(std::move(fq_name))) {}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:25-28
FqNameUnsafe::FqNameUnsafe(std::u16string fq_name, bool safe_view)
    : state_(std::make_shared<State>(std::move(fq_name), safe_view)) {}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:34-38
FqNameUnsafe::FqNameUnsafe(std::u16string fq_name, const FqNameUnsafe& parent, const Name& short_name)
    : FqNameUnsafe(std::move(fq_name)) {
    state_->parent_ = parent.state_;
    state_->short_name_ = short_name;
}
// NOTE(port): Construct another value handle over the actual cached backing.
FqNameUnsafe::FqNameUnsafe(std::shared_ptr<State> state) : state_(std::move(state)) {}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:40-49
void FqNameUnsafe::compute() const {
    const auto last_dot = index_of_last_dot_with_backticks_support(as_string());
    if (last_dot != std::u16string::npos) {
        state_->short_name_ = Name::guess_by_first_character(as_string().substr(last_dot + 1));
        state_->parent_ = FqNameUnsafe(as_string().substr(0, last_dot)).state_;
    } else {
        state_->short_name_ = Name::guess_by_first_character(as_string());
        state_->parent_ = FqName::root().to_unsafe().state_;
    }
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:51-65
std::u16string::size_type FqNameUnsafe::index_of_last_dot_with_backticks_support(const std::u16string& fq_name) {
    bool is_backtick = false;
    for (auto index = fq_name.size(); index != 0;) {
        --index;
        if (fq_name[index] == u'.' && !is_backtick) return index;
        if (fq_name[index] == u'`') is_backtick = !is_backtick;
    }
    return std::u16string::npos;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:67-69
const std::u16string& FqNameUnsafe::as_string() const { return state_->fq_name_; }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:71-71
bool FqNameUnsafe::is_safe() const {
    return state_->safe_view_ || as_string().find(u'<') == std::u16string::npos;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:73-74
FqName FqNameUnsafe::to_safe() const {
    FqName result(*this);
    state_->safe_view_ = true;
    return result;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:76-77
bool FqNameUnsafe::is_root() const { return as_string().empty(); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:79-89
FqNameUnsafe FqNameUnsafe::parent() const {
    if (state_->parent_) return FqNameUnsafe(state_->parent_);
    if (is_root()) throw std::logic_error("root");
    compute();
    return FqNameUnsafe(state_->parent_);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:91-98
FqNameUnsafe FqNameUnsafe::child(const Name& name) const {
    auto child_fq_name = is_root() ? name.as_string() : as_string() + u"." + name.as_string();
    return FqNameUnsafe(std::move(child_fq_name), *this, name);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:100-110
Name FqNameUnsafe::short_name() const {
    if (state_->short_name_) return *state_->short_name_;
    if (is_root()) throw std::logic_error("root");
    compute();
    return *state_->short_name_;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:112-118
Name FqNameUnsafe::short_name_or_special() const {
    return is_root() ? Name::special(u"<root>") : short_name();
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:134-142
bool FqNameUnsafe::starts_with(const Name& segment) const {
    if (is_root()) return false;
    const auto first_dot = as_string().find(u'.');
    const auto length = first_dot == std::u16string::npos ? as_string().size() : first_dot;
    return length == segment.as_string().size() && as_string().compare(0, length, segment.as_string()) == 0;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:144-153
bool FqNameUnsafe::starts_with(const FqNameUnsafe& other) const {
    if (is_root() || as_string().size() < other.as_string().size()) return false;
    const auto length = other.as_string().size();
    return (as_string().size() == length || as_string()[length] == u'.') &&
        as_string().compare(0, length, other.as_string()) == 0;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:155-157
std::u16string FqNameUnsafe::to_string() const {
    return is_root() ? Name::special(u"<root>").as_string() : as_string();
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:159-166
bool FqNameUnsafe::equals(const std::any& other) const {
    const auto* value = std::any_cast<FqNameUnsafe>(&other);
    return value && (state_ == value->state_ || as_string() == value->as_string());
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:168-170
std::int32_t FqNameUnsafe::hash_code() const {
    std::uint32_t result = 0;
    for (char16_t unit : as_string()) result = result * 31U + unit;
    return std::bit_cast<std::int32_t>(result);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:177-180
bool FqNameUnsafe::is_valid(const std::optional<std::u16string>& qualified_name) {
    return qualified_name && qualified_name->find(u'/') == std::u16string::npos &&
        qualified_name->find(u'*') == std::u16string::npos;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/FqNameUnsafe.kt:183-185
FqNameUnsafe FqNameUnsafe::top_level(const Name& short_name) {
    return FqNameUnsafe(short_name.as_string(), FqName::root().to_unsafe(), short_name);
}
}
