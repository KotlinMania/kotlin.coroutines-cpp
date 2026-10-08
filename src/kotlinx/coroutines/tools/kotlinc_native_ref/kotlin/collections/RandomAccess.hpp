/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/RandomAccess.kt
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/RandomAccess.kt:8-11
#pragma once

namespace kotlin::collections {
/**
 * Marker interface indicating that the [List] implementation supports fast indexed access.
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/RandomAccess.kt:11-11
class RandomAccess {
 public:
  virtual ~RandomAccess() = default;
 protected:
  RandomAccess() = default;
};
}  // namespace kotlin::collections
