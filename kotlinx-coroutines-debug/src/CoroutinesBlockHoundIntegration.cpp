#include "kotlinx/coroutines/core_fwd.hpp"
// Original Kotlin package: kotlinx.coroutines.debug
// Line-by-line C++ transliteration from Kotlin
//
// TODO: @file:Suppress - Kotlin compiler directives, not applicable in C++
// TODO: BlockHound and BlockHoundIntegration - Reactor library for detecting blocking calls on non-blocking threads
//       This is Java-specific and has no direct C++ equivalent
// TODO: All BlockHound.Builder methods - Java API, no C++ equivalent
// TODO: @suppress annotation - Kotlin documentation directive

#include <string>
#include <vector>

namespace kotlinx {
namespace coroutines {
namespace debug {

// TODO: BlockHound and BlockHoundIntegration are Java/Reactor specific
// This entire class is specific to JVM blocking detection and has no C++ equivalent

// @suppress
class CoroutinesBlockHoundIntegration /* TODO: : BlockHoundIntegration */ {
public:
    // TODO: virtual auto apply_to(builder: BlockHound.Builder)
    void apply_to(/* TODO: BlockHound.Builder& builder */) {
        // TODO: All these methods are BlockHound.Builder extensions
        // allow_blocking_calls_in_primitive_implementations();
        // allow_blocking_when_enqueueing_tasks();
        // allow_service_loader_invocations_on_init();
        // allow_blocking_calls_in_reflection_impl();
        // allow_blocking_calls_in_debug_probes();
        // allow_blocking_calls_in_work_queue();

        // Stacktrace recovery cache is guarded by lock
        // allow_blocking_calls_inside("kotlinx.coroutines.internal.ExceptionsConstructorKt", "tryCopyException");

        // The predicates that define that BlockHound should only report blocking calls from threads that are part of
        // the coroutine thread pool and currently execute a CPU-bound coroutine computation.
        // add_dynamic_thread_predicate([](auto it) { return is_scheduler_worker(it); });
        // non_blocking_thread_predicate([](auto p) { return p.or([](auto it) { return may_not_block(it); }); });
    }

private:
    // Allows blocking calls in various coroutine structures, such as flows and channels.
    //
    // They use locks in implementations, though only for protecting short pieces of fast and well-understood code, so
    // locking in such places doesn't affect the program liveness.
    void allow_blocking_calls_in_primitive_implementations(/* TODO: BlockHound.Builder& */) {
        // allow_blocking_calls_in_job_support();
        // allow_blocking_calls_in_thread_safe_heap();
        // allow_blocking_calls_in_flow();
        // allow_blocking_calls_in_channels();
    }

    // Allows blocking inside kotlinx.coroutines.JobSupport.
    void allow_blocking_calls_in_job_support(/* TODO: BlockHound.Builder& */) {
        std::vector<std::string> methods = {
            "finalizeFinishingState", "invokeOnCompletion", "makeCancelling", "tryMakeCompleting"
        };
        for (const auto& method : methods) {
            // TODO: allow_blocking_calls_inside("kotlinx.coroutines.JobSupport", method);
        }
    }

    // Allow blocking calls inside kotlinx.coroutines.debug.internal.DebugProbesImpl.
    void allow_blocking_calls_in_debug_probes(/* TODO: BlockHound.Builder& */) {
        std::vector<std::string> methods = {
            "install", "uninstall", "hierarchyTostd::string", "dumpCoroutinesInfo", "dumpDebuggerInfo",
            "dumpCoroutinesSynchronized", "updateRunningState", "updateState"
        };
        for (const auto& method : methods) {
            // TODO: allow_blocking_calls_inside("kotlinx.coroutines.debug.internal.DebugProbesImpl", method);
        }
    }

    // Allow blocking calls inside kotlinx.coroutines.scheduling.WorkQueue
    void allow_blocking_calls_in_work_queue(/* TODO: BlockHound.Builder& */) {
        // uses Thread.yield in a benign way.
        // TODO: allow_blocking_calls_inside("kotlinx.coroutines.scheduling.WorkQueue", "addLast");
    }

    // Allows blocking inside kotlinx.coroutines.internal.ThreadSafeHeap.
    void allow_blocking_calls_in_thread_safe_heap(/* TODO: BlockHound.Builder& */) {
        std::vector<std::string> methods = {"clear", "peek", "removeFirstOrNull", "addLast"};
        for (const auto& method : methods) {
            // TODO: allow_blocking_calls_inside("kotlinx.coroutines.internal.ThreadSafeHeap", method);
        }
    }

    void allow_blocking_calls_in_flow(/* TODO: BlockHound.Builder& */) {
        // allow_blocking_calls_inside_state_flow();
        // allow_blocking_calls_inside_shared_flow();
    }

    // Allows blocking inside the implementation of kotlinx.coroutines.flow.StateFlow.
    void allow_blocking_calls_inside_state_flow(/* TODO: BlockHound.Builder& */) {
        // TODO: allow_blocking_calls_inside("kotlinx.coroutines.flow.StateFlowImpl", "updateState");
    }

