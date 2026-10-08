/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:33-69
#pragma once

#include <any>
#include <tuple>
#include <utility>

namespace kotlin::collections::detail {

// NOTE(port): Public out-type specializations use their genuine element
// supertypes. The compiler descriptor hierarchy supplies its source edges.
template <typename T> struct ElementSupertypes { using Types = std::tuple<std::any>; };
template <> struct ElementSupertypes<std::any> { using Types = std::tuple<>; };

// NOTE(port): Boxing crosses the private generic virtual boundary only.
// Source reference-object codecs retain canonical references, not copies.
template <typename T> struct ElementCodec {
  static std::any box(T value) { return std::any(std::move(value)); }
  static T unbox(const std::any& value) { return std::any_cast<T>(value); }
};
template <> struct ElementCodec<std::any> {
  static std::any box(std::any value) { return value; }
  static std::any unbox(const std::any& value) { return value; }
};

template <template <typename> class Interface, typename Types> struct CovariantBases;
template <template <typename> class Interface, typename... Types>
struct CovariantBases<Interface, std::tuple<Types...>> : public virtual Interface<Types>... {};

}  // namespace kotlin::collections::detail
