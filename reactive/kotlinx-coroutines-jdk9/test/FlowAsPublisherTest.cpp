// Transliterated from: reactive/kotlinx-coroutines-jdk9/test/FlowAsPublisherTest.cpp

// TODO: #include equivalent for kotlinx.coroutines.testing.*
// TODO: #include equivalent for kotlinx.coroutines.*
// TODO: #include equivalent for kotlinx.coroutines.flow.*
// TODO: #include equivalent for org.junit.Test
// TODO: #include equivalent for java.util.concurrent.Flow as JFlow
// TODO: #include equivalent for kotlin.test.*

namespace kotlinx {
namespace coroutines {
namespace jdk9 {

class FlowAsPublisherTest : public TestBase {
public:
    // @Test
    void test_error_on_cancellation_is_reported() {
        // TODO: implement coroutine suspension
        /*
        expect(1)
        flow {
            try {
                emit(2)
            } finally {
                expect(3)
                throw TestException()
            }
        }.asPublisher().subscribe(object : JFlow.Subscriber<Int> {
            private lateinit var subscription: JFlow.Subscription

            override fun onComplete() {
                expectUnreached()
            }

            override fun onSubscribe(s: JFlow.Subscription?) {
                subscription = s!!
                subscription.request(2)
            }

            override fun onNext(t: Int) {
                expect(t)
                subscription.cancel()
            }

            override fun onError(t: Throwable?) {
                assertIs<TestException>(t)
                expect(4)
            }
        })
        finish(5)
        */
    }

    // @Test
    void test_cancellation_is_not_reported() {
        // TODO: implement coroutine suspension
        /*
        expect(1)
        flow {
            emit(2)
        }.asPublisher().subscribe(object : JFlow.Subscriber<Int> {
            private lateinit var subscription: JFlow.Subscription

            override fun onComplete() {
                expectUnreached()
            }

            override fun onSubscribe(s: JFlow.Subscription?) {
                subscription = s!!
                subscription.request(2)
            }

            override fun onNext(t: Int) {
                expect(t)
                subscription.cancel()
            }

            override fun onError(t: Throwable?) {
                expectUnreached()
            }
        })
        finish(3)
        */
    }

    // @Test
    void test_flow_with_timeout() /* = runTest */ {
        // TODO: implement coroutine suspension
        /*
        val publisher = flow<Int> {
            expect(2)
            withTimeout(1) { delay(Long.MAX_VALUE) }
        }.asPublisher()
        try {
            expect(1)
            publisher.awaitFirstOrNull()
        } catch (e: CancellationException) {
            expect(3)
        }
        finish(4)
        */
    }
};

} // namespace jdk9
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement flow builder function
// 2. Implement emit() suspending function
// 3. Implement asPublisher() extension
// 4. Implement JFlow.Subscriber interface with onSubscribe, onNext, onComplete, onError
// 5. Implement JFlow.Subscription interface
// 6. Implement TestException class
// 7. Implement assertIs<T>() template function
// 8. Implement expectUnreached() test helper
// 9. Implement lateinit equivalent (possibly std::optional or pointer)
// 10. Implement withTimeout() suspending function
// 11. Implement delay() suspending function
// 12. Implement awaitFirstOrNull() suspending function
// 13. Implement object expressions for anonymous subscriber classes
// 14. Handle nullable types (Int? -> std::optional<int> or int*)
