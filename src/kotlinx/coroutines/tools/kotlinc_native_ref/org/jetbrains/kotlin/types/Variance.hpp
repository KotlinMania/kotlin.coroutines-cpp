/*
 * Copyright 2010-2020 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source core/language.model/src/org/jetbrains/kotlin/types/Variance.kt
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:8-44
#pragma once
#include <string_view>

namespace org::jetbrains::kotlin::types {
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:8-16
enum class Variance { INVARIANT, IN_VARIANCE, OUT_VARIANCE };
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:9-16
std::u16string_view label(Variance variance);
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:10-16
bool allows_in_position(Variance variance);
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:11-16
bool allows_out_position(Variance variance);
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:18-23
bool allows_position(Variance variance, Variance position);
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:25-33
Variance superpose(Variance variance, Variance other);
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:35-41
Variance opposite(Variance variance);
// Transliterated from: core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:43-43
std::u16string_view to_string(Variance variance);
}  // namespace org::jetbrains::kotlin::types
