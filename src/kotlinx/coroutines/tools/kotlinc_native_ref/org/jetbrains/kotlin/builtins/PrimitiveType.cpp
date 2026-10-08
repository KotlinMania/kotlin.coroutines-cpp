// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:11-60
#include "PrimitiveType.hpp"
#include "StandardNames.hpp"

namespace org::jetbrains::kotlin::builtins {
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:12-19
const PrimitiveType PrimitiveType::BOOLEAN(u"Boolean");
const PrimitiveType PrimitiveType::CHAR(u"Char");
const PrimitiveType PrimitiveType::BYTE(u"Byte");
const PrimitiveType PrimitiveType::SHORT(u"Short");
const PrimitiveType PrimitiveType::INT(u"Int");
const PrimitiveType PrimitiveType::FLOAT(u"Float");
const PrimitiveType PrimitiveType::LONG(u"Long");
const PrimitiveType PrimitiveType::DOUBLE(u"Double");

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:11-24
PrimitiveType::PrimitiveType(std::u16string type_name)
    : type_name_(name::Name::identifier(type_name)),
      array_type_name_(name::Name::identifier(type_name + u"Array")) {}
// NOTE(port): Release only the winning published boxes; losing candidates
// are destroyed by their unique_ptr in the getter that computed them.
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:26-28
PrimitiveType::~PrimitiveType() {
    delete type_fq_name_.load(std::memory_order_relaxed);
    delete array_type_fq_name_.load(std::memory_order_relaxed);
}
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:22-22
const name::Name& PrimitiveType::type_name() const { return type_name_; }
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:24-24
const name::Name& PrimitiveType::array_type_name() const { return array_type_name_; }
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:26-26
// Transliterated from: libraries/stdlib/jvm/src/kotlin/util/LazyJVM.kt:114-133
const name::FqName& PrimitiveType::type_fq_name() const {
    auto value = type_fq_name_.load(std::memory_order_acquire);
    if (!value) {
        auto candidate = std::make_unique<const name::FqName>(
            StandardNames::built_ins_package_fq_name().child(type_name_));
        if (type_fq_name_.compare_exchange_strong(value, candidate.get(),
                std::memory_order_acq_rel, std::memory_order_acquire))
            value = candidate.release();
    }
    return *value;
}
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:28-28
// Transliterated from: libraries/stdlib/jvm/src/kotlin/util/LazyJVM.kt:114-133
const name::FqName& PrimitiveType::array_type_fq_name() const {
    auto value = array_type_fq_name_.load(std::memory_order_acquire);
    if (!value) {
        auto candidate = std::make_unique<const name::FqName>(
            StandardNames::built_ins_package_fq_name().child(array_type_name_));
        if (array_type_fq_name_.compare_exchange_strong(value, candidate.get(),
                std::memory_order_acq_rel, std::memory_order_acquire))
            value = candidate.release();
    }
    return *value;
}
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:34-45
const PrimitiveType* PrimitiveType::get_by_short_name(const std::u16string& name) {
    if (name == u"Boolean") return &BOOLEAN;
    if (name == u"Char") return &CHAR;
    if (name == u"Byte") return &BYTE;
    if (name == u"Short") return &SHORT;
    if (name == u"Int") return &INT;
    if (name == u"Float") return &FLOAT;
    if (name == u"Long") return &LONG;
    if (name == u"Double") return &DOUBLE;
    return nullptr;
}
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:47-58
const PrimitiveType* PrimitiveType::get_by_short_array_name(const std::u16string& name) {
    if (name == u"BooleanArray") return &BOOLEAN;
    if (name == u"CharArray") return &CHAR;
    if (name == u"ByteArray") return &BYTE;
    if (name == u"ShortArray") return &SHORT;
    if (name == u"IntArray") return &INT;
    if (name == u"FloatArray") return &FLOAT;
    if (name == u"LongArray") return &LONG;
    if (name == u"DoubleArray") return &DOUBLE;
    return nullptr;
}
}
