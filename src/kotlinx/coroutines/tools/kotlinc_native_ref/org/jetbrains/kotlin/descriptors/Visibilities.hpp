/*
 * Copyright 2010-2020 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:8-89
#pragma once

#include "Visibility.hpp"

namespace org::jetbrains::kotlin::descriptors {

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:8-89
class Visibilities final {
 public:
  // NOTE(port): Source objects are singleton types. The C++ nested type and
  // INSTANCE retain both the public type and its unique object identity.
  static const Visibilities INSTANCE;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:9-11
  class Private final : public Visibility {
   public:
    static const Private INSTANCE;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:10-10
    bool must_check_in_imports() const override;
   private:
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:9-11
    Private();
  };
  // K2 doesn't use this visibility, see KT-55446 for details
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:14-19
  class PrivateToThis final : public Visibility {
   public:
    static const PrivateToThis INSTANCE;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:18-18
    bool must_check_in_imports() const override;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:15-16
    std::u16string get_internal_display_name() const override;
   private:
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:14-19
    PrivateToThis();
  };
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:21-23
  class Protected final : public Visibility {
   public:
    static const Protected INSTANCE;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:22-22
    bool must_check_in_imports() const override;
   private:
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:21-23
    Protected();
  };
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:25-27
  class Internal final : public Visibility {
   public:
    static const Internal INSTANCE;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:26-26
    bool must_check_in_imports() const override;
   private:
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:25-27
    Internal();
  };
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:29-31
  class Public final : public Visibility {
   public:
    static const Public INSTANCE;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:30-30
    bool must_check_in_imports() const override;
   private:
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:29-31
    Public();
  };
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:33-35
  class Local final : public Visibility {
   public:
    static const Local INSTANCE;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:34-34
    bool must_check_in_imports() const override;
   private:
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:33-35
    Local();
  };
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:37-41
  class Inherited final : public Visibility {
   public:
    static const Inherited INSTANCE;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:38-40
    bool must_check_in_imports() const override;
   private:
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:37-41
    Inherited();
  };
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:43-48
  class InvisibleFake final : public Visibility {
   public:
    static const InvisibleFake INSTANCE;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:44-44
    bool must_check_in_imports() const override;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:46-47
    std::u16string get_external_display_name() const override;
   private:
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:43-48
    InvisibleFake();
  };
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:50-54
  class Unknown final : public Visibility {
   public:
    static const Unknown INSTANCE;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:51-53
    bool must_check_in_imports() const override;
   private:
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:50-54
    Unknown();
  };
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:64-73
  static std::optional<std::int32_t> compare(const Visibility& first,
                                             const Visibility& second);
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:84-86
  static bool is_private(const Visibility& visibility);
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:88-88
  static const Public& DEFAULT_VISIBILITY;

 private:
  friend class Visibility;
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:75-82
  static std::optional<std::int32_t> compare_local(const Visibility& first,
                                                 const Visibility& second);
  Visibilities() = default;
  Visibilities(const Visibilities&) = delete;
  Visibilities& operator=(const Visibilities&) = delete;
};

}  // namespace org::jetbrains::kotlin::descriptors
