// port-lint: source internal/OnUndeliveredElement.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/OnUndeliveredElement.kt
 */
#include "kotlinx/coroutines/internal/OnUndeliveredElement.hpp"

namespace kotlinx::coroutines::internal {

// Transliterated from: kotlinx-coroutines-core/common/src/internal/OnUndeliveredElement.kt:36-36
UndeliveredElementException::UndeliveredElementException(const std::string& message, std::exception_ptr cause)
    : std::runtime_error(message), cause_(std::move(cause)) {}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Throwable.kt:24-28
std::exception_ptr UndeliveredElementException::cause() const noexcept { return cause_; }

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Throwable.kt:189-199
void UndeliveredElementException::add_suppressed(std::exception_ptr exception) {
    if (!exception) throw std::invalid_argument("null suppressed exception");
    try {
        std::rethrow_exception(exception);
    } catch (const UndeliveredElementException& other) {
        if (&other == this) return;
    } catch (...) {}
    suppressed_exceptions_.push_back(std::move(exception));
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Throwable.kt:206-208
const std::vector<std::exception_ptr>& UndeliveredElementException::suppressed_exceptions() const noexcept {
    return suppressed_exceptions_;
}

} // namespace kotlinx::coroutines::internal
