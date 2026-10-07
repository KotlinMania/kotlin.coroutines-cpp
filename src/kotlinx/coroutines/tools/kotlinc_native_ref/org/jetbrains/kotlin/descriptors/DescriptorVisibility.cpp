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
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:23-86
#include "DescriptorVisibility.hpp"

namespace org::jetbrains::kotlin::descriptors {

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:26-27
const std::u16string& DescriptorVisibility::get_name() const {
  return get_delegate().get_name();
}

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:29-30
bool DescriptorVisibility::is_public_api() const { return get_delegate().is_public_api(); }

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:68-70
std::optional<std::int32_t> DescriptorVisibility::compare_to(
    const DescriptorVisibility& visibility) const {
  return get_delegate().compare_to(visibility.get_delegate());
}

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:78-78
std::u16string DescriptorVisibility::to_string() const { return get_delegate().to_string(); }

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:82-83
const EffectiveVisibility* DescriptorVisibility::custom_effective_visibility() const {
  return get_delegate().custom_effective_visibility();
}

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:85-85
bool DescriptorVisibility::visible_from_package(const name::FqName&, const name::FqName&) const {
  return true;
}

}  // namespace org::jetbrains::kotlin::descriptors
