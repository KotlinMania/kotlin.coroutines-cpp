#pragma once
// NOTE(port): C++ projection of Kotlin library imports of kotlin.coroutines.*.
// The standard-library types are defined only in their actual Kotlin namespace.
#include <memory>
#include <optional>
#include <string>
namespace kotlin::coroutines {
class CoroutineContext;
class AbstractCoroutineContextElement;
class EmptyCoroutineContext;
struct ContinuationInterceptor;
template <typename T> class Continuation;
class ContinuationBase;
class ContinuationVoidAdapter;
template <typename T> class FunctionalContinuation;
template <typename B, typename E> class AbstractCoroutineContextKey;
namespace internal {
class AbstractCoroutineContextKeyBase;
template <typename T> class ResultBoxCompletion;
template <typename T>
std::shared_ptr<Continuation<T>> result_box_completion(std::shared_ptr<Continuation<void*>> completion);
}
}
namespace kotlin::coroutines::native::internal {
class BaseContinuationImpl;
class ContinuationImpl;
class RestrictedContinuationImpl;
class CompletedContinuation;
}
namespace kotlin::coroutines::intrinsics {
using kotlin::coroutines::native::internal::BaseContinuationImpl;
using kotlin::coroutines::native::internal::ContinuationImpl;
using kotlin::coroutines::native::internal::RestrictedContinuationImpl;
using kotlin::coroutines::native::internal::CompletedContinuation;
}
namespace kotlinx::coroutines {
using kotlin::coroutines::CoroutineContext;
using kotlin::coroutines::AbstractCoroutineContextElement;
using kotlin::coroutines::EmptyCoroutineContext;
using kotlin::coroutines::ContinuationInterceptor;
using kotlin::coroutines::Continuation;
using kotlin::coroutines::ContinuationBase;
using kotlin::coroutines::ContinuationVoidAdapter;
using kotlin::coroutines::FunctionalContinuation;
using kotlin::coroutines::AbstractCoroutineContextKey;
using kotlin::coroutines::native::internal::BaseContinuationImpl;
using kotlin::coroutines::native::internal::ContinuationImpl;
using kotlin::coroutines::native::internal::RestrictedContinuationImpl;
using kotlin::coroutines::native::internal::CompletedContinuation;
namespace intrinsics { using namespace kotlin::coroutines::intrinsics; }
namespace internal {
using kotlin::coroutines::internal::AbstractCoroutineContextKeyBase;
using kotlin::coroutines::internal::ResultBoxCompletion;
using kotlin::coroutines::internal::result_box_completion;
}
// No debugging facilities on Native.
// Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:46-46
std::optional<std::string> coroutine_name(const std::shared_ptr<kotlin::coroutines::CoroutineContext>& context);
}
