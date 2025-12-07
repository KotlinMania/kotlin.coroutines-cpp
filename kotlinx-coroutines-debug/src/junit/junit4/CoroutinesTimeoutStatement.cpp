#include "kotlinx/coroutines/core_fwd.hpp"
// Original Kotlin package: kotlinx.coroutines.debug.junit4
// Line-by-line C++ transliteration from Kotlin
//
// TODO: JUnit4 Statement, Description - Java testing framework types
// TODO: TestTimedOutException - JUnit4 exception type
// TODO: TimeUnit - Java time unit, use C++ chrono
// TODO: visibility - translate to anonymous namespace or private
// TODO: runWithTimeoutDumpingCoroutines - implement from CoroutinesTimeoutImpl

#include <chrono>
#include <stdexcept>

// Forward declarations
namespace kotlinx {
namespace coroutines {
namespace debug {

// TODO: JUnit4 types
// class Statement;
// class Description;
// class TestTimedOutException;

namespace junit4 {

// TODO: class
class CoroutinesTimeoutStatement /* TODO: : Statement */ {
public:
    CoroutinesTimeoutStatement(
        /* TODO: Statement* */ void* test_statement,
        /* TODO: Description* */ void* test_description,
        long test_timeout_ms,
        bool cancel_on_timeout = false
    )
        : test_statement_(test_statement)
        , test_description_(test_description)
        , test_timeout_ms_(test_timeout_ms)
        , cancel_on_timeout_(cancel_on_timeout)
    {
    }

    // TODO: virtual auto evaluate()
    void evaluate() {
        try {
            // TODO: run_with_timeout_dumping_coroutines(
            //     test_description_.method_name,
            //     test_timeout_ms_,
            //     cancel_on_timeout_,
            //     []() { return TestTimedOutException(test_timeout_ms_, TimeUnit.MILLISECONDS); },
            //     [this]() { test_statement_->evaluate(); }
            // );
        } catch (...) {
            // TODO: DebugProbes.uninstall();
            throw;
        }
        // TODO: DebugProbes.uninstall();
    }

private:
    void* test_statement_;     // TODO: Statement*
    void* test_description_;   // TODO: Description*
    long test_timeout_ms_;
    bool cancel_on_timeout_;
};

} // namespace junit4
} // namespace debug
} // namespace coroutines
} // namespace kotlinx
