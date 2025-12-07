// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/EventLoop.kt
//
// TODO: @file:OptIn annotation
// TODO: actual keyword - platform-specific implementation
// TODO: Worker API from Kotlin/Native
// TODO: TimeSource API from Kotlin

namespace kotlinx {
namespace coroutines {

// TODO: Remove imports, fully qualify or add includes:
// import kotlin.coroutines.*
// import kotlin.native.concurrent.*
// import kotlin.time.*

// TODO: internal actual abstract class
class EventLoopImplPlatform : public EventLoop {
private:
    // TODO: Worker.current equivalent
    void* current; // = Worker.current

protected:
    void unpark() {
        // TODO: current.executeAfter equivalent
        // current.executeAfter(0L, {}) // send an empty task to unpark the waiting event loop
    }

    void reschedule(long now, EventLoopImplBase::DelayedTask* delayed_task) {
        auto delay_time_millis = delay_nanos_to_millis(delayed_task->nano_time - now);
        DefaultExecutor::instance().invoke_on_timeout(delay_time_millis, *delayed_task, EmptyCoroutineContext);
    }
};

// TODO: internal class
class EventLoopImpl : public EventLoopImplBase {
public:
    DisposableHandle invoke_on_timeout(long time_millis, Runnable block, CoroutineContext context) override {
        return kDefaultDelay.invoke_on_timeout(time_millis, block, context);
    }
};

// TODO: internal actual function
EventLoop* create_event_loop() {
    return new EventLoopImpl();
}

// TODO: private val with TimeSource.Monotonic
// private val startingPoint = TimeSource.Monotonic.markNow()
namespace {
    // TODO: TimeSource equivalent
    long starting_point = 0;
}

// TODO: internal actual function
long nano_time() {
    // TODO: (TimeSource.Monotonic.markNow() - startingPoint).inWholeNanoseconds
    return 0;
}

} // namespace coroutines
} // namespace kotlinx
