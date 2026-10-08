// port-lint: source libraries/stdlib/src/kotlin/collections/Sequence.kt
// Transliterated from: libraries/stdlib/src/kotlin/collections/Sequence.kt:8-28
#pragma once
#include "../collections/Iterator.hpp"
#include <memory>

namespace kotlin::sequences {
template <typename T> class Sequence;
namespace detail {
// NOTE(port): Match the existing Iterable/Iterator covariance boundary. The
// factory transfers a new iterator; it does not materialize the sequence.
// Transliterated from: libraries/stdlib/src/kotlin/collections/Sequence.kt:21-28
class SequenceObject {
public:
    virtual ~SequenceObject() = default;
protected:
    SequenceObject() = default;
    SequenceObject(const SequenceObject&) = delete;
    SequenceObject& operator=(const SequenceObject&) = delete;
    // Transliterated from: libraries/stdlib/src/kotlin/collections/Sequence.kt:27-27
    virtual std::unique_ptr<::kotlin::collections::detail::IteratorObject>
        iterator_dispatch() const = 0;
    template <typename> friend class ::kotlin::sequences::Sequence;
};
}
/**
 * A sequence that returns values through its iterator. Values are evaluated
 * lazily, and the sequence is potentially infinite.
 *
 * Sequences can be iterated multiple times, but some implementations constrain
 * themselves to one iteration, as documented by those implementations. Such a
 * sequence throws on an attempt to iterate it a second time.
 *
 * Sequence operations generally preserve that property; exceptions to it are
 * documented by the operation.
 * @param T the type of elements in the sequence.
 */
// Transliterated from: libraries/stdlib/src/kotlin/collections/Sequence.kt:8-28
template <typename T>
class Sequence : public virtual detail::SequenceObject,
    public ::kotlin::collections::detail::CovariantBases<Sequence,
        typename ::kotlin::collections::detail::ElementSupertypes<T>::Types> {
public:
    /**
     * Returns an iterator over the values of the sequence.
     * Throws if a constrained-once sequence is iterated a second time.
     */
    // Transliterated from: libraries/stdlib/src/kotlin/collections/Sequence.kt:22-27
    std::unique_ptr<::kotlin::collections::Iterator<T>> iterator() const {
        auto object = this->iterator_dispatch();
        auto& typed = dynamic_cast<::kotlin::collections::Iterator<T>&>(*object);
        object.release();
        return std::unique_ptr<::kotlin::collections::Iterator<T>>(&typed);
    }
};
}
