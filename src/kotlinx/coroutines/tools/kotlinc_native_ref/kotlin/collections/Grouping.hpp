// port-lint: source libraries/stdlib/src/kotlin/collections/Grouping.kt
// Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:11-32,77-88,139-145,186-192
#pragma once
#include "Map.hpp"
#include <bit>
#include <functional>
#include <type_traits>

namespace kotlin::collections {
template <typename T, typename K> class Grouping;
namespace detail {
// NOTE(port): Use the collection interfaces' private boxing convention so
// Grouping's key out-type has one implementation across its genuine supertypes.
// Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:27-32
class GroupingObject {
public:
    virtual ~GroupingObject() = default;
protected:
    // Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:29-29
    virtual std::unique_ptr<IteratorObject> source_iterator_dispatch() const = 0;
    // Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:31-31
    virtual std::any key_of_dispatch(const std::any& element) const = 0;
    template <typename, typename> friend class ::kotlin::collections::Grouping;
};
// Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:27-27
template <typename T, typename KeyTypes> struct GroupingKeyBases;
// Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:27-27
template <typename T, typename... Keys>
struct GroupingKeyBases<T, std::tuple<Keys...>> : public virtual Grouping<T, Keys>... {};

// NOTE(port): Check the source MutableMap<in K,R> bound through K's real
// collection-codec supertypes. The destination's invariant value type stays R.
// Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:77-80
template <typename M, typename R, typename Keys> struct GroupingDestination;
// Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:77-80
template <typename M, typename R>
struct GroupingDestination<M, R, std::tuple<>> : std::false_type {};
// Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:77-84
template <typename M, typename R, typename Key, typename... Keys>
struct GroupingDestination<M, R, std::tuple<Key, Keys...>>
    : std::bool_constant<std::is_base_of_v<MutableMap<Key, R>, M> ||
        GroupingDestination<M, R, std::tuple<Keys...>>::value> {
    // NOTE(port): Use the map's public typed put; decode the projected key
    // through its existing codec without exposing protected mutation dispatch.
    // Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:84-84
    static void put(M& destination, const std::any& key, R value) {
        if constexpr (std::is_base_of_v<MutableMap<Key, R>, M>)
            (void)static_cast<MutableMap<Key, R>&>(destination).put(
                ElementCodec<Key>::unbox(key), std::move(value));
        else
            GroupingDestination<M, R, std::tuple<Keys...>>::put(destination, key, std::move(value));
    }
};
// NOTE(port): NullableMapValue avoids an extra nullable layer for nullable R.
// Non-null R unwraps the optional; null cannot be cast to a non-null value.
// Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:144-145,191-192
template <typename R>
R grouping_accumulator(const typename NullableMapValue<R>::Result& accumulator) {
    if constexpr (std::is_same_v<R, typename NullableMapValue<R>::Result>) {
        return accumulator;
    } else {
        if (!accumulator) throw std::bad_any_cast();
        return *accumulator;
    }
}
}

/** A source of elements and the key selector used by group-and-fold operations. */
// Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:11-32
template <typename T, typename K>
class Grouping : public virtual detail::GroupingObject,
    public detail::GroupingKeyBases<T, typename detail::ElementSupertypes<K>::Types> {
public:
    /** Returns an iterator over the source elements. */
    // Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:28-29
    std::unique_ptr<Iterator<T>> source_iterator() const {
        auto object = this->source_iterator_dispatch();
        auto& typed = dynamic_cast<Iterator<T>&>(*object);
        object.release();
        return std::unique_ptr<Iterator<T>>(&typed);
    }
    /** Extracts the key of an element. */
    // Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:30-31
    K key_of(T element) const {
        return detail::ElementCodec<K>::unbox(
            this->key_of_dispatch(detail::ElementCodec<T>::box(std::move(element))));
    }
};

/** Aggregate each group into the supplied map, preserving its existing values. */
// Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:55-88
template <typename T, typename K, typename R, typename M>
M& aggregate_to(const Grouping<T, K>& grouping, M& destination,
    const std::function<R(K, typename detail::NullableMapValue<R>::Result, T, bool)>& operation) {
    using KeyTypes = decltype(std::tuple_cat(std::declval<std::tuple<K>>(),
        std::declval<typename detail::ElementSupertypes<K>::Types>()));
    static_assert(detail::GroupingDestination<M, R, KeyTypes>::value);
    auto iterator = grouping.source_iterator();
    while (iterator->has_next()) {
        auto element = iterator->next();
        auto key = grouping.key_of(element);
        auto boxed_key = detail::ElementCodec<K>::box(key);
        auto accumulator = detail::NullableMapValue<R>::take(destination.get_dispatch(boxed_key));
        bool is_null;
        if constexpr (std::is_same_v<decltype(accumulator), std::any>)
            is_null = !accumulator.has_value();
        else
            is_null = !accumulator;
        const bool first = is_null && !destination.contains_key_dispatch(boxed_key);
        auto value = operation(key, accumulator, element, first);
        detail::GroupingDestination<M, R, KeyTypes>::put(destination, boxed_key, std::move(value));
    }
    return destination;
}

/** Fold each group into destination, selecting an initial value for new keys. */
// Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:116-145
template <typename T, typename K, typename R, typename M>
M& fold_to(const Grouping<T, K>& grouping, M& destination,
    const std::function<R(K, T)>& initial_value_selector,
    const std::function<R(K, R, T)>& operation) {
    return aggregate_to<T, K, R>(grouping, destination,
        [&](K key, typename detail::NullableMapValue<R>::Result accumulator, T element, bool first) {
            return operation(key, first ? initial_value_selector(key, element)
                : detail::grouping_accumulator<R>(accumulator), element);
        });
}

/** Fold each group into destination, using the same initial value for new keys. */
// Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:168-192
template <typename T, typename K, typename R, typename M>
M& fold_to(const Grouping<T, K>& grouping, M& destination, R initial_value,
    const std::function<R(R, T)>& operation) {
    return aggregate_to<T, K, R>(grouping, destination,
        [&](K, typename detail::NullableMapValue<R>::Result accumulator, T element, bool first) {
            return operation(first ? initial_value : detail::grouping_accumulator<R>(accumulator), element);
        });
}

/** Reduce each group into destination, using its first element for a new key. */
// Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:221-246
template <typename S, typename T, typename K, typename M>
M& reduce_to(const Grouping<T, K>& grouping, M& destination,
    const std::function<S(K, S, T)>& operation) {
    static_assert(std::is_convertible_v<T, S>);
    return aggregate_to<T, K, S>(grouping, destination,
        [&](K key, typename detail::NullableMapValue<S>::Result accumulator, T element, bool first) {
            return first ? static_cast<S>(element)
                : operation(key, detail::grouping_accumulator<S>(accumulator), element);
        });
}

/** Count each group into destination, adding to any existing counter. */
// Transliterated from: libraries/stdlib/src/kotlin/collections/Grouping.kt:249-262
template <typename T, typename K, typename M>
M& each_count_to(const Grouping<T, K>& grouping, M& destination) {
    return fold_to<T, K, std::int32_t>(grouping, destination, 0,
        [](std::int32_t accumulator, T) {
            // NOTE(port): Kotlin Int addition wraps; avoid C++ signed overflow.
            return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(accumulator) + 1U);
        });
}
}
