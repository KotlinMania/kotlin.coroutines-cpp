#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ - kotlinx.coroutines.test.TestCoroutineDispatchers
// Original package: kotlinx.coroutines.test
//
// TODO: Import statements removed; fully qualify types or add appropriate includes
// TODO: suspend functions translated as normal functions; coroutine semantics NOT implemented
// TODO: Annotations like @ExperimentalCoroutinesApi, @Suppress preserved as comments
// TODO: Kotlin property access patterns need C++ getters/setters
// TODO: Function-level annotations (@Suppress("FunctionName")) preserved in comments

#include <memory>
#include <optional>
#include <string>
#include <stdexcept>

namespace kotlinx {
namespace coroutines {
namespace test {

// package kotlinx.coroutines.test

// import kotlinx.coroutines.*
// import kotlinx.coroutines.channels.*
// import kotlinx.coroutines.flow.*
// import kotlinx.coroutines.test.internal.TestMainDispatcher
// import kotlin.coroutines.*

/**
 * Creates an instance of an unconfined [TestDispatcher].
 *
 * This dispatcher is similar to [Dispatchers.Unconfined]: the tasks that it executes are not confined to any particular
 * thread and form an event loop; it's different in that it skips delays, as all [TestDispatcher]s do.
 *
 * Like [Dispatchers.Unconfined], this one does not provide guarantees about the execution order when several coroutines
 * are queued in this dispatcher. However, we ensure that the [launch] and [async] blocks at the top level of [runTest]
 * are entered eagerly. This allows launching child coroutines and not calling [runCurrent] for them to start executing.
 *
 * ```
 * @Test
 * auto test_eagerly_entering_child_coroutines() = runTest(UnconfinedTestDispatcher()) {
 *     auto entered = false
 *     auto deferred = CompletableDeferred<Unit>()
 *     auto completed = false
 *     launch {
 *         entered = true
 *         deferred.await()
 *         completed = true
 *     }
 *     assertTrue(entered) // `entered = true` already executed.
 *     assertFalse(completed) // however, the child coroutine then suspended, so it is enqueued.
 *     deferred.complete(Unit) // resume the coroutine.
 *     assertTrue(completed) // now the child coroutine is immediately completed.
 * }
 * ```
 *
 * Using this [TestDispatcher] can greatly simplify writing tests where it's not important which thread is used when and
 * in which order the queued coroutines are executed.
 * Another typical use case for this dispatcher is launching child coroutines that are resumed immediately, without
 * going through a dispatch; this can be helpful for testing [Channel] and [StateFlow] usages.
 *
 * ```
 * @Test
 * auto test_unconfined_dispatcher() = runTest {
 *     auto values = mutableListOf<Int>()
 *     auto stateFlow = MutableStateFlow(0)
 *     auto job = launch(UnconfinedTestDispatcher(testScheduler)) {
 *         stateFlow.collect {
 *             values.add(it)
 *         }
 *     }
 *     stateFlow.value = 1
 *     stateFlow.value = 2
 *     stateFlow.value = 3
 *     job.cancel()
 *     // each assignment will immediately resume the collecting child coroutine,
 *     // so no values will be skipped.
 *     assertEquals(listOf(0, 1, 2, 3), values)
 * }
 * ```
 *
 * Please be aware that, like [Dispatchers.Unconfined], this is a specific dispatcher with execution order
 * guarantees that are unusual and not shared by most other dispatchers, so it can only be used reliably for testing
 * functionality, not the specific order of actions.
 * See [Dispatchers.Unconfined] for a discussion of the execution order guarantees.
 *
 * In order to support delay skipping, this dispatcher is linked to a [TestCoroutineScheduler], which is used to control
 * the virtual time and can be shared among many test dispatchers.
 * If no [scheduler] is passed as an argument, [Dispatchers.Main] is checked, and if it was mocked with a
 * [TestDispatcher] via [Dispatchers.setMain], the [TestDispatcher.scheduler] of the mock dispatcher is used; if
 * [Dispatchers.Main] is not mocked with a [TestDispatcher], a new [TestCoroutineScheduler] is created.
 *
 * Additionally, [name] can be set to distinguish each dispatcher instance when debugging.
 *
 * @see StandardTestDispatcher for a more predictable [TestDispatcher].
 */
// @ExperimentalCoroutinesApi
// @Suppress("FunctionName")
// TODO: Kotlin factory function pattern; in C++ use constructor or static factory
TestDispatcher* unconfined_test_dispatcher(
    TestCoroutineScheduler* scheduler = nullptr,
    const std::string* name = nullptr
) {
    auto* actual_scheduler = scheduler != nullptr ? scheduler :
        (TestMainDispatcher::current_test_scheduler() != nullptr ?
            TestMainDispatcher::current_test_scheduler() :
            new TestCoroutineScheduler());
    return new UnconfinedTestDispatcherImpl(actual_scheduler, name);
}

class UnconfinedTestDispatcherImpl : TestDispatcher {
private:
    TestCoroutineScheduler* scheduler_;
    std::optional<std::string> name_;

public:
    UnconfinedTestDispatcherImpl(
        TestCoroutineScheduler* sched,
        const std::string* name_arg
    ) : scheduler_(sched), name_(name_arg ? std::optional<std::string>(*name_arg) : std::nullopt) {}

