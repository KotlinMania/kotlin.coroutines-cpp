// Transliterated from: test-utils/js/src/TestBase.kt
// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <test_framework.hpp>
// TODO: #include <emscripten.h> // For JS interop

namespace kotlinx {
namespace coroutines {
namespace testing {

// actual typealias NoJs = Ignore
using NoJs = Ignore;

const bool kVerbose = false;

const bool kIsStressTest = false;
const int kStressTestMultiplier = 1;
const int kStressTestMultiplierSqrt = 1;

// @JsName("Promise")
// external class MyPromise {
//     fun then(onFulfilled: ((Unit) -> Unit), onRejected: ((Throwable) -> Unit)): MyPromise
//     fun then(onFulfilled: ((Unit) -> Unit)): MyPromise
// }
// TODO: Implement JavaScript Promise interop
class MyPromise {
public:
    MyPromise then(std::function<void(void*)> on_fulfilled, std::function<void(Throwable*)> on_rejected);
    MyPromise then(std::function<void(void*)> on_fulfilled);
};

// Always a `Promise<Unit>`
using TestResult = MyPromise;

void last_resort_report_exception(Throwable* error) {
    std::cout << error << std::endl;
    // TODO: console.log(error) equivalent
}

class TestBase : public OrderedExecutionTestBase, public ErrorCatching {
private:
    ErrorCatching::Impl error_catching_impl;
    void* last_test_promise = nullptr; // TODO: Promise<void*>* type

public:
    TestBase() : error_catching_impl() {}

    TestBase(ErrorCatching::Impl error_catching)
        : error_catching_impl(error_catching) {}

    void println(const std::string& message) {
        std::cout << message << std::endl;
    }

    TestResult run_test(
        std::function<bool(Throwable*)> expected = nullptr,
        std::vector<std::function<bool(Throwable*)>> unhandled = {},
        std::function<void(CoroutineScope&)> block = nullptr // TODO: implement coroutine suspension
    ) {
        int ex_count = 0;
        Throwable* ex = nullptr;

        // This is an additional sanity check against `runTest` mis-usage on JS.
        // The only way to write an async test on JS is to return Promise from the test function.
        // _Just_ launching promise and returning `Unit` won't suffice as the underlying test framework
        // won't be able to detect an asynchronous failure in a timely manner.
        // We cannot detect such situations, but we can detect the most common erroneous pattern
        // in our code base, an attempt to use multiple `runTest` in the same `@Test` method,
        // which typically is a premise to the same error:
        // ```
        // @Test
        // fun incorrectTestForJs() { // <- promise is not returned
        //     for (parameter in parameters) {
        //         runTest {
        //             runTestForParameter(parameter)
        //         }
        //     }
        // }
        // ```
        if (last_test_promise != nullptr) {
            throw std::runtime_error("Attempt to run multiple asynchronous test within one @Test method");
        }

        // TODO: Implement GlobalScope.promise with CoroutineExceptionHandler
        // auto result = GlobalScope::promise(
        //     block,
        //     CoroutineExceptionHandler([&](CoroutineContext& ctx, Throwable& e) {
        //         if (dynamic_cast<CancellationException*>(&e)) return; // are ignored
        //         ex_count++;
        //         if (ex_count > unhandled.size()) {
        //             error(error_catching_impl, "Too many unhandled exceptions " + std::to_string(ex_count) +
        //                   ", expected " + std::to_string(unhandled.size()) + ", got: " + std::string(e.what()), &e);
        //         }
        //         if (!unhandled[ex_count - 1](&e)) {
        //             error(error_catching_impl, "Unhandled exception was unexpected: " + std::string(e.what()), &e);
        //         }
        //     })
        // ).catch_exception([&](Throwable& e) {
        //     ex = &e;
        //     if (expected != nullptr) {
        //         if (!expected(&e)) {
        //             std::cout << e.what() << std::endl;
        //             error(error_catching_impl, "Unexpected exception " + std::string(e.what()), &e);
        //         }
        //     } else {
        //         throw e;
        //     }
        // }).finally_handler([&]() {
        //     if (ex == nullptr && expected != nullptr) {
        //         throw std::runtime_error("Exception was expected but none produced");
        //     }
        //     if (ex_count < unhandled.size()) {
        //         error(error_catching_impl, "Too few unhandled exceptions " + std::to_string(ex_count) +
        //               ", expected " + std::to_string(unhandled.size()));
        //     }
        //     error_catching_impl.close();
        //     check_finish_call();
        // });

        // last_test_promise = result;
        // @Suppress("CAST_NEVER_SUCCEEDS")
        // return result as MyPromise;

        return MyPromise(); // TODO: Replace with actual implementation
    }

    bool has_error() override {
        return error_catching_impl.has_error();
    }

    void report_error(Throwable* error) override {
        error_catching_impl.report_error(error);
    }
};

const bool kIsNative = false;
const bool kIsBoundByJsTestTimeout = true;
const bool kIsJavaAndWindows = false;
const bool kUsesSharedEventLoop = false;

} // namespace testing
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement suspend function mechanics (coroutine suspension)
// 2. Implement JavaScript Promise interop (MyPromise class)
// 3. Implement GlobalScope::promise function
// 4. Implement CoroutineExceptionHandler
// 5. Implement promise.catch() and promise.finally() methods
// 6. Implement CancellationException detection
// 7. Implement Ignore annotation/type
// 8. Add proper includes for all dependencies
// 9. Implement console.log equivalent for error reporting
// 10. Handle memory management for promise and exception objects
