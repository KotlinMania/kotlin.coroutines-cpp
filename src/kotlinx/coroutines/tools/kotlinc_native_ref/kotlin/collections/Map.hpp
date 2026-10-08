/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:43-245
#pragma once
#include "Set.hpp"
#include <optional>
#include <type_traits>
namespace kotlin::collections {
template <typename K, typename V> class Map;
template <typename K, typename V> class MutableMap;
// NOTE(port): Kotlin's static nested Entry types use namespaces in C++ so
// their identity does not capture an enclosing Map template specialization.
namespace map { template <typename K, typename V> class Entry; }
namespace mutable_map { template <typename K, typename V> class MutableEntry; }
namespace detail {
// NOTE(port): This is the private virtual boundary for the actual source Entry
// properties and implicit structural object contracts, not a replacement entry.
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:129-141
class MapEntryObject {
 public:
  virtual ~MapEntryObject() = default;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:122-128
virtual bool equals(const std::any& other) const = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:122-128
virtual std::int32_t hash_code() const = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:122-128
virtual std::u16string to_string() const = 0;
 protected:
/**
         * Returns the key of this key/value pair.
         */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:133-133
virtual std::any key_dispatch() const = 0;
/**
         * Returns the value of this key/value pair.
         */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:138-138
virtual std::any value_dispatch() const = 0;
template <typename, typename> friend class kotlin::collections::map::Entry;
};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:236-245
class MutableMapEntryObject : public virtual MapEntryObject {
 protected:
/**
         * Changes the value associated with the key of this entry.
         *
         * @return the previous value corresponding to the key.
         */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:243-243
virtual std::any set_value_dispatch(const std::any& value) = 0;
template <typename, typename> friend class kotlin::collections::mutable_map::MutableEntry;
};
// NOTE(port): The source Entry is covariant in both key and value. Actual
// element supertype edges are reused, and each path retains one entry object.
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:129-141
template <typename K, typename V, typename KeyTypes> struct EntryKeyBases;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:129-141
template <typename K, typename V, typename... Types>
struct EntryKeyBases<K, V, std::tuple<Types...>> : public virtual map::Entry<Types, V>... {};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:129-141
template <typename K, typename V, typename ValueTypes> struct EntryValueBases;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:129-141
template <typename K, typename V, typename... Types>
struct EntryValueBases<K, V, std::tuple<Types...>> : public virtual map::Entry<K, Types>... {};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:129-141
template <typename K, typename V, typename KeyTypes> struct EntryKeyPointers;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:129-141
template <typename K, typename V, typename... Types>
struct EntryKeyPointers<K, V, std::tuple<Types...>> { using TypesTuple = std::tuple<map::Entry<Types, V>*...>; };
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:129-141
template <typename K, typename V, typename ValueTypes> struct EntryValuePointers;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:129-141
template <typename K, typename V, typename... Types>
struct EntryValuePointers<K, V, std::tuple<Types...>> { using TypesTuple = std::tuple<map::Entry<K, Types>*...>; };
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:129-141
template <typename K, typename V>
struct ElementSupertypes<map::Entry<K, V>*> {
  using Types = decltype(std::tuple_cat(
      std::declval<typename EntryKeyPointers<K, V, typename ElementSupertypes<K>::Types>::TypesTuple>(),
      std::declval<typename EntryValuePointers<K, V, typename ElementSupertypes<V>::Types>::TypesTuple>(),
      std::declval<std::tuple<std::any>>()));
};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:236-245
template <typename K, typename V>
struct ElementSupertypes<mutable_map::MutableEntry<K, V>*> {
  using Types = std::tuple<map::Entry<K, V>*, std::any>;
};
// NOTE(port): Canonical boxing preserves the exact source entry identity
// across its typed/read-only/mutable views and the Any virtual boundary.
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:129-141
template <typename K, typename V> struct ElementCodec<map::Entry<K, V>*> {
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:129-141
static std::any box(map::Entry<K, V>* value) { return static_cast<MapEntryObject*>(value); }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:129-141
static map::Entry<K, V>* unbox(const std::any& value) {
    return dynamic_cast<map::Entry<K, V>*>(std::any_cast<MapEntryObject*>(value));
  }
};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:236-245
template <typename K, typename V> struct ElementCodec<mutable_map::MutableEntry<K, V>*> {
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:236-245
static std::any box(mutable_map::MutableEntry<K, V>* value) { return static_cast<MapEntryObject*>(value); }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:236-245
static mutable_map::MutableEntry<K, V>* unbox(const std::any& value) {
    return dynamic_cast<mutable_map::MutableEntry<K, V>*>(std::any_cast<MapEntryObject*>(value));
  }
};
// NOTE(port): Kotlin V? keeps null distinct from a value without adding an
// extra nullable layer for already nullable pointer/handle/Any/optional types.
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:86-86
template <typename V> struct NullableMapValue {
  using Result = std::optional<V>;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:86-86
static Result take(const std::any& value) {
    if (!value.has_value()) return std::nullopt;
    return ElementCodec<V>::unbox(value);
  }
};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:86-86
template <typename V> struct NullableMapValue<V*> {
  using Result = V*;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:86-86
static Result take(const std::any& value) {
    return value.has_value() ? ElementCodec<V*>::unbox(value) : nullptr;
  }
};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:86-86
template <typename V> struct NullableMapValue<std::shared_ptr<V>> {
  using Result = std::shared_ptr<V>;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:86-86
static Result take(const std::any& value) {
    return value.has_value() ? ElementCodec<Result>::unbox(value) : nullptr;
  }
};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:86-86
template <typename V> struct NullableMapValue<std::optional<V>> {
  using Result = std::optional<V>;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:86-86
static Result take(const std::any& value) {
    return value.has_value() ? ElementCodec<Result>::unbox(value) : std::nullopt;
  }
};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:86-86
template <> struct NullableMapValue<std::any> {
  using Result = std::any;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:86-86
static Result take(const std::any& value) { return value; }
};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:43-142
class MapObject {
 public:
  virtual ~MapObject() = default;
/**
     * Returns the number of key/value pairs in the map.
     *
     * If a map contains more than [Int.MAX_VALUE] elements, the value of this property is unspecified.
     * For implementations allowing to have more than [Int.MAX_VALUE] elements,
     * it is recommended to explicitly document behavior of this property.
     *
     * @sample samples.collections.Maps.CoreApi.size
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:54-54
virtual std::int32_t get_size() const = 0;
/**
     * Returns `true` if the map is empty (contains no elements), `false` otherwise.
     *
     * @sample samples.collections.Maps.CoreApi.isEmpty
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:61-61
virtual bool is_empty() const = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:23-35
virtual bool equals(const std::any& other) const = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:23-35
virtual std::int32_t hash_code() const = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:23-35
virtual std::u16string to_string() const = 0;
/**
     * Returns `true` if the map contains the specified [key].
     *
     * @sample samples.collections.Maps.CoreApi.containsKey
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:68-68
virtual bool contains_key_dispatch(const std::any& key) const = 0;
/**
     * Returns `true` if the map maps one or more keys to the specified [value].
     *
     * @sample samples.collections.Maps.CoreApi.containsValue
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:75-75
virtual bool contains_value_dispatch(const std::any& value) const = 0;
/**
     * Returns the value corresponding to the given [key], or `null` if such a key is not present in the map.
     *
     * Note that for maps supporting `null` values,
     * the returned `null` value associated with the [key] is indistinguishable from the missing [key],
     * so [containsKey] should be used to check if the map actually contains the [key].
     *
     * @sample samples.collections.Maps.CoreApi.get
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:86-86
virtual std::any get_dispatch(const std::any& key) const = 0;
/**
     * Returns a read-only [Set] of all keys in this map.
     *
     * @sample samples.collections.Maps.CoreApi.keySet
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:94-94
virtual SetObject& keys_dispatch() const = 0;
/**
     * Returns a read-only [Collection] of all values in this map. Note that this collection may contain duplicate values.
     *
     * @sample samples.collections.Maps.CoreApi.valueSet
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:101-101
virtual CollectionObject& values_dispatch() const = 0;
/**
     * Returns a read-only [Set] of all key/value pairs in this map.
     *
     * @sample samples.collections.Maps.CoreApi.entrySet
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:108-108
virtual SetObject& entries_dispatch() const = 0;
};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:160-245
class MutableMapObject : public virtual MapObject {
 public:
/**
     * Removes all elements from this map.
     *
     * @sample samples.collections.Maps.CoreApi.clear
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:199-199
virtual void clear() = 0;
protected:
/**
     * Associates the specified [value] with the specified [key] in the map.
     *
     * If the map doesn't contain a mapping for [key], the mapping is added and the function returns `null`.
     * If the map already contains a mapping for [key], the value for that key is replaced with the specified
     * [value] and the function returns the previous value.
     *
     * @sample samples.collections.Maps.CoreApi.put
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:172-172
virtual std::any put_dispatch(const std::any& key, const std::any& value) = 0;
/**
     * Removes the specified key and its corresponding value from this map.
     *
     * @return the previous value associated with the key, or `null` if the key was not present in the map.
     *
     * @sample samples.collections.Maps.CoreApi.remove
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:182-182
virtual std::any remove_dispatch(const std::any& key) = 0;
/**
     * Updates this map with key/value pairs from the specified map [from].
     *
     * The effect of this call is equivalent to calling [put] for each entry of [from].
     *
     * @sample samples.collections.Maps.CoreApi.putAll
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:192-192
virtual void put_all_dispatch(const MapObject& from) = 0;
template <typename, typename> friend class kotlin::collections::MutableMap;
};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:43-142
template <typename K, typename ValueTypes> struct MapValueBases;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:43-142
template <typename K, typename... Types>
struct MapValueBases<K, std::tuple<Types...>> : public virtual MapObject, public virtual Map<K, Types>... {
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:54-54
  std::int32_t get_size() const override = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:61-61
  bool is_empty() const override = 0;
};
}  // namespace detail
namespace map {
/**
     * Represents a key/value pair held by a [Map].
     *
     * Map entries obtained from the iteration of [Map.entries] set are not supposed to be stored separately or
     * used long after they are obtained.
     * The behavior of an entry is unspecified if the backing map has been modified after the entry was obtained.
     *
     * To create an immutable entry not connected to any map, one can use [Map.Entry.copy] function.
     *
     * [Entry] implementations must override [Any.toString], [Any.equals] and [Any.hashCode] functions
     * and provide implementations such that:
     * - [Entry.toString] should return a string representation of the key-value pair in form of `key=value`.
     * - [Entry.equals] should consider any two instances of [Entry] equal if their keys are equal and values are equal.
     * - [Entry.hashCode] should be computed as exclusive or (XOR) of
     *   hash codes corresponding to a key and a value: `key.hashCode() xor value.hashCode()`
     *
     * @param K the type of the entry key. The entry is covariant in its key type.
     * @param V the type of the entry value. The entry is covariant in its value type.
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:129-141
template <typename K, typename V>
class Entry : public virtual detail::MapEntryObject,
    public detail::EntryKeyBases<K, V, typename detail::ElementSupertypes<K>::Types>,
    public detail::EntryValueBases<K, V, typename detail::ElementSupertypes<V>::Types> {
 public:
/**
         * Returns the key of this key/value pair.
         */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:133-133
K key() const { return detail::ElementCodec<K>::unbox(key_dispatch()); }
/**
         * Returns the value of this key/value pair.
         */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:138-138
V value() const { return detail::ElementCodec<V>::unbox(value_dispatch()); }
};
}  // namespace map
namespace mutable_map {
/**
     * Represents a key/value pair held by a [MutableMap].
     *
     * Map entries obtained from the iteration of [MutableMap.entries] set are not supposed to be stored separately or
     * used long after they are obtained.
     * The behavior of an entry is unspecified if the backing map has been modified after the entry was obtained,
     * except when the map was modified through the [setValue] method.
     *
     * To create an immutable entry not connected to any map, one can use [Map.Entry.copy] function.
     *
     * @param K the type of the entry key. The entry is invariant in its key type.
     * @param V the type of the entry value. The entry is invariant in its value type.
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:236-245
template <typename K, typename V>
class MutableEntry : public virtual map::Entry<K, V>, public virtual detail::MutableMapEntryObject {
 public:
/**
         * Changes the value associated with the key of this entry.
         *
         * @return the previous value corresponding to the key.
         */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:243-243
V set_value(V new_value) {
    return detail::ElementCodec<V>::unbox(set_value_dispatch(detail::ElementCodec<V>::box(std::move(new_value))));
  }
};
}  // namespace mutable_map
/**
 * A collection that holds pairs of objects (keys and values) and supports retrieving the value corresponding to each key,
 * checking if a collection holds a particular key or a value. Maps also allow iterating over keys, values or key-value pairs (entries).
 * Complex operations are built upon this functionality and provided in form of [kotlin.collections] extension functions.
 *
 * Map keys are unique; the map holds only one value for each key. In contrast, the same value can be associated with several unique keys.
 *
 * It is implementation-specific how [Map] defines key's uniqueness. If not stated otherwise, [Map] implementations are usually
 * distinguishing elements using [Any.equals]. However, it is not the only way to distinguish elements, and some implementations may use
 * referential equality or compare elements by some of their properties. It is recommended to explicitly specify how a class
 * implementing [Map] distinguish elements.
 *
 * It is also implementation-specific how [Map] handles `null` keys and values: some [Map] implementations may support them, while
 * other may not. It is recommended to explicitly define key/value nullability policy when implementing [Map].
 *
 * Unlike [Collection] implementations, [Map] implementations must override [Any.toString], [Any.equals] and [Any.hashCode] functions
 * and provide implementations such that:
 * - [Map.toString] should return a string containing string representation of contained key-value pairs in iteration order.
 * - [Map.equals] should consider two maps equal if and only if they contain the same keys and values associated with these keys
 *   are equal. Unlike some other `equals` implementations, [Map.equals] should consider two maps equal even
 *   if they are instances of different classes; the only requirement here is that both maps have to implement [Map] interface.
 * - [Map.hashCode] should be computed as a sum of [Entry] hash codes, and entry's hash code should be computed as exclusive or (XOR) of
 *   hash codes corresponding to a key and a value:
 *   ```kotlin
 *   var hashCode: Int = 0
 *   for ((k, v) in entries) hashCode += k.hashCode() xor v.hashCode()
 *   ```
 *
 * Functions in this interface support only read-only access to the map; read-write access is supported through
 * the [MutableMap] interface.
 *
 * @param K the type of map keys. The map is invariant in its key type, as it
 *          can accept a key as a parameter (of [containsKey] for example) and return it in a [keys] set.
 * @param V the type of map values. The map is covariant in its value type.
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:43-142
template <typename K, typename V>
class Map : public virtual detail::MapObject,
    public detail::MapValueBases<K, typename detail::ElementSupertypes<V>::Types> {
 public:
/**
     * Returns the number of key/value pairs in the map.
     *
     * If a map contains more than [Int.MAX_VALUE] elements, the value of this property is unspecified.
     * For implementations allowing to have more than [Int.MAX_VALUE] elements,
     * it is recommended to explicitly document behavior of this property.
     *
     * @sample samples.collections.Maps.CoreApi.size
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:54-54
std::int32_t get_size() const override = 0;
/**
     * Returns `true` if the map is empty (contains no elements), `false` otherwise.
     *
     * @sample samples.collections.Maps.CoreApi.isEmpty
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:61-61
bool is_empty() const override = 0;
/**
     * Returns `true` if the map contains the specified [key].
     *
     * @sample samples.collections.Maps.CoreApi.containsKey
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:68-68
bool contains_key(K key) const { return contains_key_dispatch(detail::ElementCodec<K>::box(std::move(key))); }
/**
     * Returns `true` if the map maps one or more keys to the specified [value].
     *
     * @sample samples.collections.Maps.CoreApi.containsValue
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:75-75
bool contains_value(V value) const { return contains_value_dispatch(detail::ElementCodec<V>::box(std::move(value))); }
/**
     * Returns the value corresponding to the given [key], or `null` if such a key is not present in the map.
     *
     * Note that for maps supporting `null` values,
     * the returned `null` value associated with the [key] is indistinguishable from the missing [key],
     * so [containsKey] should be used to check if the map actually contains the [key].
     *
     * @sample samples.collections.Maps.CoreApi.get
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:86-86
typename detail::NullableMapValue<V>::Result get(K key) const {
    return detail::NullableMapValue<V>::take(get_dispatch(detail::ElementCodec<K>::box(std::move(key))));
  }
/**
     * Returns a read-only [Set] of all keys in this map.
     *
     * @sample samples.collections.Maps.CoreApi.keySet
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:94-94
Set<K>& keys() const { return dynamic_cast<Set<K>&>(keys_dispatch()); }
/**
     * Returns a read-only [Collection] of all values in this map. Note that this collection may contain duplicate values.
     *
     * @sample samples.collections.Maps.CoreApi.valueSet
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:101-101
Collection<V>& values() const { return dynamic_cast<Collection<V>&>(values_dispatch()); }
/**
     * Returns a read-only [Set] of all key/value pairs in this map.
     *
     * @sample samples.collections.Maps.CoreApi.entrySet
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:108-108
Set<map::Entry<K, V>*>& entries() const { return dynamic_cast<Set<map::Entry<K, V>*>&>(entries_dispatch()); }
};
/**
 * A collection that holds pairs of objects (keys and values) and supports retrieving
 * the value corresponding to each key, as well as adding new, removing or updating existing pairs.
 *
 * Map keys are unique; the map holds only one value for each key. In contrast, the same value can be associated with several unique keys.
 *
 * If a particular use case does not require map's modification, a read-only counterpart, [Map] could be used instead.
 *
 * [MutableMap] extends [Map] contact with functions allowing to add, remove and update mapping between keys and values.
 *
 * Unlike [Map], [keys], [values] and [entries] collections are all mutable, and changes in them update the map.
 *
 * Until stated otherwise, [MutableMap] implementations are not thread-safe and their modification without
 * explicit synchronization may result in data corruption, loss, and runtime errors.
 *
 * @param K the type of map keys. The map is invariant in its key type.
 * @param V the type of map values. The mutable map is invariant in its value type.
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:160-245
template <typename K, typename V>
class MutableMap : public virtual Map<K, V>, public virtual detail::MutableMapObject {
 public:
/**
     * Associates the specified [value] with the specified [key] in the map.
     *
     * If the map doesn't contain a mapping for [key], the mapping is added and the function returns `null`.
     * If the map already contains a mapping for [key], the value for that key is replaced with the specified
     * [value] and the function returns the previous value.
     *
     * @sample samples.collections.Maps.CoreApi.put
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:172-172
typename detail::NullableMapValue<V>::Result put(K key, V value) {
    return detail::NullableMapValue<V>::take(put_dispatch(detail::ElementCodec<K>::box(std::move(key)),
                                                       detail::ElementCodec<V>::box(std::move(value))));
  }
/**
     * Removes the specified key and its corresponding value from this map.
     *
     * @return the previous value associated with the key, or `null` if the key was not present in the map.
     *
     * @sample samples.collections.Maps.CoreApi.remove
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:182-182
typename detail::NullableMapValue<V>::Result remove(K key) {
    return detail::NullableMapValue<V>::take(remove_dispatch(detail::ElementCodec<K>::box(std::move(key))));
  }
/**
     * Updates this map with key/value pairs from the specified map [from].
     *
     * The effect of this call is equivalent to calling [put] for each entry of [from].
     *
     * @sample samples.collections.Maps.CoreApi.putAll
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:192-192
template <typename FromKey> requires std::is_convertible_v<FromKey, K>
  void put_all(const Map<FromKey, V>& from) { put_all_dispatch(from); }
/**
     * Removes all elements from this map.
     *
     * @sample samples.collections.Maps.CoreApi.clear
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:199-199
void clear() override = 0;
/**
     * Returns a [MutableSet] of all keys in this map.
     *
     * @sample samples.collections.Maps.CoreApi.keySetMutable
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:207-207
MutableSet<K>& keys() const { return dynamic_cast<MutableSet<K>&>(this->keys_dispatch()); }
/**
     * Returns a [MutableCollection] of all values in this map. Note that this collection may contain duplicate values.
     *
     * @sample samples.collections.Maps.CoreApi.valueSetMutable
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:214-214
MutableCollection<V>& values() const { return dynamic_cast<MutableCollection<V>&>(this->values_dispatch()); }
/**
     * Returns a [MutableSet] of all key/value pairs in this map.
     *
     * @sample samples.collections.Maps.CoreApi.entrySetMutable
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Map.kt:221-221
MutableSet<mutable_map::MutableEntry<K, V>*>& entries() const { return dynamic_cast<MutableSet<mutable_map::MutableEntry<K, V>*>&>(this->entries_dispatch()); }
};
}  // namespace kotlin::collections
