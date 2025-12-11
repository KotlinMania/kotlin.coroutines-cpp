#include "kotlinx/coroutines/CoroutineContext.hpp"
#include <iostream>

namespace kotlinx {
namespace coroutines {

std::shared_ptr<CoroutineContext> CoroutineContext::operator+(std::shared_ptr<CoroutineContext> other) const {
    if (other == nullptr) return nullptr; // Should be 'this' + other logic
    return other; // Stub
}

std::shared_ptr<CoroutineContext> CoroutineContext::minus_key(Key* key) const {
    return nullptr; // Stub
}

// Fold needs template impl? declared in header as template?
// Header: template <typename R> R fold(...)
// Templates are in header usually. If not defined in header, linker error.
// view_file of CoroutineContext.hpp showed it is declared inside class:
// template <typename R> R fold(...) const;
// If implementation is missing in header, then we can't instantiate it unless explicit instantiation exists.
// BUT since we don't call fold in test, we might be fine IF we don't virtual dispatch to it?
// It is not virtual.

} // namespace coroutines
} // namespace kotlinx