    // Allows blocking inside the implementation of kotlinx.coroutines.flow.SharedFlow.
    void allow_blocking_calls_inside_shared_flow(/* TODO: BlockHound.Builder& */) {
        std::vector<std::string> methods = {
            "emitSuspend", "awaitValue", "getReplayCache", "tryEmit", "cancelEmitter",
            "tryTakeValue", "resetReplayCache"
        };
        for (const auto& method : methods) {
            // TODO: allow_blocking_calls_inside("kotlinx.coroutines.flow.SharedFlowImpl", method);
        }

        std::vector<std::string> methods2 = {"getSubscriptionCount", "allocateSlot", "freeSlot"};
        for (const auto& method : methods2) {
            // TODO: allow_blocking_calls_inside("kotlinx.coroutines.flow.internal.AbstractSharedFlow", method);
        }
    }

    void allow_blocking_calls_in_channels(/* TODO: BlockHound.Builder& */) {
        // allow_blocking_calls_in_broadcast_channels();
        // allow_blocking_calls_in_conflated_channels();
    }

    // Allows blocking inside kotlinx.coroutines.channels.BroadcastChannel.
    void allow_blocking_calls_in_broadcast_channels(/* TODO: BlockHound.Builder& */) {
        std::vector<std::string> methods = {
            "openSubscription", "removeSubscriber", "send", "trySend", "registerSelectForSend",
            "close", "cancelImpl", "isClosedForSend", "value", "valueOrNull"
        };
        for (const auto& method : methods) {
            // TODO: allow_blocking_calls_inside("kotlinx.coroutines.channels.BroadcastChannelImpl", method);
        }

        std::vector<std::string> methods2 = {"cancelImpl"};
        for (const auto& method : methods2) {
            // TODO: allow_blocking_calls_inside("kotlinx.coroutines.channels.BroadcastChannelImpl$SubscriberConflated", method);
        }

        for (const auto& method : methods2) {
            // TODO: allow_blocking_calls_inside("kotlinx.coroutines.channels.BroadcastChannelImpl$SubscriberBuffered", method);
        }
    }

    // Allows blocking inside kotlinx.coroutines.channels.ConflatedBufferedChannel.
    void allow_blocking_calls_in_conflated_channels(/* TODO: BlockHound.Builder& */) {
        std::vector<std::string> methods = {
            "receive", "receiveCatching", "tryReceive", "registerSelectForReceive",
            "send", "trySend", "sendBroadcast", "registerSelectForSend",
            "close", "cancelImpl", "isClosedForSend", "isClosedForReceive", "isEmpty"
        };
        for (const auto& method : methods) {
            // TODO: allow_blocking_calls_inside("kotlinx.coroutines.channels.ConflatedBufferedChannel", method);
        }

        std::vector<std::string> methods2 = {"hasNext"};
        for (const auto& method : methods2) {
            // TODO: allow_blocking_calls_inside("kotlinx.coroutines.channels.ConflatedBufferedChannel$ConflatedChannelIterator", method);
        }
    }

    // Allows blocking when enqueuing tasks into a thread pool.
    //
    // Without this, the following code breaks:
    // ```
    // withContext(Dispatchers.Default) {
    //     withContext(newSingleThreadContext("singleThreadedContext")) {
    //     }
    // }
    // ```
    void allow_blocking_when_enqueueing_tasks(/* TODO: BlockHound.Builder& */) {
        // This method may block as part of its implementation, but is probably safe.
        // TODO: allow_blocking_calls_inside("java.util.concurrent.ScheduledThreadPoolExecutor", "execute");
    }

    // Allows instances of java.util.ServiceLoader being called.
    //
    // Each instance is listed separately; another approach could be to generally allow the operations performed by
    // service loaders, as they can generally be considered safe. This was not done here because ServiceLoader has a
    // large API surface, with some methods being hidden as implementation details (in particular, the implementation of
    // its iterator is completely opaque). Relying on particular names being used in ServiceLoader's implementation
    // would be brittle, so here we only provide clearance rules for some specific instances.
    void allow_service_loader_invocations_on_init(/* TODO: BlockHound.Builder& */) {
        // TODO: allow_blocking_calls_inside("kotlinx.coroutines.reactive.ReactiveFlowKt", "<clinit>");
        // TODO: allow_blocking_calls_inside("kotlinx.coroutines.CoroutineExceptionHandlerImplKt", "<clinit>");
        // not part of the coroutines library, but it would be nice if reflection also wasn't considered blocking
        // TODO: allow_blocking_calls_inside("kotlin.reflect.jvm.internal.impl.resolve.OverridingUtil", "<clinit>");
    }

    // Allows some blocking calls from the reflection API.
    //
    // The API is big, so surely some other blocking calls will show up, but with these rules in place, at least some
    // simple examples work without problems.
    void allow_blocking_calls_in_reflection_impl(/* TODO: BlockHound.Builder& */) {
        // TODO: allow_blocking_calls_inside("kotlin.reflect.jvm.internal.impl.builtins.jvm.JvmBuiltInsPackageFragmentProvider", "findPackage");
    }
};

} // namespace debug
} // namespace coroutines
} // namespace kotlinx
