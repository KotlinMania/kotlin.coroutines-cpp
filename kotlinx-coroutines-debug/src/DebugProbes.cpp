// Original Kotlin package: kotlinx.coroutines.debug
// Line-by-line C++ transliteration from Kotlin
//
// TODO: @file:Suppress - Kotlin compiler directives
// TODO: @ExperimentalCoroutinesApi - Kotlin annotation
// TODO: object - Kotlin singleton, translate to namespace with static members or singleton class
// TODO: inline function - C++ inline keyword
// TODO: PrintStream - Java I/O, translate to std::ostream
// TODO: Job, CoroutineScope, CoroutineContext - Kotlin coroutine types
// TODO: DebugProbesImpl - internal implementation class
// TODO: Property getters/setters with @Suppress("INVISIBLE_SETTER")

#include <vector>
#include <string>
#include <ostream>
#include <iostream>

// Forward declarations
namespace kotlinx {
namespace coroutines {
class Job;
class CoroutineScope;
class CoroutineContext;
namespace debug {
class CoroutineInfo;
namespace internal {
class DebugProbesImpl;
}
}
}
}

namespace kotlinx {
namespace coroutines {
namespace debug {

// Kotlin debug probes support.
//
// Debug probes is a dynamic attach mechanism which installs multiple hooks into coroutines machinery.
// It slows down all coroutine-related code, but in return provides diagnostic information, including
// asynchronous stacktraces, coroutine dumps (similar to ThreadMXBean.dumpAllThreads and `jstack`) via DebugProbes.dumpCoroutines,
// and programmatic introspection of all alive coroutines.
// All introspecting methods throw exception if debug probes were not installed.
//
// ### Consistency guarantees
//
// All snapshotting operations (e.g. dumpCoroutines) are *weakly-consistent*, meaning that they happen
// concurrently with coroutines progressing their own state. These operations are guaranteed to observe
// each coroutine's state exactly once, but the state is not guaranteed to be the most recent before the operation.
// In practice, it means that for snapshotting operations in progress, for each concurrent coroutine either
// the state prior to the operation or the state that was reached during the current operation is observed.
//
// ### Overhead
//
//  - Every created coroutine is stored in a concurrent hash map, and the hash map is looked up in and
//    updated on each suspension and resumption.
//  - If DebugProbes.enableCreationStackTraces is enabled, stack trace of the current thread is captured on
//    each created coroutine that is a rough equivalent of throwing an exception per each created coroutine.
//
// ### Internal machinery and classloading.
//
// Under the hood, debug probes replace internal `kotlin.coroutines.jvm.internal.DebugProbesKt` class that has the following
// empty static methods:
//
// - `probeCoroutineResumed` that is invoked on every Continuation.resume.
// - `probeCoroutineSuspended` that is invoked on every continuation suspension.
// - `probeCoroutineCreated` that is invoked on every coroutine creation.
//
// with a `kotlinx-coroutines`-specific class to keep track of all the coroutines machinery.
//
// The new class is located in the `kotlinx-coroutines-core` module, meaning that all target application classes that use
// coroutines and `suspend` functions have to be loaded by the classloader in which `kotlinx-coroutines-core` classes are available.
// TODO: @ExperimentalCoroutinesApi
// TODO: object -> namespace with static functions or singleton pattern
namespace DebugProbes {

    // Whether coroutine creation stack traces should be sanitized.
    // Sanitization removes all frames from `kotlinx.coroutines` package except
    // the first one and the last one to simplify diagnostic.
    //
    // `true` by default.
    // TODO: Property with getter/setter -> static functions
    inline bool get_sanitize_stack_traces() {
        // TODO: return DebugProbesImpl::sanitize_stack_traces;
        return true;
    }

    // TODO: @Suppress("INVISIBLE_SETTER")
    inline void set_sanitize_stack_traces(bool value) {
        // TODO: DebugProbesImpl::sanitize_stack_traces = value;
    }

    // Whether coroutine creation stack traces should be captured.
    // When enabled, for each created coroutine a stack trace of the current thread is captured and attached to the coroutine.
    // This option can be useful during local debug sessions, but is recommended
    // to be disabled in production environments to avoid performance overhead of capturing real stacktraces.
    //
    // `false` by default.
    inline bool get_enable_creation_stack_traces() {
        // TODO: return DebugProbesImpl::enable_creation_stack_traces;
        return false;
    }

