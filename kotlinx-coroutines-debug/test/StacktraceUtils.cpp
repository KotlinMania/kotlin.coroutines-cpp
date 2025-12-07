// Original file: kotlinx-coroutines-debug/test/StacktraceUtils.kt
// TODO: Convert imports to C++ includes
// TODO: Implement string manipulation utilities
// TODO: Implement DebugProbes API
// TODO: Convert Kotlin regex to C++ equivalents
// TODO: Implement data classes as structs
// TODO: Implement companion object patterns

namespace kotlinx {
namespace coroutines {
namespace debug {

// Extension function converted to free function
std::string trim_stack_trace(const std::string& str) {
    // TODO: Implement trimIndent()
    //     .replace(Regex(":[0-9]+"), "")
    //     .replace(Regex("#[0-9]+"), "")
    //     .replace(Regex("(?<=\tat )[^\n]*/"), "")
    //     .replace(Regex("\t"), "")
    //     .replace("sun.misc.Unsafe.", "jdk.internal.misc.Unsafe.") // JDK8->JDK11
    return "";
}

void verify_stack_trace(const std::exception& e, const std::vector<std::string>& traces) {
    // TODO: auto stacktrace = toStackTrace(e)
    // auto trimmedStackTrace = stacktrace.trimStackTrace()
    // for (auto& trace : traces) {
    //     assertTrue(
    //         trimmedStackTrace.contains(trace.trimStackTrace()),
    //         "\nExpected trace element:\n" + trace + "\n\nActual stacktrace:\n" + stacktrace
    //     )
    // }
    //
    // auto causes = stacktrace.count("Caused by")
    // assertNotEquals(0, causes)
    // assertEquals(causes, traces.map { it.count("Caused by") }.sum())
}

std::string to_stack_trace(const std::exception& t) {
    // TODO: auto sw = StringWriter()
    // t.printStackTrace(PrintWriter(sw))
    // return sw.toString()
    return "";
}

int count_substring(const std::string& str, const std::string& substring) {
    // TODO: return str.split(substring).size - 1
    return 0;
}

void verify_dump_with_finally(const std::vector<std::string>& traces, const char* ignored_coroutine,
                               std::function<void()> finally_func) {
    // TODO: try {
    //     verifyDump(traces, ignoredCoroutine)
    // } finally {
    //     finally_func()
    // }
}

/** Clean the stacktraces from artifacts of BlockHound instrumentation
 *
 * BlockHound works by switching a native call by a class generated with ByteBuddy, which, if the blocking
 * call is allowed in this context, in turn calls the real native call that is now available under a
 * different name.
 *
 * The traces thus undergo the following two changes when the execution is instrumented:
 *   - The original native call is replaced with a non-native one with the same FQN, and
 *   - An additional native call is placed on top of the stack, with the original name that also has
 *     `$$BlockHound$$_` prepended at the last component.
 */
std::vector<std::string> clean_block_hound_traces(const std::vector<std::string>& frames) {
    std::vector<std::string> result;
    const std::string block_hound_substr = "$$BlockHound$$_";
    size_t i = 0;
    while (i < frames.size()) {
        // TODO: result.push_back(frames[i].replace(blockHoundSubstr, ""))
        // if (frames[i].contains(blockHoundSubstr)) {
        //     i += 1
        // }
        i += 1;
    }
    return result;
}

/**
 * Removes all frames that contain "java.util.concurrent" in it.
 *
 * We do leverage Java's locks for proper rendezvous and to fix the coroutine stack's state,
 * but this API doesn't have (nor expected to) stable stacktrace, so we are filtering all such
 * frames out.
 *
 * See https://github.com/Kotlin/kotlinx.coroutines/issues/3700 for the example of failure
 */
std::vector<std::string> remove_java_util_concurrent_traces(const std::vector<std::string>& frames) {
    // TODO: return frames.filter { !it.contains("java.util.concurrent") }
    return {};
}

struct CoroutineDump {
    struct Header {
        std::string* name;      // String?
        std::string class_name;
        std::string state;

        // TODO: static Header parse(const std::string& header)
    };

    Header header;
    std::vector<std::string> coroutine_stack_trace;
    std::vector<std::string> thread_stack_trace;
    std::string origin_dump;
    std::string origin_header;

    // TODO: static CoroutineDump parse(const std::string& dump, ...)

    void verify(const CoroutineDump& expected) {
        // TODO: assertEquals(expected.header, header, ...)
        // verify_stack_trace("coroutine stack", coroutine_stack_trace, expected.coroutine_stack_trace)
        // verify_stack_trace("thread stack", thread_stack_trace, expected.thread_stack_trace)
    }

private:
    void verify_stack_trace(const std::string& trace_name,
                           const std::vector<std::string>& actual_stack_trace,
                           const std::vector<std::string>& expected_stack_trace) {
        // TODO: for (size_t ix = 0; ix < expected_stack_trace.size(); ++ix) {
        //     auto expectedLine = expected_stack_trace[ix]
        //     auto actualLine = actual_stack_trace[ix]
        //     assertEquals(expectedLine, actualLine, ...)
        // }
    }
};

void verify_dump(const std::vector<std::string>& expected_traces, const char* ignored_coroutine = nullptr) {
    // TODO: auto baos = ByteArrayOutputStream()
    // DebugProbes.dumpCoroutines(PrintStream(baos))
    // auto wholeDump = baos.toString()
    // auto traces = wholeDump.split("\n\n")
    // assertTrue(traces[0].startsWith("Coroutines dump"))
    //
    // auto dumps = traces
    //     .drop(1)
    //     .mapNotNull { trace ->
    //         auto dump = CoroutineDump.parse(trace, {
    //             removeJavaUtilConcurrentTraces(cleanBlockHoundTraces(it))
    //         })
    //         if (dump.header.className == ignoredCoroutine) {
    //             null
    //         } else {
    //             dump
    //         }
    //     }
    //
    // assertEquals(expectedTraces.size, dumps.size)
    // dumps.zip(expectedTraces.map { CoroutineDump.parse(it, ::removeJavaUtilConcurrentTraces) })
    //     .forEach { (dump, expectedDump) ->
    //         dump.verify(expectedDump)
    //     }
}

std::string trim_package(const std::string& str) {
    // TODO: return str.replace("kotlinx.coroutines.debug.", "")
    return "";
}

void verify_partial_dump(int created_coroutines_count, const std::vector<std::string>& frames) {
    // TODO: auto baos = ByteArrayOutputStream()
    // DebugProbes.dumpCoroutines(PrintStream(baos))
    // auto dump = baos.toString()
    // auto trace = dump.split("\n\n")
    // auto matches = std::all_of(frames.begin(), frames.end(), [&](const auto& frame) {
    //     return std::any_of(trace.begin(), trace.end(), [&](const auto& tr) {
    //         return tr.contains(frame)
    //     })
    // })
    //
    // assertEquals(created_coroutines_count, DebugProbes.dumpCoroutinesInfo().size)
    // assertTrue(matches)
}

} // namespace debug
} // namespace coroutines
} // namespace kotlinx
