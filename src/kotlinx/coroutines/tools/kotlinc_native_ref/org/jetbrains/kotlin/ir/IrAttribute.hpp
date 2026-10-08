/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:42-160
#pragma once

#include "IrElementBase.hpp"
#include "../../../../kotlin/Any.hpp"
#include "../../../../kotlin/reflect/KProperty.hpp"
#include "../../../../java/lang/ref/WeakReference.hpp"
#include <memory>
#include <optional>
#include <string>
#include <type_traits>

namespace org::jetbrains::kotlin::ir {
namespace ir_attribute_types {
// NOTE(port): Kotlin nested classes are independent of the outer class's type
// parameters. Keep their actual classes in a namespace, without type aliases.
template <typename E, typename T> class Delegate;
template <typename E> class Flag;
namespace flag_types { template <typename E> class Delegate; }
}
/**
 * Creates new [IrAttribute] which can be used to store additional data of type [T] inside of [E]. Designed to use as delegate, e.g.:
 * ```
 * var IrFunction.binaryName: String? by irAttribute()
 * ```
 *
 * ## When to use
 *
 * Here's a general guideline on when to choose which mechanism of associating data with [IrElement]:
 *
 * - [irAttribute]
 * 1. When the data is only used in one of the Kotlin backends.
 *    In that case, define it in either one of the dedicated files (`JvmIrAttributes.kt`, `JsIrAttributes.kt` etc.)
 *    or close to the primary usage.
 * 2. For "auxiliary" data / caches.
 * 3. When the data is expected to be `null` most of the time. `null` values are optimized away by [irAttribute].
 * - Regular properties (defined in [org.jetbrains.kotlin.ir.generator.IrTree])
 * 1. For the "primary" kind of data, which constitutes the given [IrElement]. E.g.: `IrFunction.name`.
 * 2. The above can generally be restated as "the data that should be serialized into Klib".
 * - `MutableMap<IrElement, T>`
 * 1. When the data is used in a single scope, e.g. inside a single class, function.
 * 2. When the data is used in a single lowering phase.
 * 3. When you have to enumerate over all elemenets with this data.
 *
 * @param copyByDefault Whether to copy this attribute in [IrElement.copyAttributes] by default.
 * If `false`, it will only be copied when specifying `copyAttributes(other, includeAll = true)`.
 */
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:42-43
template <typename E, typename T>
std::shared_ptr<ir_attribute_types::Delegate<E, T>> ir_attribute(bool copy_by_default) {
  return std::make_shared<ir_attribute_types::Delegate<E, T>>(copy_by_default);
}

/**
 * Creates new [IrAttribute] which can be used to put an additional mark on an [IrElement] of type [E]. Designed to use as delegate, e.g.:
 * ```
 * var IrFunction.isPublicAbi: Boolean by irFlag()
 * ```
 * ## When to use
 * See [irAttribute].
 *
 * ## [irFlag] vs [irAttribute]
 * [irFlag] is similar to `irAttribute<E, Boolean>()`, except:
 * - Boolean attribute has 3 states: `false`, `true` and `null`,
 * while a flag has 2: set or not set.
 * - It is possible to store a flag in memory a bit more efficiently.
 *
 */
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:60-61
template <typename E>
std::shared_ptr<ir_attribute_types::flag_types::Delegate<E>> ir_flag(bool copy_by_default) {
  return std::make_shared<ir_attribute_types::flag_types::Delegate<E>>(
      std::make_shared<ir_attribute_types::Delegate<E, bool>>(copy_by_default));
}

/**
 * Returns a value of [attribute], or null if the value is missing.
 */
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:67-69
template <typename E, typename T>
typename ::kotlin::collections::detail::NullableMapValue<T>::Result get(
    E& element, const std::shared_ptr<IrAttribute<E, T>>& attribute) {
  static_assert(std::is_base_of_v<IrElement, E>);
  return dynamic_cast<IrElementBase&>(element).get_attribute_internal(attribute);
}

/**
 * Stores a [value] associated with [attribute] in this IrElement, or removes an association if [value] is null.
 *
 * @return The previous value associated with the attribute, or null if the attribute was not present.
 */
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:76-78
template <typename E, typename T>
typename ::kotlin::collections::detail::NullableMapValue<T>::Result set(
    E& element, const std::shared_ptr<IrAttribute<E, T>>& attribute,
    typename ::kotlin::collections::detail::NullableMapValue<T>::Result value) {
  static_assert(std::is_base_of_v<IrElement, E>);
  return dynamic_cast<IrElementBase&>(element).set_attribute_internal(attribute, std::move(value));
}

/**
 * A key for storing additional data inside [IrElement].
 *
 * @see [irAttribute]
 * @param E restricts the type of [IrElement] on which this attribute can be stored.
 * @param T the type of the data stored in the attribute.
 */
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:87-160
template <typename E, typename T>
class IrAttribute final : public ::kotlin::Any, public detail::IrAttributeObject,
                    public std::enable_shared_from_this<IrAttribute<E, T>> {
  static_assert(std::is_base_of_v<IrElement, E>);
  static_assert(!std::is_void_v<T> && !std::is_same_v<T, std::nullptr_t>);
 public:
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:88-88
  const std::optional<std::u16string>& name() const { return name_; }
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:90-90
  bool copy_by_default() const override { return copy_by_default_; }
    /**
     * Used solely for debug, to help distinguish between multiple instances of attribute keys.
     * This may happen if the key is defined inside some class, instead of on top level.
     */
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:96-96
  const ::java::lang::ref::WeakReference<::kotlin::Any>* owner_for_debug() const {
    return owner_for_debug_.get();
  }

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:99-101
  template <typename V>
  typename ::kotlin::collections::detail::NullableMapValue<T>::Result get_value(
      E& this_ref, const ::kotlin::reflect::KProperty<V>&) {
    return ir::get(this_ref, this->shared_from_this());
  }

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:104-106
  template <typename V>
  void set_value(E& this_ref, const ::kotlin::reflect::KProperty<V>&,
                 typename ::kotlin::collections::detail::NullableMapValue<T>::Result value) {
    ir::set(this_ref, this->shared_from_this(), std::move(value));
  }

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:108-114
  std::u16string to_string() const override {
    if (name_ && owner_for_debug_ && owner_for_debug_->get()) {
      // NOTE(port): Preserve the second weak read made by source interpolation.
      // A referent reclaimed between reads is rendered as Kotlin's null text.
      const auto owner = owner_for_debug_->get();
      return *name_ + u" (inside of " + (owner ? owner->to_string() : u"null") + u")";
    } else if (name_) {
      return *name_;
    } else {
      return ::kotlin::Any::to_string();
    }
  }

 private:
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:87-96
  IrAttribute(std::optional<std::u16string> name, const std::shared_ptr<::kotlin::Any>& owner,
              bool copy_by_default)
      : name_(std::move(name)),
        owner_for_debug_(owner
            ? std::make_unique<::java::lang::ref::WeakReference<::kotlin::Any>>(owner)
            : nullptr),
        copy_by_default_(copy_by_default) {}

  // NOTE(port): A key is created by its actual delegate and retained as one
  // shared C++ object by node storage. The debug owner remains a weak reference;
  // this compiler-object storage does not impose Kotlin GC on application objects.
  const std::optional<std::u16string> name_;
  const std::unique_ptr<::java::lang::ref::WeakReference<::kotlin::Any>> owner_for_debug_;
  const bool copy_by_default_;
  friend class ir_attribute_types::Delegate<E, T>;
};

namespace ir_attribute_types {
    /**
     * See [irFlag]
     */
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:119-144
template <typename E>
class Flag final : public ::kotlin::Any {
  static_assert(std::is_base_of_v<IrElement, E>);
 public:
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:119-121
  explicit Flag(std::shared_ptr<IrAttribute<E, bool>> attribute) : attribute_(std::move(attribute)) {}

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:123-123
  template <typename V>
  bool get_value(E& this_ref, const ::kotlin::reflect::KProperty<V>&) const { return get(this_ref); }

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:126-126
  template <typename V>
  void set_value(E& this_ref, const ::kotlin::reflect::KProperty<V>&, bool value) const {
    set(this_ref, value);
  }

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:128-130
  bool get(E& element) const { return ir::get(element, attribute_) == true; }

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:132-134
  void set(E& element, bool value) const {
    ir::set(element, attribute_, value ? std::optional<bool>(true) : std::nullopt);
  }

