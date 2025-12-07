#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ - kotlinx.coroutines.test.TestDispatchers
// Original package: kotlinx.coroutines.test
//
// TODO: @file:JvmName annotation not applicable in C++
// TODO: Import statements removed; fully qualify types or add appropriate includes
// TODO: Kotlin extension functions on Dispatchers translated to free functions
// TODO: Annotations (@ExperimentalCoroutinesApi) preserved as comments

#include <stdexcept>

namespace kotlinx {
namespace coroutines {
namespace test {

// @file:JvmName("TestDispatchers")

// package kotlinx.coroutines.test

// import kotlinx.coroutines.*
// import kotlinx.coroutines.test.internal.*
// import kotlin.jvm.*

/**
 * Sets the given [dispatcher] as an underlying dispatcher of [Dispatchers.Main].
 * All subsequent usages of [Dispatchers.Main] will use the given [dispatcher] under the hood.
 *
 * Using [TestDispatcher] as an argument has special behavior: subsequently-called [runTest], as well as
 * [TestScope] and test dispatcher constructors, will use the [TestCoroutineScheduler] of the provided dispatcher.
 *
 * It is unsafe to call this method if alive coroutines launched in [Dispatchers.Main] exist.
 */
// @ExperimentalCoroutinesApi
// TODO: Kotlin extension function on Dispatchers; translate to free function or namespace method
void set_main(Dispatchers& dispatchers, CoroutineDispatcher* dispatcher) {
    // TODO: Kotlin type checking with 'is' operator; use dynamic_cast in C++
    if (dynamic_cast<TestMainDispatcher*>(dispatcher) != nullptr) {
        throw std::invalid_argument("Dispatchers.setMain(Dispatchers.Main) is prohibited, probably Dispatchers.resetMain() should be used instead");
    }
    get_test_main_dispatcher(dispatchers).set_dispatcher(dispatcher);
}

/**
 * Resets state of the [Dispatchers.Main] to the original main dispatcher.
 *
 * For example, in Android, the Main thread dispatcher will be set as [Dispatchers.Main].
 * This method undoes a dependency injection performed for tests, and so should be used in tear down (`@After`) methods.
 *
 * It is unsafe to call this method if alive coroutines launched in [Dispatchers.Main] exist.
 */
// @ExperimentalCoroutinesApi
// TODO: Kotlin extension function on Dispatchers; translate to free function or namespace method
void reset_main(Dispatchers& dispatchers) {
    get_test_main_dispatcher(dispatchers).reset_dispatcher();
}

} // namespace test
} // namespace coroutines
} // namespace kotlinx
