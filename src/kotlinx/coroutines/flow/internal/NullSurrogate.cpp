// port-lint: source flow/internal/NullSurrogate.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/NullSurrogate.kt */
#include "kotlinx/coroutines/flow/internal/NullSurrogate.hpp"

namespace kotlinx::coroutines::flow::internal {
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/NullSurrogate.kt:12-12
kotlinx::coroutines::internal::Symbol& NULL_VALUE() {
    static kotlinx::coroutines::internal::Symbol value("NULL");
    return value;
}
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/NullSurrogate.kt:19-19
kotlinx::coroutines::internal::Symbol& UNINITIALIZED() {
    static kotlinx::coroutines::internal::Symbol value("UNINITIALIZED");
    return value;
}
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/NullSurrogate.kt:26-26
kotlinx::coroutines::internal::Symbol& DONE() {
    static kotlinx::coroutines::internal::Symbol value("DONE");
    return value;
}
} // namespace kotlinx::coroutines::flow::internal
