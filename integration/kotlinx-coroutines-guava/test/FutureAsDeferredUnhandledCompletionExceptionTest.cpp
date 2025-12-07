// Transliterated from: integration/kotlinx-coroutines-guava/test/FutureAsDeferredUnhandledCompletionExceptionTest.kt

// TODO: actually import kotlinx::coroutines::testing::
// import kotlinx.coroutines.testing.*
// import com.google.common.util.concurrent.*
// import kotlinx.coroutines.*
// import org.junit.*
// import org.junit.Test
// import kotlin.test.*

namespace kotlinx {
namespace coroutines {
namespace guava {

class FutureAsDeferredUnhandledCompletionExceptionTest : public TestBase {
private:
    // This is a separate test in order to avoid interference with uncaught exception handlers in other tests
    Thread::UncaughtExceptionHandler* exception_handler_;
    Throwable* caught_exception_;

public:
    // @Before
    void set_up() {
        exception_handler_ = Thread::get_default_uncaught_exception_handler();
        Thread::set_default_uncaught_exception_handler([this](Thread*, Throwable& e) {
            caught_exception_ = &e;
        });
    }

    // @After
    void tear_down() {
        Thread::set_default_uncaught_exception_handler(exception_handler_);
    }

    // @Test
    void test_lost_exception_on_success() {
        // TODO: implement coroutine suspension
        run_test([]() {
            auto future = SettableFuture::create<int>();
            auto deferred = future.as_deferred();
            deferred.invoke_on_completion([]() {
                throw TestException();
            });
            future.set(1);
            assert_true(dynamic_cast<CompletionHandlerException*>(caught_exception_) != nullptr &&
                       dynamic_cast<TestException*>(caught_exception_->cause()) != nullptr);
        });
    }

    // @Test
    void test_lost_exception_on_failure() {
        // TODO: implement coroutine suspension
        run_test([]() {
            auto future = SettableFuture::create<int>();
            auto deferred = future.as_deferred();
            deferred.invoke_on_completion([]() {
                throw TestException();
            });
            future.set_exception(TestException2());
            assert_true(dynamic_cast<CompletionHandlerException*>(caught_exception_) != nullptr &&
                       dynamic_cast<TestException*>(caught_exception_->cause()) != nullptr);
        });
    }
};

} // namespace guava
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement TestBase base class
// 2. Implement runTest coroutine test wrapper
// 3. Implement SettableFuture
// 4. Implement asDeferred conversion
// 5. Implement invokeOnCompletion
// 6. Implement Thread::setDefaultUncaughtExceptionHandler
// 7. Handle Before/After test lifecycle
// 8. Implement CompletionHandlerException
