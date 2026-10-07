/*
 * Copyright 2010-2020 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:10-34
#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace org::jetbrains::kotlin::name { class FqName; }

namespace org::jetbrains::kotlin::descriptors {

class EffectiveVisibility;

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:10-34
class Visibility {
 public:
  // NOTE(port): A Kotlin reference object needs a virtual C++ destructor and
  // stable identity. Name strings retain Kotlin's UTF-16 code units.
  virtual ~Visibility() = default;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:11-11
  const std::u16string& get_name() const;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:12-12
  bool is_public_api() const;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:14-15
  virtual std::u16string get_internal_display_name() const;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:17-18
  virtual std::u16string get_external_display_name() const;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:20-20
  virtual bool must_check_in_imports() const = 0;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:22-24
  virtual std::optional<std::int32_t> compare_to(const Visibility& visibility) const;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:26-26
  virtual std::u16string to_string() const final;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:28-28
  virtual const Visibility& normalize() const;
  // Should be overloaded in Java visibilities
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:30-31
  virtual const EffectiveVisibility* custom_effective_visibility() const;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:33-33
  virtual bool visible_from_package(const name::FqName& from_package,
                                    const name::FqName& my_package) const;

 protected:
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:10-13
  Visibility(std::u16string name, bool is_public_api);
  Visibility(const Visibility&) = delete;
  Visibility& operator=(const Visibility&) = delete;

 private:
  const std::u16string name_;
  const bool is_public_api_;
};

}  // namespace org::jetbrains::kotlin::descriptors
