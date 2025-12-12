#pragma once
/**
 * @file CoroutineName.hpp
 * @brief User-specified name of coroutine for debugging
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/CoroutineName.kt
 */

#include "kotlinx/coroutines/CoroutineContext.hpp"
#include <string>

namespace kotlinx {
namespace coroutines {

/**
 * User-specified name of coroutine. This name is used in debugging mode.
 * See new_coroutine_context() for the description of coroutine debugging facilities.
 */
class CoroutineName : public AbstractCoroutineContextElement {
public:
    /**
     * Key for CoroutineName instance in the coroutine context.
     */
    inline static CoroutineContext::KeyTyped<CoroutineName> key_instance;
    static constexpr CoroutineContext::Key* type_key = &key_instance;

    /**
     * User-defined coroutine name.
     */
    std::string name;

    /**
     * Constructor
     */
    explicit CoroutineName(const std::string& name_param)
        : AbstractCoroutineContextElement(type_key), name(name_param) {}

    /**
     * Returns a string representation of the object.
     */
    std::string to_string() const {
        return "CoroutineName(" + name + ")";
    }

    // Data class-like methods
    bool operator==(const CoroutineName& other) const {
        return name == other.name;
    }

    bool operator!=(const CoroutineName& other) const {
        return !(*this == other);
    }

    /**
     * Copy with optional new name (data class copy feature)
     */
    CoroutineName copy(const std::string& new_name) const {
        return CoroutineName(new_name);
    }

    CoroutineName copy() const {
        return CoroutineName(name);
    }
};

} // namespace coroutines
} // namespace kotlinx
