#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ - kotlinx.coroutines.test.TestDispatcher
// Original package: kotlinx.coroutines.test
//
// TODO: Import statements removed; fully qualify types or add appropriate includes
// TODO: suspend functions translated as normal functions; coroutine semantics NOT implemented
// TODO: Annotations (@Suppress, @Deprecated) preserved as comments
// TODO: Kotlin abstract class needs C++ virtual methods
// TODO: DelayWithTimeoutDiagnostics is an interface, needs special handling

#include <functional>
#include <string>
#include <chrono>

namespace kotlinx {
namespace coroutines {
namespace test {

// package kotlinx.coroutines.test

// import kotlinx.coroutines.*
// import kotlin.coroutines.*
// import kotlin.jvm.*
// import kotlin.time.*

/**
 * A test dispatcher that can struct with a [TestCoroutineScheduler].
 *
 * The available implementations are:
 * - [StandardTestDispatcher] is a dispatcher that places new tasks into a queue.
 * - [UnconfinedTestDispatcher] is a dispatcher that behaves like [Dispatchers.Unconfined] while allowing to control
 *   the virtual time.
 */
// @Suppress("INVISIBLE_REFERENCE")
class TestDispatcher : CoroutineDispatcher, Delay, DelayWithTimeoutDiagnostics {
protected:
    // TODO: constructor - make protected or document
    TestDispatcher() {}

public:
    /** The scheduler that this dispatcher is linked to. */
    virtual TestCoroutineScheduler& scheduler() = 0;

    /** Notifies the dispatcher that it should process a single event marked with [marker] happening at time [time]. */
    void process_event(void* marker) {
        // TODO: check() in Kotlin is assertion-like; map to C++ assertion or exception
        if (dynamic_cast<Runnable*>(static_cast<Runnable*>(marker)) == nullptr) {
            throw std::logic_error("marker must be a Runnable");
        }
        static_cast<Runnable*>(marker)->run();
    }

    /** @suppress */
    void schedule_resume_after_delay(int64_t time_millis, CancellableContinuation<void>* continuation) override {
        auto* timed_runnable = new CancellableContinuationRunnable(continuation, this);
        auto* handle = scheduler().register_event(
            this,
            time_millis,
            timed_runnable,
            continuation->context(),
            &cancellable_runnable_is_cancelled
        );
        continuation->dispose_on_cancellation(handle);
    }

    /** @suppress */
    DisposableHandle* invoke_on_timeout(int64_t time_millis, Runnable* block, const CoroutineContext& context) override {
        return scheduler().register_event(this, time_millis, block, context, [](Runnable*) { return false; });
    }

    /** @suppress */
    // @Suppress("CANNOT_OVERRIDE_INVISIBLE_MEMBER")
    // @Deprecated("Is only needed internally", level = DeprecationLevel.HIDDEN)
    [[deprecated("Is only needed internally")]]
    std::string timeout_message(Duration timeout) const override {
        return "Timed out after " + std::to_string(timeout.count()) + " of _virtual_ (kotlinx.coroutines.test) time. " +
            "To use the real time, wrap 'withTimeout' in 'withContext(Dispatchers.Default.limitedParallelism(1))'";
    }
};

/**
 * This class exists to allow cleanup code to avoid throwing for cancelled continuations scheduled
 * in the future.
 */
class CancellableContinuationRunnable : Runnable {
public:
    // @JvmField
    CancellableContinuation<void>* continuation;
private:
    CoroutineDispatcher* dispatcher_;

public:
    CancellableContinuationRunnable(CancellableContinuation<void>* cont, CoroutineDispatcher* disp)
        : continuation(cont), dispatcher_(disp) {}

    void run() override {
        // TODO: Kotlin's 'with' scoping function and extension receiver pattern
        // with(dispatcher) { with(continuation) { resumeUndispatched(Unit) } }
        dispatcher_->resume_undispatched(*continuation, {});
    }
};

bool cancellable_runnable_is_cancelled(CancellableContinuationRunnable* runnable) {
    return !runnable->continuation->is_active();
}

} // namespace test
} // namespace coroutines
} // namespace kotlinx
