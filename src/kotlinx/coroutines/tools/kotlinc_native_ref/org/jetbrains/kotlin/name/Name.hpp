/*
 * Copyright 2010-2015 JetBrains s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// port-lint: source core/names/src/org/jetbrains/kotlin/name/Name.java
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:22-135
#pragma once

#include <any>
#include <cstdint>
#include <optional>
#include <string>

namespace org::jetbrains::kotlin::name {

// NOTE(port): Java String uses UTF-16 code units. Nullable values use optional;
// the Object argument of equals uses any, retaining its runtime type test.
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:22-135
class Name final {
 public:
  // Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:33-35
  const std::u16string& as_string() const;
  // Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:38-43
  const std::u16string& get_identifier() const;
  // Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:45-47
  bool is_special() const;
  // Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:50-53
  std::u16string as_string_strip_special_markers() const;
  // Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:56-58
  std::int32_t compare_to(const Name& that) const;
  // Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:61-63
  static Name identifier(std::u16string name);
  // Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:65-82
  static bool is_valid_identifier(const std::u16string& name);
  // Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:82-85
  static std::optional<Name> identifier_if_valid(std::u16string name);
  // Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:88-93
  static Name special(std::u16string name);
  // Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:96-103
  static Name guess_by_first_character(std::u16string name);
  // Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:106-109
  std::optional<std::u16string> get_identifier_or_null_if_special() const;
  // Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:112-114
  const std::u16string& to_string() const;
  // Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:117-127
  bool equals(const std::any& other) const;
  // Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:130-134
  std::int32_t hash_code() const;

 private:
  // Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:27-30
  Name(std::u16string name, bool special);
  std::u16string name_;
  bool special_;
};

}  // namespace org::jetbrains::kotlin::name
