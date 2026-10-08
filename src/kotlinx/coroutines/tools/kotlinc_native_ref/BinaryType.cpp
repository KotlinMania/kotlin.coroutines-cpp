// port-lint: source native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt:8-16
#include "BinaryType.hpp"

namespace org::jetbrains::kotlin::backend::konan {
// NOTE(port): Polymorphic C++ lifetime and sealed-family construction.
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt:8-11
BinaryType::BinaryType() = default;
BinaryType::~BinaryType() = default;
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt:9-9
BinaryType::Primitive::Primitive(PrimitiveBinaryType type) : type_(type) {}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt:9-9
PrimitiveBinaryType BinaryType::Primitive::type() const { return type_; }
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt:13-16
std::optional<PrimitiveBinaryType> primitive_binary_type_or_null(const BinaryType& type) {
    if (const auto* primitive = dynamic_cast<const BinaryType::Primitive*>(&type))
        return primitive->type();
    return std::nullopt;
}
}
