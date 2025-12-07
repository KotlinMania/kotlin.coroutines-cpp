// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/CoroutineName.kt
//
// TODO: data class maps to struct with equality operators
// TODO: AbstractCoroutineContextElement is a base class

#include "kotlinx/coroutines/core_fwd.hpp"
#include <string>

namespace kotlinx {
namespace coroutines {

// TODO: import kotlin.coroutines.AbstractCoroutineContextElement
// TODO: import kotlin.coroutines.CoroutineContext

/**
 * User-specified name of coroutine. This name is used in debugging mode.
 * See [newCoroutineContext][CoroutineScope.newCoroutineContext] for the description of coroutine debugging facilities.
 */
// TODO: data class - generate equality operators, copy constructor, etc.
class CoroutineName : public AbstractCoroutineContextElement {
public:
    /**
     * User-defined coroutine name.
     */
    std::string name;

    /**
     * Key for [CoroutineName] instance in the coroutine context.
     */
    // TODO: companion object Key - static nested class
    class Key : public CoroutineContext::Key<CoroutineName> {
    public:
        static Key instance;
    };

    // Constructor
    CoroutineName(const std::string& name_param)
        : AbstractCoroutineContextElement(Key::instance), name(name_param) {}

    /**
     * Returns a string representation of the object.
     */
    std::string to_string() const override {
        return "CoroutineName(" + name + ")";
    }

    // TODO: data class auto-generated methods
    // Equality operator
    bool operator==(const CoroutineName& other) const {
        return name == other.name;
    }

    bool operator!=(const CoroutineName& other) const {
        return !(*this == other);
    }

    // Copy method (data class feature)
    CoroutineName copy(const std::string& name_param = name) const {
        return CoroutineName(name_param);
    }
};

// Static member initialization
CoroutineName::Key CoroutineName::Key::instance;

} // namespace coroutines
} // namespace kotlinx
