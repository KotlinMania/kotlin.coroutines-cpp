#pragma once
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"

namespace kotlinx {
namespace coroutines {

class MainCoroutineDispatcher; // Forward declaration

/**
 * Groups various implementations of [CoroutineDispatcher].
 */
class Dispatchers {
public:
    /**
     * The default [CoroutineDispatcher] that is used by all standard builders.
     * It is backed by a shared pool of threads.
     */
    static CoroutineDispatcher& get_default();

    /**
     * A coroutine dispatcher that is confined to the Main thread operating with UI objects.
     */
    static MainCoroutineDispatcher& get_main();

    /**
     * A coroutine dispatcher that is not confined to any specific thread.
     */
    static CoroutineDispatcher& get_unconfined();

    /**
     * The [CoroutineDispatcher] that is designed for offloading blocking IO tasks to a shared pool of threads.
     */
    static CoroutineDispatcher& get_io();

    /**
     * Shuts down built-in dispatchers, such as Default and IO, and prevents creating new ones.
     * This is a **delicate** API. It is not supposed to be called from a general application-level
     * code and its invocation is irreversible. All Kotlin coroutines APIs that use dispatchers will
     * stop working after this function is invoked.
     */
    static void shutdown();

private:
    Dispatchers() = delete; // Prevent instantiation
};

} // namespace coroutines
} // namespace kotlinx
