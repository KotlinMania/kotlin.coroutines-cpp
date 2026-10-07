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
// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:18-46
#include "ClassKind.hpp"
namespace org::jetbrains::kotlin::descriptors {
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:18-24
std::optional<std::u16string> code_representation(ClassKind kind) {
  switch (kind) {
    case ClassKind::CLASS: return u"class";
    case ClassKind::INTERFACE: return u"interface";
    case ClassKind::ENUM_CLASS: return u"enum class";
    case ClassKind::ENUM_ENTRY: return std::nullopt;
    case ClassKind::ANNOTATION_CLASS: return u"annotation class";
    case ClassKind::OBJECT: return u"object";
  }
  __builtin_unreachable();
}
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:26-27
bool is_singleton(ClassKind kind) { return kind == ClassKind::OBJECT || kind == ClassKind::ENUM_ENTRY; }
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:30-31
bool is_class(ClassKind kind) { return kind == ClassKind::CLASS; }
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:33-34
bool is_interface(ClassKind kind) { return kind == ClassKind::INTERFACE; }
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:36-37
bool is_enum_class(ClassKind kind) { return kind == ClassKind::ENUM_CLASS; }
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:39-40
bool is_enum_entry(ClassKind kind) { return kind == ClassKind::ENUM_ENTRY; }
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:42-43
bool is_annotation_class(ClassKind kind) { return kind == ClassKind::ANNOTATION_CLASS; }
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:45-46
bool is_object(ClassKind kind) { return kind == ClassKind::OBJECT; }
}  // namespace org::jetbrains::kotlin::descriptors
