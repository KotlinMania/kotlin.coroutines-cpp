// port-lint: source kotlinx-coroutines-core/native/src/Exceptions.kt
// port-lint: source libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt
/**
 * Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt
 */
#include "kotlinx/coroutines/native/Exceptions.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include <utility>
#include <bit>
#include <iterator>
#include "../../../../third_party/utfcpp/utf8/with_replacement.h"

namespace kotlin::coroutines::cancellation {

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

} // namespace kotlin::coroutines::cancellation

namespace kotlinx::coroutines {

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

} // namespace kotlinx::coroutines

namespace kotlin::coroutines::cancellation {

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:31-31
bool CancellationException::equals(const std::exception* other) const {
    return this == other;
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:41-41
// Transliterated from: kotlin-native/runtime/src/main/cpp/Natives.cpp:40-49
std::int32_t CancellationException::hash_code() const {
    // NOTE(port): Inline the actual Native identity primitive for ordinary C++ object storage.
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(this)));
}

} // namespace kotlin::coroutines::cancellation

namespace kotlinx::coroutines {

namespace {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/String.kt:19-21
// Transliterated from: kotlin-native/runtime/src/main/cpp/KString.cpp:125-145,543-564
// Transliterated from: kotlin-native/runtime/src/main/cpp/polyhash/naive.h:11-17
// NOTE(port): Existing C++ messages use UTF-8. Use Native's actual UTF-8 conversion dependency,
// then hash UTF-16 code units. Native object-header caching is unnecessary for C++ value storage.
std::uint32_t string_hash_code(const std::string& message) {
    std::u16string units;
    utf8::with_replacement::utf8to16(message.begin(), message.end(), std::back_inserter(units));
    std::uint32_t result = 0;
    auto current = units.begin();
    while (current != units.end()) result = result * 31 + static_cast<std::uint16_t>(*current++);
    return result;
}

// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:31-31
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:41-41
// Transliterated from: kotlin-native/runtime/src/main/cpp/Natives.cpp:40-49
// NOTE(port): The actual std::exception carrier stays borrowed. CancellationException dispatches
// its virtual source hash; other exception carriers inherit Throwable's identity hash.
std::uint32_t exception_hash_code(std::exception_ptr cause) {
    if (!cause) return 0;
    try { std::rethrow_exception(cause); }
    catch (const CancellationException& exception) { return static_cast<std::uint32_t>(exception.hash_code()); }
    catch (const std::exception& exception) {
        return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&exception));
    }
}

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

// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:30-31
std::int32_t JobCancellationException::hash_code() const {
    // NOTE(port): Kotlin Int arithmetic wraps; unsigned intermediates retain its low 32 bits.
    const auto message_hash = string_hash_code(get_message().value());
    const auto job_hash = static_cast<std::uint32_t>(job_->hash_code());
    const auto cause_hash = exception_hash_code(get_cause());
    return std::bit_cast<std::int32_t>((message_hash * 31 + job_hash) * 31 + cause_hash);
}

// For use in tests.
// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:34-35
const bool RECOVER_STACK_TRACES = false;

} // namespace kotlinx::coroutines
