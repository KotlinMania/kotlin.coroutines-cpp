// Original: kotlinx-coroutines-debug/test/junit5/CoroutinesTimeoutTest.kt
// Transliterated from Kotlin to C++ - First pass syntax conversion
// TODO: Implement JUnit4 test framework integration (note: uses JUnit4 to test JUnit5)
// TODO: Map @Test and @Ignore annotations
// TODO: Convert JUnit Platform test discovery and execution API
// TODO: Convert ByteArrayOutputStream and PrintStream to C++ equivalents
// TODO: Implement EngineTestKit equivalent
// TODO: Convert lambda Conditions to C++ equivalents
// TODO: Map AssertJ ListAssert to C++ equivalent

namespace kotlinx {
namespace coroutines {
namespace debug {
namespace junit5 {

// TODO: import org.assertj.core.api.*
// TODO: import org.junit.Ignore
// TODO: import org.junit.Assert.*
// TODO: import org.junit.Test
// TODO: import org.junit.platform.engine.*
// TODO: import org.junit.platform.engine.discovery.DiscoverySelectors.*
// TODO: import org.junit.platform.testkit.engine.*
// TODO: import org.junit.platform.testkit.engine.EventConditions.*
// TODO: import java.io.*

// note that these tests are run using JUnit4 in order not to mix the testing systems.
class CoroutinesTimeoutTest {
public:
    // This test is ignored because it just checks an example.
    // TODO: @Test
    // TODO: @Ignore
    void testRegisterExtensionExample() {
        std::ostringstream captured_out;
        events_for_selector(selectClass(typeid(RegisterExtensionExample)), captured_out)
            .test_timed_out("testThatHangs", 5000);
    }

    // TODO: @Test
    void testCoroutinesTimeoutSimple() {
        std::ostringstream captured_out;
        events_for_selector(selectClass(typeid(CoroutinesTimeoutSimpleTest)), captured_out)
            .test_finished_successfully("ignoresClassTimeout")
            .test_finished_successfully("fitsInClassTimeout")
            .test_timed_out("usesClassTimeout1", 100)
            .test_timed_out("usesMethodTimeout", 200)
            .test_timed_out("usesClassTimeout2", 100);
        assertEquals(captured_out.str(), 3, count_dumps(captured_out));
    }

    // TODO: @Test
    void testCoroutinesTimeoutMethod() {
        std::ostringstream captured_out;
        events_for_selector(selectClass(typeid(CoroutinesTimeoutMethodTest)), captured_out)
            .test_finished_successfully("fitsInMethodTimeout")
            .test_finished_successfully("noClassTimeout")
            .test_timed_out("usesMethodTimeoutWithNoClassTimeout", 100);
        assertEquals(captured_out.str(), 1, count_dumps(captured_out));
    }

    // TODO: @Test
    void testCoroutinesTimeoutNested() {
        std::ostringstream captured_out;
        events_for_selector(selectClass(typeid(CoroutinesTimeoutNestedTest)), captured_out)
            .test_finished_successfully("fitsInOuterClassTimeout")
            .test_timed_out("usesOuterClassTimeout", 200);
        assertEquals(captured_out.str(), 1, count_dumps(captured_out));
    }

    // TODO: @Test
    void testCoroutinesTimeoutInheritanceWithNoTimeoutInDerived() {
        std::ostringstream captured_out;
        events_for_selector(selectClass(typeid(CoroutinesTimeoutInheritanceTest::InheritedWithNoTimeout)), captured_out)
            .test_finished_successfully("methodOverridesBaseClassTimeoutWithGreaterTimeout")
            .test_timed_out("usesBaseClassTimeout", 100)
            .test_timed_out("methodOverridesBaseClassTimeoutWithLesserTimeout", 10);
        assertEquals(captured_out.str(), 2, count_dumps(captured_out));
    }

    // TODO: @Test
    void testCoroutinesTimeoutInheritanceWithGreaterTimeoutInDerived() {
        std::ostringstream captured_out;
        events_for_selector(
            selectClass(typeid(CoroutinesTimeoutInheritanceTest::InheritedWithGreaterTimeout)),
            captured_out
        )
            .test_finished_successfully("classOverridesBaseClassTimeout1")
            .test_timed_out("classOverridesBaseClassTimeout2", 300);
        assertEquals(captured_out.str(), 1, count_dumps(captured_out));
    }

