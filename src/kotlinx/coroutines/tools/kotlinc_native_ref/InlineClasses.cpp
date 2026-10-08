// port-lint: source native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:50-70
#include "InlineClasses.hpp"
#include "KonanFqNames.hpp"
#include "org/jetbrains/kotlin/builtins/PrimitiveType.hpp"
#include <array>
#include <utility>
namespace org::jetbrains::kotlin::backend::konan {
namespace {
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:50-68
class KonanPrimitiveProperties final {
public:
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:67-68
    KonanPrimitiveProperties(name::ClassId class_id, PrimitiveBinaryType type)
        : class_id_(std::move(class_id)), binary_type_(type) {}
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:64-65
    KonanPrimitiveProperties(const builtins::PrimitiveType& type, PrimitiveBinaryType binary_type)
        : KonanPrimitiveProperties(name::ClassId::top_level(type.type_fq_name()), binary_type) {}
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:50-50
    const name::ClassId& class_id() const { return class_id_; }
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:50-50
    const BinaryType::Primitive& binary_type() const { return binary_type_; }
private:
    const name::ClassId class_id_;
    const BinaryType::Primitive binary_type_;
};
// NOTE(port): Keep each source enum instance's properties in a retained catalog.
// Initialization occurs on first property access.
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:51-60
const KonanPrimitiveProperties& properties(KonanPrimitiveType type) {
    using builtins::PrimitiveType;
    static const std::array<KonanPrimitiveProperties, 10> PROPERTIES{{
        {PrimitiveType::BOOLEAN, PrimitiveBinaryType::BOOLEAN},
        {PrimitiveType::CHAR, PrimitiveBinaryType::SHORT},
        {PrimitiveType::BYTE, PrimitiveBinaryType::BYTE},
        {PrimitiveType::SHORT, PrimitiveBinaryType::SHORT},
        {PrimitiveType::INT, PrimitiveBinaryType::INT},
        {PrimitiveType::LONG, PrimitiveBinaryType::LONG},
        {PrimitiveType::FLOAT, PrimitiveBinaryType::FLOAT},
        {PrimitiveType::DOUBLE, PrimitiveBinaryType::DOUBLE},
        {name::ClassId::top_level(KonanFqNames::non_null_native_ptr().to_safe()), PrimitiveBinaryType::POINTER},
        {name::ClassId::top_level(KonanFqNames::vector128()), PrimitiveBinaryType::VECTOR128}
    }};
    return PROPERTIES.at(static_cast<std::size_t>(type));
}
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:50-50
const name::ClassId& class_id(KonanPrimitiveType type) { return properties(type).class_id(); }
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:50-50
const BinaryType::Primitive& binary_type(KonanPrimitiveType type) { return properties(type).binary_type(); }
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:70-70
name::FqNameUnsafe fq_name(KonanPrimitiveType type) { return class_id(type).as_single_fq_name().to_unsafe(); }
}