    // TODO: @Suppress("INVISIBLE_SETTER")
    inline void set_enable_creation_stack_traces(bool value) {
        // TODO: DebugProbesImpl::enable_creation_stack_traces = value;
    }

    // Whether to ignore coroutines whose context is EmptyCoroutineContext.
    //
    // Coroutines with empty context are considered to be irrelevant for the concurrent coroutines' observability:
    // - They do not contribute to any concurrent executions
    // - They do not contribute to the (concurrent) system's liveness and/or deadlocks, as no other coroutines might wait for them
    // - The typical usage of such coroutines is a combinator/builder/lookahead parser that can be debugged using more convenient tools.
    //
    // `true` by default.
    inline bool get_ignore_coroutines_with_empty_context() {
        // TODO: return DebugProbesImpl::ignore_coroutines_with_empty_context;
        return true;
    }

    // TODO: @Suppress("INVISIBLE_SETTER")
    inline void set_ignore_coroutines_with_empty_context(bool value) {
        // TODO: DebugProbesImpl::ignore_coroutines_with_empty_context = value;
    }

    // Determines whether debug probes were installed.
    inline bool is_installed() {
        // TODO: return DebugProbesImpl::is_installed();
        return false;
    }

    // Installs a DebugProbes instead of no-op stdlib probes by redefining
    // debug probes class using the same class loader as one loaded DebugProbes class.
    inline void install() {
        // TODO: DebugProbesImpl::install();
    }

    // Uninstall debug probes.
    inline void uninstall() {
        // TODO: DebugProbesImpl::uninstall();
    }

    // Invokes given block of code with installed debug probes and uninstall probes in the end.
    // TODO: inline function with lambda
    template<typename F>
    inline void with_debug_probes(F&& block) {
        install();
        try {
            block();
        } catch (...) {
            uninstall();
            throw;
        }
        uninstall();
    }

    // Returns string representation of the coroutines job hierarchy with additional debug information.
    // Hierarchy is printed from the job as a root transitively to all children.
    inline std::string job_to_string(Job* job) {
        // TODO: return DebugProbesImpl::hierarchy_to_string(job);
        return "";
    }

    // Returns string representation of all coroutines launched within the given scope.
    // Throws exception if the scope has no a job in it.
    inline std::string scope_to_string(CoroutineScope* scope) {
        // TODO: job_to_string(scope.coroutineContext[Job] ?: error("Job is not present in the scope"))
        // TODO: implement context[Job] lookup and error handling
        return "";
    }

    // Prints job hierarchy representation from job_to_string to the given out.
    // TODO: PrintStream -> std::ostream&
    inline void print_job(Job* job, std::ostream& out = std::cout) {
        // TODO: out.println(DebugProbesImpl::hierarchy_to_string(job));
        out << job_to_string(job) << std::endl;
    }

    // Prints all coroutines launched within the given scope.
    // Throws exception if the scope has no a job in it.
    inline void print_scope(CoroutineScope* scope, std::ostream& out = std::cout) {
        // TODO: print_job(scope.coroutineContext[Job] ?: error("Job is not present in the scope"), out);
    }

    // Returns all existing coroutines' info.
    // The resulting collection represents a consistent snapshot of all existing coroutines at the moment of invocation.
    inline std::vector<CoroutineInfo> dump_coroutines_info() {
        // TODO: return DebugProbesImpl::dump_coroutines_info().map { CoroutineInfo(it) };
        return {};
    }

    // Dumps all active coroutines into the given output stream, providing a consistent snapshot of all existing coroutines at the moment of invocation.
    // The output of this method is similar to `jstack` or a full thread dump. It can be used as the replacement to
    // "Dump threads" action.
    //
    // Example of the output:
    // ```
    // Coroutines dump 2018/11/12 19:45:14
    //
    // Coroutine "coroutine#42":StandaloneCoroutine{Active}@58fdd99, state: SUSPENDED
    //     at MyClass$awaitData.invokeSuspend(MyClass.kt:37)
    //     at _COROUTINE._CREATION._(CoroutineDebugging.kt)
    //     at MyClass.createIoRequest(MyClass.kt:142)
    //     at MyClass.fetchData(MyClass.kt:154)
    //     at MyClass.showData(MyClass.kt:31)
    // ...
    // ```
    inline void dump_coroutines(std::ostream& out = std::cout) {
        // TODO: DebugProbesImpl::dump_coroutines(out);
    }

} // namespace DebugProbes

} // namespace debug
} // namespace coroutines
} // namespace kotlinx
