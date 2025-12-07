#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/internal/CoroutineExceptionHandlerImpl.kt
//
// TODO: actual keyword - platform-specific implementation
// TODO: kotlin.native.* APIs
// TODO: @OptIn annotation

namespace kotlinx {
namespace coroutines {
namespace {

// TODO: Remove imports, fully qualify or add includes:
// import kotlinx.coroutines.*
// import kotlin.coroutines.*
// import kotlin.native.*

namespace {
    // TODO: auto     SynchronizedObject lock;
}

// TODO: actual :vector<CoroutineExceptionHandler*> std get_platform_exception_handlers() {
    // TODO: synchronized(lock) { platformExceptionHandlers_ }
    return platform_exception_handlers_;
}

namespace {
    // TODO: :vector<CoroutineExceptionHandler*> std platform_exception_handlers_;
}

// TODO: actual function
void ensure_platform_exception_handler_loaded(CoroutineExceptionHandler* callback) {
    // TODO: synchronized(lock) {
    //     platformExceptionHandlers_ += callback
    // }
    platform_exception_handlers_.push_back(callback);
}

// TODO: @OptIn(ExperimentalStdlibApi::class)
// TODO: actual function
void propagate_exception_final_resort(std::exception* exception) {
    // log exception
    process_unhandled_exception(exception);
}

// TODO: actual class
class DiagnosticCoroutineContextException : std::runtime_error {
public:
    DiagnosticCoroutineContextException(CoroutineContext context)
        : std::runtime_error(context.to_string())
    {
    }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