    TestCoroutineScheduler& scheduler() override {
        return *scheduler_;
    }

    bool is_dispatch_needed(const CoroutineContext& context) const override {
        return false;
    }

    // do not remove the INVISIBLE_REFERENCE and INVISIBLE_SETTER suppressions: required in K2
    // @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE", "INVISIBLE_SETTER")
    void dispatch(const CoroutineContext& context, Runnable* block) override {
        check_scheduler_in_context(*scheduler_, context);
        scheduler_->send_dispatch_event(context);

        /** copy-pasted from [kotlinx.coroutines.Unconfined.dispatch] */
        /** It can only be called by the [yield] function. See also code of [yield] function. */
        auto* yield_context = context[YieldContext::Key];
        if (yield_context != nullptr) {
            // report to "yield" that it is an unconfined dispatcher and don't call "block.run()"
            yield_context->dispatcher_was_unconfined = true;
            return;
        }
        throw std::runtime_error(
            "Function UnconfinedTestCoroutineDispatcher.dispatch can only be used by " +
            std::string("the yield function. If you wrap Unconfined dispatcher in your code, make sure you properly delegate ") +
            "isDispatchNeeded and dispatch calls."
        );
    }

    std::string to_string() const override {
        std::string base_name = name_ ? *name_ : "UnconfinedTestDispatcher";
        return base_name + "[scheduler=" + scheduler_->to_string() + "]";
    }
};

/**
 * Creates an instance of a [TestDispatcher] whose tasks are run inside calls to the [scheduler].
 *
 * This [TestDispatcher] instance does not itself execute any of the tasks. Instead, it always sends them to its
 * [scheduler], which can then be accessed via [TestCoroutineScheduler.runCurrent],
 * [TestCoroutineScheduler.advanceUntilIdle], or [TestCoroutineScheduler.advanceTimeBy], which will then execute these
 * tasks in a blocking manner.
 *
 * In practice, this means that [launch] or [async] blocks will not be entered immediately (unless they are
 * parameterized with [CoroutineStart.UNDISPATCHED]), and one should either call [TestCoroutineScheduler.runCurrent] to
 * run these pending tasks, which will block until there are no more tasks scheduled at this point in time, or, when
 * inside [runTest], call [yield] to yield the (only) thread used by [runTest] to the newly-launched coroutines.
 *
 * If no [scheduler] is passed as an argument, [Dispatchers.Main] is checked, and if it was mocked with a
 * [TestDispatcher] via [Dispatchers.setMain], the [TestDispatcher.scheduler] of the mock dispatcher is used; if
 * [Dispatchers.Main] is not mocked with a [TestDispatcher], a new [TestCoroutineScheduler] is created.
 *
 * One can additionally pass a [name] in order to more easily distinguish this dispatcher during debugging.
 *
 * @see UnconfinedTestDispatcher for a dispatcher that is not confined to any particular thread.
 */
// @Suppress("FunctionName")
// TODO: Kotlin factory function pattern; in C++ use constructor or static factory
TestDispatcher* standard_test_dispatcher(
    TestCoroutineScheduler* scheduler = nullptr,
    const std::string* name = nullptr
) {
    auto* actual_scheduler = scheduler != nullptr ? scheduler :
        (TestMainDispatcher::current_test_scheduler() != nullptr ?
            TestMainDispatcher::current_test_scheduler() :
            new TestCoroutineScheduler());
    return new StandardTestDispatcherImpl(actual_scheduler, name);
}

class StandardTestDispatcherImpl : TestDispatcher {
private:
    TestCoroutineScheduler* scheduler_;
    std::optional<std::string> name_;

public:
    StandardTestDispatcherImpl(
        TestCoroutineScheduler* sched = nullptr,
        const std::string* name_arg = nullptr
    ) : scheduler_(sched != nullptr ? sched : new TestCoroutineScheduler()),
        name_(name_arg ? std::optional<std::string>(*name_arg) : std::nullopt) {}

    TestCoroutineScheduler& scheduler() override {
        return *scheduler_;
    }

    void dispatch(const CoroutineContext& context, Runnable* block) override {
        scheduler_->register_event(this, 0, block, context, [](Runnable*) { return false; });
    }

    std::string to_string() const override {
        std::string base_name = name_ ? *name_ : "StandardTestDispatcher";
        return base_name + "[scheduler=" + scheduler_->to_string() + "]";
    }
};

} // namespace test
} // namespace coroutines
} // namespace kotlinx
