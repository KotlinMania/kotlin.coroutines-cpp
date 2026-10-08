// port-lint: source kotlinx-coroutines-core/common/src/flow/Channels.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt
 */
#include "kotlinx/coroutines/flow/Channels.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include <utility>

namespace kotlinx::coroutines::flow::internal {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:104-108
// NOTE(port): The generic receiver binds its concrete Boolean state here.
void mark_channel_consumed(bool consume, std::atomic<bool>& consumed) {
    if (consume) {
        if (consumed.exchange(true)) {
            throw IllegalStateException(
                "ReceiveChannel.consumeAsFlow can be collected just once");
        }
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
// NOTE(port): Typed bindings retain the source iterator and element. The Clang
// frontend lowers this loop and its locals through the existing Continuation ABI.
[[clang::annotate("suspend")]]
void* emit_all_erased(
    std::function<void()> make_iterator,
    std::function<void*(Continuation<void*>*)> has_next,
    std::function<void*(Continuation<void*>*)> emit_next,
    std::function<void()> finish_emit,
    std::function<void(std::exception_ptr)> cancel_consumed,
    bool consume, std::shared_ptr<Continuation<void*>> completion) {
    std::exception_ptr cause;
    try {
        make_iterator();
        while (true) {
            // NOTE(port): The iterator transfers an owning bool result box on
            // both immediate and resumed returns; this call site releases it.
            auto has_next_box = std::unique_ptr<bool>(static_cast<bool*>(
                dsl::suspend(has_next(completion.get()))));
            if (!*has_next_box) break;
            dsl::suspend(emit_next(completion.get()));
            finish_emit();
        }
    } catch (...) {
        cause = std::current_exception();
        if (consume) cancel_consumed(cause);
        throw;
    }
    if (consume) cancel_consumed(cause);
    return nullptr;
}

} // namespace kotlinx::coroutines::flow::internal
