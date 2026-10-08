// port-lint: source native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt:8-20
#pragma once
#include "kotlin/sequences/Sequence.hpp"
#include <memory>
#include <optional>
#include <utility>

namespace org::jetbrains::kotlin::backend::konan {
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt:18-20
enum class PrimitiveBinaryType {
    BOOLEAN, BYTE, SHORT, INT, LONG, FLOAT, DOUBLE, POINTER, VECTOR128
};

// NOTE(port): Erase the internal family's out-type at its common boundary.
// Reference<T> retains its genuine typed Sequence; Primitive requires no
// fabricated C++ Nothing object. The private constructor seals the variants.
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt:8-11
class BinaryType {
public:
    class Primitive;
    template <typename T> class Reference;
    virtual ~BinaryType();
    BinaryType(const BinaryType&) = delete;
    BinaryType& operator=(const BinaryType&) = delete;
private:
    BinaryType();
};

// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt:9-9
class BinaryType::Primitive final : public BinaryType {
public:
    explicit Primitive(PrimitiveBinaryType type);
    PrimitiveBinaryType type() const;
private:
    const PrimitiveBinaryType type_;
};

// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt:10-10
template <typename T>
class BinaryType::Reference final : public BinaryType {
public:
    // NOTE(port): Retain the supplied sequence owner without iterating or copying
    // its elements. Element pointers retain their own source ownership policy.
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt:10-10
    Reference(std::shared_ptr<::kotlin::sequences::Sequence<T>> types, bool nullable)
        : types_(std::move(types)), nullable_(nullable) {}
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt:10-10
    const std::shared_ptr<::kotlin::sequences::Sequence<T>>& types() const { return types_; }
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt:10-10
    bool nullable() const { return nullable_; }
private:
    const std::shared_ptr<::kotlin::sequences::Sequence<T>> types_;
    const bool nullable_;
};

// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/BinaryType.kt:13-16
std::optional<PrimitiveBinaryType> primitive_binary_type_or_null(const BinaryType& type);
}
