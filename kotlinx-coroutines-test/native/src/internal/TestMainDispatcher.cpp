#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ - kotlinx.coroutines.test.internal.TestMainDispatcher (Native)
// Original package: kotlinx.coroutines.test.internal
// Platform: Native
//
// TODO: Import statements removed; fully qualify types or add appropriate includes
// TODO: actual/expect mechanism - this is the native-specific implementation
// TODO: Annotations (@Suppress) preserved as comments
// TODO: Kotlin visibility needs C++ equivalent

namespace kotlinx {
namespace coroutines {
namespace test {
namespace {

// package kotlinx.coroutines.test.internal
// import kotlinx.coroutines.*

/**
 * Native implementation of getTestMainDispatcher.
 * Returns the existing TestMainDispatcher or creates a new one.
 */
// @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE") // do not remove the INVISIBLE_REFERENCE suppression: required in K2
// TODO: actual function - platform-specific implementation
TestMainDispatcher& get_test_main_dispatcher(Dispatchers& dispatchers) {
    auto* main_dispatcher = dispatchers.main();

    // Check if it's already a TestMainDispatcher
    if (auto* test_main = dynamic_cast<TestMainDispatcher*>(main_dispatcher)) {
        return *test_main;
    }

    // Create a new TestMainDispatcher and inject it
    auto* new_test_main = new TestMainDispatcher(main_dispatcher);
    dispatchers.inject_main(new_test_main);
    return *new_test_main;
}

} // namespace internal
} // namespace test
} // namespace coroutines
} // namespace kotlinx
