/*
 * Copyright 2010-2020 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:8-89
#include "Visibilities.hpp"

#include <bit>
#include <stdexcept>
#include <unordered_map>

namespace org::jetbrains::kotlin::descriptors {

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:8-89
const Visibilities Visibilities::INSTANCE;

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:9-11
const Visibilities::Private Visibilities::Private::INSTANCE;

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:9-11
Visibilities::Private::Private() : Visibility(u"private", false) {}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:10-10
bool Visibilities::Private::must_check_in_imports() const {
  return true;
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:14-19
const Visibilities::PrivateToThis Visibilities::PrivateToThis::INSTANCE;

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:14-19
Visibilities::PrivateToThis::PrivateToThis() : Visibility(u"private_to_this", false) {}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:18-18
bool Visibilities::PrivateToThis::must_check_in_imports() const {
  return true;
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:15-16
std::u16string Visibilities::PrivateToThis::get_internal_display_name() const {
  return u"private/*private to this*/";
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:21-23
const Visibilities::Protected Visibilities::Protected::INSTANCE;

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:21-23
Visibilities::Protected::Protected() : Visibility(u"protected", true) {}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:22-22
bool Visibilities::Protected::must_check_in_imports() const {
  return false;
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:25-27
const Visibilities::Internal Visibilities::Internal::INSTANCE;

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:25-27
Visibilities::Internal::Internal() : Visibility(u"internal", false) {}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:26-26
bool Visibilities::Internal::must_check_in_imports() const {
  return true;
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:29-31
const Visibilities::Public Visibilities::Public::INSTANCE;

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:29-31
Visibilities::Public::Public() : Visibility(u"public", true) {}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:30-30
bool Visibilities::Public::must_check_in_imports() const {
  return false;
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:33-35
const Visibilities::Local Visibilities::Local::INSTANCE;

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:33-35
Visibilities::Local::Local() : Visibility(u"local", false) {}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:34-34
bool Visibilities::Local::must_check_in_imports() const {
  return true;
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:37-41
const Visibilities::Inherited Visibilities::Inherited::INSTANCE;

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:37-41
Visibilities::Inherited::Inherited() : Visibility(u"inherited", false) {}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:38-40
bool Visibilities::Inherited::must_check_in_imports() const {
  throw std::logic_error("This method shouldn't be invoked for INHERITED visibility");
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:43-48
const Visibilities::InvisibleFake Visibilities::InvisibleFake::INSTANCE;

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:43-48
Visibilities::InvisibleFake::InvisibleFake() : Visibility(u"invisible_fake", false) {}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:44-44
bool Visibilities::InvisibleFake::must_check_in_imports() const {
  return true;
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:46-47
std::u16string Visibilities::InvisibleFake::get_external_display_name() const {
  return u"invisible (private in a supertype)";
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:50-54
const Visibilities::Unknown Visibilities::Unknown::INSTANCE;

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:50-54
Visibilities::Unknown::Unknown() : Visibility(u"unknown", false) {}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:51-53
bool Visibilities::Unknown::must_check_in_imports() const {
  throw std::logic_error("This method shouldn't be invoked for UNKNOWN visibility");
}

namespace {

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:56-62
const std::unordered_map<const Visibility*, std::int32_t> ORDERED_VISIBILITIES = {
    {&Visibilities::PrivateToThis::INSTANCE, 0},
    {&Visibilities::Private::INSTANCE, 0},
    {&Visibilities::Internal::INSTANCE, 1},
    {&Visibilities::Protected::INSTANCE, 1},
    {&Visibilities::Public::INSTANCE, 2},
};

}  // namespace

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:64-73
std::optional<std::int32_t> Visibilities::compare(const Visibility& first,
                                                  const Visibility& second) {
  const auto result = first.compare_to(second);
  if (result.has_value()) {
    return result;
  }
  const auto opposite_result = second.compare_to(first);
  if (opposite_result.has_value()) {
    // NOTE(port): Kotlin Int negation wraps; unsigned arithmetic avoids C++
    // signed-overflow undefined behavior for a custom comparison returning MIN.
    return std::bit_cast<std::int32_t>(
        std::uint32_t{0} - std::bit_cast<std::uint32_t>(*opposite_result));
  }
  return std::nullopt;
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:75-82
std::optional<std::int32_t> Visibilities::compare_local(const Visibility& first,
                                                        const Visibility& second) {
  if (&first == &second) return 0;
  const auto first_entry = ORDERED_VISIBILITIES.find(&first);
  const auto second_entry = ORDERED_VISIBILITIES.find(&second);
  if (first_entry == ORDERED_VISIBILITIES.end() || second_entry == ORDERED_VISIBILITIES.end()) {
    return std::nullopt;
  }
  const auto first_index = first_entry->second;
  const auto second_index = second_entry->second;
  return first_index == second_index ? std::nullopt
                                    : std::optional<std::int32_t>(first_index - second_index);
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:84-86
bool Visibilities::is_private(const Visibility& visibility) {
  return &visibility == &Private::INSTANCE || &visibility == &PrivateToThis::INSTANCE;
}

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:88-88
const Visibilities::Public& Visibilities::DEFAULT_VISIBILITY = Public::INSTANCE;

}  // namespace org::jetbrains::kotlin::descriptors
