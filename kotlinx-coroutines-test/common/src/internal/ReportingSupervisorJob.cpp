#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ - kotlinx.coroutines.test.internal.ReportingSupervisorJob
// Original package: kotlinx.coroutines.test.internal
//
// TODO: Import statements removed; fully qualify types or add appropriate includes
// TODO: Kotlin visibility needs C++ equivalent
// TODO: Annotations (@Suppress) preserved as comments
// TODO: Kotlin try-catch-let pattern needs C++ adaptation

#include <functional>
#include <exception>

namespace kotlinx {
namespace coroutines {
namespace test {
namespace {

// package kotlinx.coroutines.test.internal

// import kotlinx.coroutines.*

/**
 * A variant of [SupervisorJob] that additionally notifies about child failures via a callback.
 */
// @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
class ReportingSupervisorJob : JobImpl {
private:
    std::function<void(std::exception_ptr)> on_child_cancellation_;

public:
    ReportingSupervisorJob(
        Job* parent = nullptr,
        std::function<void(std::exception_ptr)> on_child_cancellation = nullptr
    ) : JobImpl(parent), on_child_cancellation_(on_child_cancellation) {}

    bool child_cancelled(std::exception_ptr cause) override {
        try {
            if (on_child_cancellation_) {
                on_child_cancellation_(cause);
            }
        } catch (...) {
            auto current_exception = std::current_exception();
            try {
                std::rethrow_exception(cause);
            } catch (std::exception& e) {
                // TODO: Kotlin addSuppressed needs C++ equivalent (exception chaining)
                // cause.addSuppressed(e)
            }
            /* the coroutine context does not matter here, because we're only interested in reporting this exception
            to the platform-specific global handler, not to a [CoroutineExceptionHandler] of any sort. */
            handle_coroutine_exception(*this, cause);
        }
        return false; // .let { false }
    }
};

} // namespace internal
} // namespace test
} // namespace coroutines
} // namespace kotlinx