 private:
  const std::shared_ptr<IrAttribute<E, bool>> attribute_;
};

namespace flag_types {
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:136-143
template <typename E>
class Delegate final : public ::kotlin::Any {
  static_assert(std::is_base_of_v<IrElement, E>);
 public:
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:136-138
  explicit Delegate(std::shared_ptr<ir_attribute_types::Delegate<E, bool>> attribute_delegate)
      : attribute_delegate_(std::move(attribute_delegate)) {}

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:139-142
  template <typename V>
  std::shared_ptr<Flag<E>> provide_delegate(const std::shared_ptr<::kotlin::Any>& this_ref,
                           const ::kotlin::reflect::KProperty<V>& property) const {
    auto attribute = attribute_delegate_->provide_delegate(this_ref, property);
    return std::make_shared<Flag<E>>(std::move(attribute));
  }

 private:
  const std::shared_ptr<ir_attribute_types::Delegate<E, bool>> attribute_delegate_;
};
}  // namespace flag_types
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:146-159
template <typename E, typename T>
class Delegate final : public ::kotlin::Any {
  static_assert(std::is_base_of_v<IrElement, E>);
 public:
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:146-148
  explicit Delegate(bool copy_by_default) : copy_by_default_(copy_by_default) {}

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:149-155
  std::shared_ptr<IrAttribute<E, T>> create(const std::shared_ptr<::kotlin::Any>& owner,
                                         std::optional<std::u16string> name) const {
    return std::shared_ptr<IrAttribute<E, T>>(new IrAttribute<E, T>(std::move(name), owner, copy_by_default_));
  }

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:157-158
  template <typename V>
  std::shared_ptr<IrAttribute<E, T>> provide_delegate(
      const std::shared_ptr<::kotlin::Any>& this_ref, const ::kotlin::reflect::KProperty<V>& property) const {
    return create(this_ref, property.name());
  }

 private:
  const bool copy_by_default_;
};

}  // namespace ir_attribute_types

}  // namespace org::jetbrains::kotlin::ir
