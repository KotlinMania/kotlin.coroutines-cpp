#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/NamedDispatcher.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: CoroutineDispatcher, CoroutineContext, Delay need C++ equivalents
// TODO: @InternalCoroutinesApi annotation - translate to comment
// TODO: Runnable struct needs C++ equivalent

#include <string>

namespace kotlinx {
namespace coroutines {
namespace {

// Forward declarations
class CoroutineDispatcher;
class CoroutineContext;
class Delay;
class DefaultDelay;
class Runnable;

/**
 * Wrapping dispatcher that has a nice user-supplied `tostd::string()` representation
 */
class NamedDispatcher : CoroutineDispatcher /* , Delay */ {
private:
    CoroutineDispatcher* dispatcher_;
    std::string name_;

public:
    NamedDispatcher(CoroutineDispatcher* dispatcher, const std::string& name)
        : dispatcher_(dispatcher), name_(name) {}

    bool is_dispatch_needed(CoroutineContext* context) override {
        return dispatcher_->is_dispatch_needed(context);
    }

    void dispatch(CoroutineContext* context, Runnable* block) override {
        dispatcher_->dispatch(context, block);
    }

    // TODO: @InternalCoroutinesApi annotation
    void dispatch_yield(CoroutineContext* context, Runnable* block) override {
        dispatcher_->dispatch_yield(context, block);
    }

    std::string to_string() override {
        return name_;
    }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
