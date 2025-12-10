#pragma once

#include <memory>

namespace kotlinx {
namespace coroutines {

// Forward Declarations
struct CoroutineContext;
struct CoroutineDispatcher;
struct CoroutineScope;
struct Job;
struct JobSupport;
struct ChildJob;
struct ParentJob;
template <typename T> class AbstractCoroutine;

namespace internal {
    class LockFreeLinkedListNode;
    class LockFreeLinkedListHead;
}

// Common aliases if needed (or move to types.hpp)

} // namespace coroutines
} // namespace kotlinx
