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
#include "Name.hpp"

#include <algorithm>
#include <bit>
#include <stdexcept>
#include <utility>

namespace org::jetbrains::kotlin::name {
namespace {

// NOTE(port): C++ exception messages are byte strings. Encode UTF-16 diagnostics
// as UTF-8, preserving lone surrogate code units as WTF-8. Name storage,
// comparison and hashing continue to operate on the original UTF-16 units.
std::string diagnostic_string(const std::u16string& text) {
  std::string result;
  for (std::size_t i = 0; i < text.size(); ++i) {
    std::uint32_t ch = text[i];
    if (ch >= 0xd800 && ch <= 0xdbff && i + 1 < text.size() &&
        text[i + 1] >= 0xdc00 && text[i + 1] <= 0xdfff) {
      ch = 0x10000 + ((ch - 0xd800) << 10) + (text[++i] - 0xdc00);
    }
    if (ch < 0x80) {
      result.push_back(static_cast<char>(ch));
    } else if (ch < 0x800) {
      result.push_back(static_cast<char>(0xc0 | (ch >> 6)));
      result.push_back(static_cast<char>(0x80 | (ch & 0x3f)));
    } else if (ch < 0x10000) {
      result.push_back(static_cast<char>(0xe0 | (ch >> 12)));
      result.push_back(static_cast<char>(0x80 | ((ch >> 6) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | (ch & 0x3f)));
    } else {
      result.push_back(static_cast<char>(0xf0 | (ch >> 18)));
      result.push_back(static_cast<char>(0x80 | ((ch >> 12) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | ((ch >> 6) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | (ch & 0x3f)));
    }
  }
  return result;
}

}  // namespace

// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:27-30
Name::Name(std::u16string name, bool special)
    : name_(std::move(name)), special_(special) {}

// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:33-35
const std::u16string& Name::as_string() const { return name_; }

// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:38-43
const std::u16string& Name::get_identifier() const {
  if (special_) {
    // NOTE(port): Java IllegalStateException maps to logic_error.
    throw std::logic_error("not identifier: " + diagnostic_string(to_string()));
  }
  return as_string();
}

// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:45-47
bool Name::is_special() const { return special_; }

// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:50-53
std::u16string Name::as_string_strip_special_markers() const {
  if (is_special()) {
    // NOTE(port): Java substring(1, length - 1) rejects reversed bounds.
    // C++ substr's count would underflow for the source-accepted name "<".
    if (as_string().size() < 2) throw std::out_of_range("substring(1, length - 1)");
    return as_string().substr(1, as_string().size() - 2);
  }
  return as_string();
}

// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:56-58
std::int32_t Name::compare_to(const Name& that) const {
  // NOTE(port): Java String.compareTo returns the first UTF-16 unit difference,
  // or the length difference. std::u16string::compare promises only the sign.
  const auto limit = std::min(name_.size(), that.name_.size());
  for (std::size_t i = 0; i < limit; ++i) {
    if (name_[i] != that.name_[i]) {
      return static_cast<std::int32_t>(name_[i]) - static_cast<std::int32_t>(that.name_[i]);
    }
  }
  return static_cast<std::int32_t>(name_.size()) - static_cast<std::int32_t>(that.name_.size());
}

// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:61-63
Name Name::identifier(std::u16string name) { return Name(std::move(name), false); }

// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:65-82
bool Name::is_valid_identifier(const std::u16string& name) {
  // JVM spec:
  // 4.2.2. Unqualified Names
  // Names of methods, fields, local variables, and formal parameters must not
  // contain any of the ASCII characters . ; [ / (period, semicolon, left square
  // bracket, or forward slash).
  if (name.empty() || name.starts_with(u"<")) return false;
  for (std::size_t i = 0; i < name.size(); ++i) {
    const auto ch = name[i];
    if (ch == u'.' || ch == u';' || ch == u'[' || ch == u'/') return false;
  }
  return true;
}

// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:82-85
std::optional<Name> Name::identifier_if_valid(std::u16string name) {
  if (!is_valid_identifier(name)) return std::nullopt;
  return identifier(std::move(name));
}

// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:88-93
Name Name::special(std::u16string name) {
  if (!name.starts_with(u"<")) {
    // NOTE(port): Java IllegalArgumentException maps to invalid_argument.
    throw std::invalid_argument("special name must start with '<': " + diagnostic_string(name));
  }
  return Name(std::move(name), true);
}

// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:96-103
Name Name::guess_by_first_character(std::u16string name) {
  if (name.starts_with(u"<")) return special(std::move(name));
  return identifier(std::move(name));
}

// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:106-109
std::optional<std::u16string> Name::get_identifier_or_null_if_special() const {
  if (special_) return std::nullopt;
  return as_string();
}

// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:112-114
const std::u16string& Name::to_string() const { return name_; }

// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:117-127
bool Name::equals(const std::any& other) const {
  const auto* name1 = std::any_cast<Name>(&other);
  if (this == name1) return true;
  if (name1 == nullptr) return false;
  if (special_ != name1->special_) return false;
  if (name_ != name1->name_) return false;
  return true;
}

// Transliterated from: core/names/src/org/jetbrains/kotlin/name/Name.java:130-134
std::int32_t Name::hash_code() const {
  // NOTE(port): Java String.hashCode folds UTF-16 units with multiplier 31.
  // Unsigned arithmetic preserves Java int overflow without C++ signed UB.
  std::uint32_t result = 0;
  for (const auto ch : name_) result = 31 * result + ch;
  result = 31 * result + (special_ ? 1 : 0);
  return std::bit_cast<std::int32_t>(result);
}

}  // namespace org::jetbrains::kotlin::name
