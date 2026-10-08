// port-lint: source core/names/src/org/jetbrains/kotlin/name/CallableId.kt
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:45-141
#include "CallableId.hpp"
#include "SpecialNames.hpp"
#include <algorithm>
#include <bit>
#include <utility>
namespace org::jetbrains::kotlin::name {
namespace {
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:54-54
const Name& local_name() { return SpecialNames::local(); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:57-58
std::optional<ClassId> calculate_class_id(const FqName& package_name, const std::optional<FqName>& class_name) {
    if (!class_name) return std::nullopt;
    return ClassId(package_name, *class_name, package_name.equals(CallableId::package_fq_name_for_local()));
}
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:55-55
const FqName& CallableId::package_fq_name_for_local() {
    static const FqName VALUE = FqName::top_level(local_name());
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:45-52
CallableId::CallableId(FqName package_name, std::optional<FqName> class_name, Name callable_name,
    std::optional<ClassId> class_id, std::optional<FqName> path_to_local)
    : package_name_(std::move(package_name)), class_name_(std::move(class_name)),
      callable_name_(std::move(callable_name)), class_id_(std::move(class_id)), path_to_local_(std::move(path_to_local)) {}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:69-71
CallableId::CallableId(FqName package_name, std::optional<FqName> class_name, Name callable_name)
    : CallableId(package_name, class_name, std::move(callable_name), calculate_class_id(package_name, class_name), std::nullopt) {}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:73-77
CallableId::CallableId(FqName package_name, std::optional<FqName> class_name, Name callable_name, std::optional<FqName> path_to_local)
    : CallableId(package_name, class_name, std::move(callable_name), calculate_class_id(package_name, class_name), std::move(path_to_local)) {}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:79-80
CallableId::CallableId(ClassId class_id, Name callable_name)
    : CallableId(class_id.package_fq_name(), class_id.relative_class_name(), std::move(callable_name), class_id, std::nullopt) {}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:82-83
CallableId::CallableId(FqName package_name, Name callable_name)
    : CallableId(std::move(package_name), std::nullopt, std::move(callable_name), std::nullopt, std::nullopt) {}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:85-89
CallableId::CallableId(Name callable_name, std::optional<FqName> path_to_local)
    : CallableId(package_fq_name_for_local(), std::nullopt, std::move(callable_name), std::nullopt, std::move(path_to_local)) {}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:91-91
CallableId::CallableId(Name callable_name)
    : CallableId(package_fq_name_for_local(), std::nullopt, std::move(callable_name), std::nullopt, std::nullopt) {}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:46-46
const FqName& CallableId::package_name() const {
    return package_name_;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:47-47
const std::optional<FqName>& CallableId::class_name() const {
    return class_name_;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:48-48
const Name& CallableId::callable_name() const {
    return callable_name_;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:49-49
const std::optional<ClassId>& CallableId::class_id() const {
    return class_id_;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:61-67
bool CallableId::is_local() const {
    return package_name_.equals(package_fq_name_for_local()) || (class_id_ && class_id_->is_local());
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:93-96
FqName CallableId::as_fq_name_for_debug_info() const {
    if (path_to_local_) return path_to_local_->child(callable_name_);
    return as_single_fq_name();
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:98-100
FqName CallableId::as_single_fq_name() const {
    return class_id_ ? class_id_->as_single_fq_name().child(callable_name_) : package_name_.child(callable_name_);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:102-102
CallableId CallableId::copy(Name callable_name) const {
    return CallableId(package_name_, class_name_, std::move(callable_name), class_id_, path_to_local_);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:104-111
bool CallableId::equals(const std::any& other) const {
    const auto* value = std::any_cast<CallableId>(&other);
    if (value == this) return true;
    if (!value) return false;
    const bool same_class = class_name_ ? value->class_name_ && class_name_->equals(*value->class_name_) : !value->class_name_;
    return package_name_.equals(value->package_name_) && same_class && callable_name_.equals(value->callable_name_);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:113-119
std::int32_t CallableId::hash_code() const {
    // NOTE(port): Kotlin Int hash accumulation wraps; null contributes zero.
    std::uint32_t result = 17;
    result = result * 31U + static_cast<std::uint32_t>(package_name_.hash_code());
    result = result * 31U + (class_name_ ? static_cast<std::uint32_t>(class_name_->hash_code()) : 0U);
    result = result * 31U + static_cast<std::uint32_t>(callable_name_.hash_code());
    return std::bit_cast<std::int32_t>(result);
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:121-131
std::u16string CallableId::to_string() const {
    auto result = package_name_.as_string();
    std::replace(result.begin(), result.end(), u'.', u'/');
    result += u"/";
    if (class_name_) result += class_name_->to_string() + u".";
    result += callable_name_.to_string();
    return result;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:134-136
CallableId with_class_id(const CallableId& callable_id, ClassId class_id) {
    return CallableId(std::move(class_id), callable_id.callable_name());
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:138-138
FqName package_name(const CallableId* callable_id) {
    return callable_id ? callable_id->package_name() : CallableId::package_fq_name_for_local();
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/CallableId.kt:140-141
bool is_local(const CallableId* callable_id) {
    return !callable_id || callable_id->is_local();
}
}