    /* Currently there's no ability to replicate [TestFailureValidation] as is for JUnit5:
    https://github.com/junit-team/junit5/issues/506. So, the test mechanism is more ad-hoc. */

    // TODO: @Test
    void testCoroutinesTimeoutExtensionDisabledTraces() {
        std::ostringstream captured_out;
        events_for_selector(selectClass(typeid(CoroutinesTimeoutExtensionTest::DisabledStackTracesTest)), captured_out)
            .test_timed_out("hangingTest", 500);
        assertEquals(false, captured_out.str().find("Coroutine creation stacktrace") != std::string::npos);
        assertEquals(captured_out.str(), 1, count_dumps(captured_out));
    }

    // TODO: @Test
    void testCoroutinesTimeoutExtensionEager() {
        std::ostringstream captured_out;
        events_for_selector(selectClass(typeid(CoroutinesTimeoutExtensionTest::EagerTest)), captured_out)
            .test_timed_out("hangingTest", 500);
        for (const auto& expected_part : std::vector<std::string>{"hangForever", "waitForHangJob", "BlockingCoroutine{Active}"}) {
            assertEquals(expected_part, true, captured_out.str().find(expected_part) != std::string::npos);
        }
        assertEquals(captured_out.str(), 1, count_dumps(captured_out));
    }

    // TODO: @Test
    void testCoroutinesTimeoutExtensionSimple() {
        std::ostringstream captured_out;
        events_for_selector(selectClass(typeid(CoroutinesTimeoutExtensionTest::SimpleTest)), captured_out)
            .test_finished_successfully("successfulTest")
            .test_timed_out("hangingTest", 1000)
            .haveExactly(1, event(
                test("throwingTest"),
                finishedWithFailure(Condition([](const auto& it) { return dynamic_cast<const RuntimeException*>(&it) != nullptr; }, "is RuntimeException"))
            ));
        for (const auto& expected_part : std::vector<std::string>{"suspendForever", "invokeSuspend", "BlockingCoroutine{Active}"}) {
            assertEquals(expected_part, true, captured_out.str().find(expected_part) != std::string::npos);
        }
        for (const auto& non_expected_part : std::vector<std::string>{"delay", "throwingTest"}) {
            assertEquals(non_expected_part, false, captured_out.str().find(non_expected_part) != std::string::npos);
        }
        assertEquals(captured_out.str(), 1, count_dumps(captured_out));
    }
};

template<typename T>
ListAssert<Event> events_for_selector(DiscoverySelector selector, T& captured_out) {
    std::ostream* system_out = &std::cout;
    std::ostream* system_err = &std::cerr;
    try {
        // TODO: Redirect System.out and System.err
        // System.setOut(PrintStream(capturedOut))
        // System.setErr(PrintStream(capturedOut))
        return EngineTestKit::engine("junit-jupiter")
            .selectors(selector)
            .execute()
            .testEvents()
            .assertThatEvents();
    } catch (...) {
        // TODO: Restore System.out and System.err
        // System.setOut(systemOut)
        // System.setErr(systemErr)
        throw;
    }
}

ListAssert<Event> test_finished_successfully(ListAssert<Event> events, const std::string& test_name) {
    return events.haveExactly(1, event(
        test(test_name),
        finishedSuccessfully()
    ));
}

ListAssert<Event> test_timed_out(ListAssert<Event> events, const std::string& test_name, long after) {
    return events.haveExactly(1, event(
        test(test_name),
        finishedWithFailure(Condition([after](const auto& it) {
            const CoroutinesTimeoutException* ex = dynamic_cast<const CoroutinesTimeoutException*>(&it);
            return ex != nullptr && ex->timeout_ms == after;
        }, "is CoroutinesTimeoutException(" + std::to_string(after) + ")"))
    ));
}

/** Counts the number of occurrences of "Coroutines dump" in [capturedOut] */
template<typename T>
int count_dumps(const T& captured_out) {
    int result = 0;
    std::string out_str = captured_out.str();
    std::string header = "Coroutines dump";
    size_t i = 0;
    while (i < out_str.length() - header.length()) {
        if (out_str.substr(i, header.length()) == header) {
            result += 1;
            i += header.length();
        } else {
            i += 1;
        }
    }
    return result;
}

} // namespace junit5
} // namespace debug
} // namespace coroutines
} // namespace kotlinx
