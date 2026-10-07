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
#pragma once
#include <optional>
#include <string>
namespace org::jetbrains::kotlin::descriptors {
// NOTE(port): Kotlin enum properties/extensions map to free C++ functions.
// Source member-function/constant-identity gaps remain explicit in the deep audit.
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:18-28
enum class ClassKind { CLASS, INTERFACE, ENUM_CLASS, ENUM_ENTRY, ANNOTATION_CLASS, OBJECT };
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:18-24
std::optional<std::u16string> code_representation(ClassKind kind);
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:26-27
bool is_singleton(ClassKind kind);
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:30-31
bool is_class(ClassKind kind);
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:33-34
bool is_interface(ClassKind kind);
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:36-37
bool is_enum_class(ClassKind kind);
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:39-40
bool is_enum_entry(ClassKind kind);
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:42-43
bool is_annotation_class(ClassKind kind);
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/ClassKind.kt:45-46
bool is_object(ClassKind kind);
}  // namespace org::jetbrains::kotlin::descriptors
