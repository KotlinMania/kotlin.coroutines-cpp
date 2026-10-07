/*
 * Copyright 2010-2020 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source core/language.model/src/org/jetbrains/kotlin/types/Variance.kt
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:8-44
#include "Variance.hpp"
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace org::jetbrains::kotlin::types {
namespace {
// NOTE(port): Store the Kotlin enum constructor properties with each enum value.
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:8-16
struct VarianceProperties {
  std::u16string_view label;
  bool allows_in_position;
  bool allows_out_position;
  std::int32_t superposition_factor;
};
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:14-16
constexpr std::array<VarianceProperties, 3> VARIANCE_PROPERTIES{{
    {u"", true, true, 0}, {u"in", true, false, -1}, {u"out", false, true, +1}}};
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:8-16
const VarianceProperties& properties(Variance variance) {
  return VARIANCE_PROPERTIES[static_cast<std::size_t>(variance)];
}
}  // namespace
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:9-16
std::u16string_view label(Variance variance) { return properties(variance).label; }
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:10-16
bool allows_in_position(Variance variance) { return properties(variance).allows_in_position; }
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:11-16
bool allows_out_position(Variance variance) { return properties(variance).allows_out_position; }
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:18-23
bool allows_position(Variance variance, Variance position) {
  switch (position) {
    case Variance::IN_VARIANCE: return allows_in_position(variance);
    case Variance::OUT_VARIANCE: return allows_out_position(variance);
    case Variance::INVARIANT: return allows_in_position(variance) && allows_out_position(variance);
  }
  __builtin_unreachable();
}
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:25-33
Variance superpose(Variance variance, Variance other) {
  const auto r = properties(variance).superposition_factor * properties(other).superposition_factor;
  switch (r) {
    case 0: return Variance::INVARIANT;
    case -1: return Variance::IN_VARIANCE;
    case +1: return Variance::OUT_VARIANCE;
    default: throw std::logic_error("Illegal factor: " + std::to_string(r));
  }
}
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:35-41
Variance opposite(Variance variance) {
  switch (variance) {
    case Variance::INVARIANT: return Variance::INVARIANT;
    case Variance::IN_VARIANCE: return Variance::OUT_VARIANCE;
    case Variance::OUT_VARIANCE: return Variance::IN_VARIANCE;
  }
  __builtin_unreachable();
}
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:43-43
std::u16string_view to_string(Variance variance) { return label(variance); }
}  // namespace org::jetbrains::kotlin::types
