// port-lint: source kotlinx-coroutines-core/native/src/Exceptions.kt
/**
 * Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt
 */
#include "kotlinx/coroutines/native/Exceptions.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include <utility>

namespace kotlinx::coroutines {

// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:12-12
CancellationException::CancellationException() : CancellationException(std::nullopt, nullptr) {}

// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:13-13
CancellationException::CancellationException(std::optional<std::string> message)
    : CancellationException(std::move(message), nullptr) {}

// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:13-13
CancellationException::CancellationException(const std::string& message)
    : CancellationException(std::optional<std::string>(message), nullptr) {}

// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:13-13
CancellationException::CancellationException(const char* message)
    : CancellationException(message ? std::optional<std::string>(message) : std::nullopt, nullptr) {}

// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:14-14
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Throwable.kt:24-30
CancellationException::CancellationException(std::optional<std::string> message, std::exception_ptr cause)
    // NOTE(port): what() is a non-null C++ text view; get_message() retains Native nullability.
    : IllegalStateException(message.value_or("")), message_(std::move(message)), cause_(cause) {}

// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:14-14
CancellationException::CancellationException(const std::string& message, std::exception_ptr cause)
    : CancellationException(std::optional<std::string>(message), cause) {}

// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:14-14
CancellationException::CancellationException(const char* message, std::exception_ptr cause)
    : CancellationException(message ? std::optional<std::string>(message) : std::nullopt, cause) {}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Throwable.kt:26-26
const std::optional<std::string>& CancellationException::get_message() const { return message_; }

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Throwable.kt:27-27
std::exception_ptr CancellationException::get_cause() const { return cause_; }

// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:13-14
// NOTE(port): This owning factory result must be deleted by its C++ caller.
CancellationException* cancellation_exception(const std::string& message, std::exception_ptr cause) {
    return new CancellationException(message, cause);
}

// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:13-14
CancellationException* cancellation_exception(std::optional<std::string> message, std::exception_ptr cause) {
    return new CancellationException(std::move(message), cause);
}

// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:13-14
CancellationException* cancellation_exception(const char* message, std::exception_ptr cause) {
    return new CancellationException(message, cause);
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:31-31
bool CancellationException::equals(const std::exception* other) const {
    return this == other;
}

namespace {
// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:29-29
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:31-31
// NOTE(port): exception_ptr preserves the actual thrown object identity. Open
// CancellationException equality dispatches on that object after rethrow; other
// C++ exception carriers keep the source Throwable's default identity equality.
bool exception_equals(std::exception_ptr left, std::exception_ptr right) {
    if (!left || !right) return left == right;
    try {
        std::rethrow_exception(right);
    } catch (const std::exception& other) {
        try {
            std::rethrow_exception(left);
        } catch (const CancellationException& receiver) {
            return receiver.equals(&other);
        } catch (...) {
            return left == right;
        }
    } catch (...) {
        return left == right;
    }
}
} // namespace

// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:21-25
JobCancellationException::JobCancellationException(
    const std::string& message, std::exception_ptr cause, Job* job)
    : CancellationException(message, cause), job_(job) {}

// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:24-24
Job* JobCancellationException::get_job() const { return job_; }

// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:27-29
bool JobCancellationException::equals(const std::exception* other) const {
    if (other == this) return true;
    auto* cancellation = dynamic_cast<const JobCancellationException*>(other);
    return cancellation && cancellation->get_message() == get_message() &&
        (cancellation->job_ ? cancellation->job_->equals(job_) : job_ == nullptr) &&
        exception_equals(cancellation->get_cause(), get_cause());
}

// For use in tests.
// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:34-35
const bool RECOVER_STACK_TRACES = false;

} // namespace kotlinx::coroutines
