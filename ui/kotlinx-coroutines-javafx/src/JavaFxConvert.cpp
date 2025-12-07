#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin: kotlinx.coroutines.javafx.JavaFxConvert
// Original package: kotlinx.coroutines.javafx
//
// TODO: This is a mechanical C++ transliteration. The following constructs need proper implementation:
// - JavaFX API calls (ObservableValue, ChangeListener)
// - Kotlin extension functions (ObservableValue<T>.asFlow()) -> free function
// - Kotlin Flow and Channel APIs (Flow, callbackFlow, flowOn, conflate, buffer, produceIn)
// - Kotlin suspend functions and coroutine builders (callbackFlow, send, awaitClose)
// - @ExperimentalCoroutinesApi annotation
// - Kotlin lambda syntax and trailing lambda
// - Dispatchers.JavaFx extension property
// - Kotlin generic type parameters

#include <memory>
#include <functional>

// TODO: Remove placeholder imports; map to actual JavaFX/coroutine headers
// #include <javafx/beans/value/ObservableValue.h>
// #include <javafx/beans/value/ChangeListener.h>
// #include <kotlinx/coroutines/flow/Flow.h>
// #include <kotlinx/coroutines/channels/callbackFlow.h>
// #include <kotlinx/coroutines/Dispatchers.h>

namespace kotlinx {
namespace coroutines {
namespace javafx {

// TODO: Forward declarations for unmapped types
template<typename T>
class ObservableValue;

template<typename T>
class ChangeListener;

template<typename T>
class Flow;

class Dispatchers;

/**
 * Creates an instance of a cold [Flow] that subscribes to the given [ObservableValue] and emits
 * its values as they change. The resulting flow is conflated, meaning that if several values arrive in quick
 * succession, only the last one will be emitted.
 * Since this implementation uses [ObservableValue.addListener], even if this [ObservableValue]
 * supports lazy evaluation, eager computation will be enforced while the flow is being collected.
 * All the calls to JavaFX API are performed in [Dispatchers.JavaFx].
 * This flow emits at least the initial value.
 *
 * ### Operator fusion
 *
 * Adjacent applications of [flowOn], [buffer], [conflate], and [produceIn] to the result of `asFlow` are fused.
 * [conflate] has no effect, as this flow is already conflated; one can use [buffer] to change that instead.
 */
// @ExperimentalCoroutinesApi // Since 1.3.x
// fun <T> ObservableValue<T>.asFlow(): Flow<T>
// TODO: Extension function -> free function taking ObservableValue as first parameter
template<typename T>
Flow<T>* as_flow(ObservableValue<T>& observable_value) {
    // callbackFlow<T> {
    //     auto listener = ChangeListener<T> { _, _, newValue ->
    //         /*
    //          * Do not propagate the exception to the ObservableValue, it
    //          * already should've been handled by the downstream
    //          */
    //         trySend(newValue)
    //     }
    //     addListener(listener)
    //     send(value)
    //     awaitClose {
    //         removeListener(listener)
    //     }
    // }.flowOn(Dispatchers.JavaFx).conflate()

    // TODO: Implement callback_flow builder
    // TODO: Create ChangeListener with lambda
    // TODO: observable_value.add_listener(listener)
    // TODO: send(observable_value.get_value())
    // TODO: await_close with cleanup lambda
    // TODO: Apply flow_on(Dispatchers::java_fx())
    // TODO: Apply conflate()

    return nullptr; // TODO: Return actual Flow instance
}

} // namespace javafx
} // namespace coroutines
} // namespace kotlinx
