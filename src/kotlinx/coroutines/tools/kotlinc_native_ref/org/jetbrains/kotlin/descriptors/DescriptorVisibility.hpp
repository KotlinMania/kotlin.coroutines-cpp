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
#pragma once

#include "Visibility.hpp"
#include "DeclarationDescriptorWithVisibility.hpp"

namespace org::jetbrains::kotlin::resolve::scopes::receivers { class ReceiverValue; }

namespace org::jetbrains::kotlin::descriptors {

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:23-86
class DescriptorVisibility {
 public:
  virtual ~DescriptorVisibility() = default;
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:24-24
  virtual const Visibility& get_delegate() const = 0;
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:26-27
  const std::u16string& get_name() const;
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:29-30
  bool is_public_api() const;
    /**
     * @param receiver can be used to determine callee accessibility for some special receiver value
     *
     * 'null'-value basically means that receiver is absent in current call
     *
     * In case if it's needed to perform basic checks ignoring ones considering receiver (e.g. when checks happen beyond any call),
     * special value Visibilities.ALWAYS_SUITABLE_RECEIVER should be used.
     * If it's needed to determine whether visibility accepts any receiver, Visibilities.IRRELEVANT_RECEIVER should be used.
     *
     * NB: Currently Visibilities.IRRELEVANT_RECEIVER has the same effect as 'null'
     *
     * Also it's important that implementation that take receiver into account do aware about these special values.
     */
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:45-51
  virtual bool is_visible(const resolve::scopes::receivers::ReceiverValue* receiver,
                          const DeclarationDescriptorWithVisibility& what,
                          const DeclarationDescriptor& from,
                          bool use_special_rules_for_private_sealed_constructors) const = 0;
    /**
     * True, if it makes sense to check this visibility in imports and not import inaccessible declarations with such visibility.
     * Hint: return true, if this visibility can be checked on file's level.
     * Examples:
     * it returns false for PROTECTED because protected members of classes can be imported to be used in subclasses of their containers,
     * so when we are looking at the import, we don't know whether it is legal somewhere in this file or not.
     * it returns true for INTERNAL, because an internal declaration is either visible everywhere in a file, or invisible everywhere in the same file.
     * it returns true for PRIVATE, because there's no point in importing privates: they are inaccessible unless their short name is
     * already available without an import
     */
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:63-63
  virtual bool must_check_in_imports() const = 0;
    /**
     * @return null if the answer is unknown
     */
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:68-70
  std::optional<std::int32_t> compare_to(const DescriptorVisibility& visibility) const;

  // internal representation for descriptors
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:73-73
  virtual std::u16string get_internal_display_name() const = 0;
  // external representation for diagnostics
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:76-76
  virtual std::u16string get_external_display_name() const = 0;
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:78-78
  virtual std::u16string to_string() const final;
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:80-80
  virtual const DescriptorVisibility& normalize() const = 0;
  // Should be overloaded in Java visibilities
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:82-83
  const EffectiveVisibility* custom_effective_visibility() const;
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:85-85
  virtual bool visible_from_package(const name::FqName& from_package,
                                    const name::FqName& my_package) const;

 protected:
  DescriptorVisibility() = default;
  // NOTE(port): Preserve source reference identity and compiler-owned lifetime.
  DescriptorVisibility(const DescriptorVisibility&) = delete;
  DescriptorVisibility& operator=(const DescriptorVisibility&) = delete;
};

}  // namespace org::jetbrains::kotlin::descriptors
