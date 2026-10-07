/*
 * Copyright 2010-2020 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:10-34
#include "Visibility.hpp"
#include "Visibilities.hpp"
#include <utility>

namespace org::jetbrains::kotlin::descriptors {

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:10-13
Visibility::Visibility(std::u16string name, bool is_public_api)
    : name_(std::move(name)), is_public_api_(is_public_api) {}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:11-11
const std::u16string& Visibility::get_name() const { return name_; }

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:12-12
bool Visibility::is_public_api() const { return is_public_api_; }

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:14-15
std::u16string Visibility::get_internal_display_name() const { return name_; }

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:17-18
std::u16string Visibility::get_external_display_name() const {
  return get_internal_display_name();
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:22-24
std::optional<std::int32_t> Visibility::compare_to(const Visibility& visibility) const {
  return Visibilities::compare_local(*this, visibility);
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:26-26
std::u16string Visibility::to_string() const { return get_internal_display_name(); }

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:28-28
const Visibility& Visibility::normalize() const { return *this; }

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:30-31
const EffectiveVisibility* Visibility::custom_effective_visibility() const {
  return nullptr;
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:33-33
bool Visibility::visible_from_package(const name::FqName&, const name::FqName&) const {
  return true;
}

}  // namespace org::jetbrains::kotlin::descriptors
