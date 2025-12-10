#pragma once

namespace kotlinx {
namespace coroutines {

/**
 * Defines start options for coroutines builders.
 */
enum class CoroutineStart {
    DEFAULT,
    LAZY,
    ATOMIC,
    UNDISPATCHED
};

inline bool is_lazy(CoroutineStart start) {
    return start == CoroutineStart::LAZY;
}

// Function helper to replace method call on enum
template <typename Block, typename Receiver, typename Completion>
void invoke(CoroutineStart start, Block&& block, Receiver&& receiver, Completion&& completion) {
    // TODO: Implement dispatch logic based on start mode
    if (start == CoroutineStart::DEFAULT) {
         // Default dispatch
    } else if (start == CoroutineStart::UNDISPATCHED) { 
         // Undispatched
    }
    // Stub implementation to allow compilation of call sites
}

} // namespace coroutines
} // namespace kotlinx
