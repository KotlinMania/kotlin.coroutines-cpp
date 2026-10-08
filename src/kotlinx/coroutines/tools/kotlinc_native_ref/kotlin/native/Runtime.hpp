/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/native/Runtime.kt
// port-lint: source kotlin-native/runtime/src/main/cpp/Natives.cpp
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/Runtime.kt:93-100
#pragma once

#include <cstdint>

namespace kotlin::native {
/**
 * Compute stable wrt potential object relocations by the memory manager identity hash code.
 * @return 0 for `null` object, identity hash code otherwise.
 */
// NOTE(port): A borrowed erased pointer retains its caller's object identity.
// This operation requires neither Native object layout nor a Kotlin runtime.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/Runtime.kt:97-100
// Transliterated from: kotlin-native/runtime/src/main/cpp/Natives.cpp:40-49
std::int32_t identity_hash_code(const void* object);
}  // namespace kotlin::native
