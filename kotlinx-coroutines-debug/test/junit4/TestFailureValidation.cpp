// Original: kotlinx-coroutines-debug/test/junit4/TestFailureValidation.kt
// Transliterated from Kotlin to C++ - First pass syntax conversion
// TODO: Implement JUnit4 RuleChain equivalent
// TODO: Implement TestRule interface
// TODO: Convert DebugProbes static initialization
// TODO: Map Description and Statement JUnit types
// TODO: Convert PrintStream and ByteArrayOutputStream to C++ equivalents
// TODO: Implement Throwable reflection (::class, isInstance)
// TODO: Convert data class to C++ struct

namespace kotlinx {
namespace coroutines {
namespace debug {
namespace junit4 {

// TODO: import kotlinx.coroutines.debug.*
// TODO: import org.junit.rules.*
// TODO: import org.junit.runner.*
// TODO: import org.junit.runners.model.*
// TODO: import java.io.*
// TODO: import kotlin.test.*

RuleChain test_failure_validation(
    long timeout_ms,
    bool cancel_on_timeout,
    bool creation_stack_traces,
    const std::vector<TestResultSpec>& specs
) {
    std::map<std::string, TestResultSpec> specs_map;
    for (const auto& spec : specs) {
        specs_map[spec.test_name] = spec;
    }

    return RuleChain()
        .outerRule(TestFailureValidation(specs_map))
        .around(
            CoroutinesTimeout(
                timeout_ms,
                cancel_on_timeout,
                creation_stack_traces
            )
        );
}

/**
 * Rule that captures test result, serr and sout and validates it against provided [testsSpec]
 */
class TestFailureValidation : public TestRule {
private:
    std::map<std::string, TestResultSpec> tests_spec_;

public:
    static void static_init() {
        // TODO: Companion object init block
        DebugProbes::sanitizeStackTraces = false;
    }

    explicit TestFailureValidation(const std::map<std::string, TestResultSpec>& tests_spec)
        : tests_spec_(tests_spec) {}

    Statement apply(Statement base, Description description) override {
        return TestFailureStatement(base, description, *this);
    }

    class TestFailureStatement : public Statement {
    private:
        Statement test_;
        Description description_;
        TestFailureValidation& parent_;
        std::ostream* sout_;
        std::ostream* serr_;
        std::ostringstream captured_out_;

    public:
        TestFailureStatement(Statement test, Description description, TestFailureValidation& parent)
            : test_(test), description_(description), parent_(parent) {}

        void evaluate() override {
            try {
                replace_out();
                test_.evaluate();
            } catch (const std::exception& e) {
                validate_failure(e);
                reset_out();
                return;
            } catch (...) {
                reset_out();
                throw;
            }

            reset_out();
            validate_success(); // To avoid falling into catch
        }

    private:
        void validate_success() {
            auto it = parent_.tests_spec_.find(description_.method_name);
            if (it == parent_.tests_spec_.end()) {
                throw std::runtime_error("Test spec not found: " + description_.method_name);
            }
            const auto& spec = it->second;

            if (spec.error != nullptr) {
                throw std::runtime_error("Expected exception of type " + std::string(spec.error->name()) +
                                       ", but test successfully passed");
            }

            std::string captured = captured_out_.str();
            assertFalse(captured.find("Coroutines dump") != std::string::npos);
            assertTrue(captured.empty(), captured);
        }

        void validate_failure(const std::exception& e) {
            auto it = parent_.tests_spec_.find(description_.method_name);
            if (it == parent_.tests_spec_.end()) {
                throw std::runtime_error("Test spec not found: " + description_.method_name);
            }
            const auto& spec = it->second;

            if (spec.error == nullptr || !spec.error->__do_catch(&e, 0, 0)) {
                throw std::runtime_error("Unexpected failure, expected " +
                    std::string(spec.error ? spec.error->name() : "null") +
                    ", had " + typeid(e).name());
            }

            // TODO: Check if e is TestTimedOutException
            // if (dynamic_cast<const TestTimedOutException*>(&e) == nullptr) return;

            std::string captured = captured_out_.str();
            assertTrue(captured.find("Coroutines dump") != std::string::npos);
            for (const auto& part : spec.expected_out_parts) {
                assertTrue(captured.find(part) != std::string::npos,
                          "Expected " + part + " to be part of the\n" + captured);
            }

            for (const auto& part : spec.not_expected_out_parts) {
                assertFalse(captured.find(part) != std::string::npos,
                           "Expected " + part + " not to be part of the\n" + captured);
            }
        }

        void replace_out() {
            // TODO: Redirect std::cout and std::cerr
            sout_ = &std::cout;
            serr_ = &std::cerr;

            // System.setOut(PrintStream(capturedOut))
            // System.setErr(PrintStream(capturedOut))
        }

        void reset_out() {
            // TODO: Restore std::cout and std::cerr
            // System.setOut(sout)
            // System.setErr(serr)
        }
    };
};

struct TestResultSpec {
    std::string test_name;
    std::vector<std::string> expected_out_parts;
    std::vector<std::string> not_expected_out_parts;
    const std::type_info* error;

    TestResultSpec(const std::string& test_name,
                   const std::vector<std::string>& expected_out_parts = {},
                   const std::vector<std::string>& not_expected_out_parts = {},
                   const std::type_info* error = nullptr)
        : test_name(test_name),
          expected_out_parts(expected_out_parts),
          not_expected_out_parts(not_expected_out_parts),
          error(error) {}
};

} // namespace junit4
} // namespace debug
} // namespace coroutines
} // namespace kotlinx
