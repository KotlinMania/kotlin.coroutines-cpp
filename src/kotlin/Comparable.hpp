/**
 * Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Comparable.kt
 */
#ifndef KOTLIN_COMPARABLE_HPP_
#define KOTLIN_COMPARABLE_HPP_
namespace kotlin {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Comparable.kt:11-21
// Classes which inherit from this interface define a total ordering.
template <typename T>
class Comparable {
public:
    // NOTE(port): Virtual destruction preserves ordinary C++ derived cleanup.
    virtual ~Comparable() = default;
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Comparable.kt:14-20
    // Zero means equal, negative means less, positive means greater.
    // NOTE(port): The source argument is borrowed; C++ does not copy its object.
    virtual int compare_to(const T& other) const = 0;
};
} // namespace kotlin
#endif
