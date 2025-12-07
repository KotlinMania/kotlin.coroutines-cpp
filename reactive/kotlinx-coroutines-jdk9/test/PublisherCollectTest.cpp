// Transliterated from: reactive/kotlinx-coroutines-jdk9/test/PublisherCollectTest.cpp

// TODO: #include equivalent for kotlinx.coroutines.testing.*
// TODO: #include equivalent for kotlinx.coroutines.*
// TODO: #include equivalent for kotlinx.coroutines.reactive.*
// TODO: #include equivalent for org.junit.Test
// TODO: #include equivalent for org.reactivestreams.*
// TODO: #include equivalent for kotlin.test.*
// TODO: #include equivalent for java.util.concurrent.Flow as JFlow

namespace kotlinx {
namespace coroutines {
namespace jdk9 {

class PublisherCollectTest : public TestBase {
public:
    /** Tests the simple scenario where the publisher outputs a bounded stream of values to collect. */
    // @Test
    void test_collect() /* = runTest */ {
        // TODO: implement coroutine suspension
        /*
        val x = 100
        val xSum = x * (x + 1) / 2
        val publisher = JFlow.Publisher<Int> { subscriber ->
            var requested = 0L
            var lastOutput = 0
            subscriber.onSubscribe(object: JFlow.Subscription {

                override fun request(n: Long) {
                    requested += n
                    if (n <= 0) {
                        subscriber.onError(IllegalArgumentException())
                        return
                    }
                    while (lastOutput < x && lastOutput < requested) {
                        lastOutput += 1
                        subscriber.onNext(lastOutput)
                    }
                    if (lastOutput == x)
                        subscriber.onComplete()
                }

                override fun cancel() {
                    /** According to rule 3.5 of the
                     * [reactive spec](https://github.com/reactive-streams/reactive-streams-jvm/blob/v1.0.3/README.md#3.5),
                     * this method can be called by the subscriber at any point, so it's not an error if it's called
                     * in this scenario. */
                }

            })
        }
        var sum = 0
        publisher.collect {
            sum += it
        }
        assertEquals(xSum, sum)
        */
    }

    /** Tests the behavior of [collect] when the publisher raises an error. */
    // @Test
    void test_collect_throwing_publisher() /* = runTest */ {
        // TODO: implement coroutine suspension
        /*
        val errorString = "Too many elements requested"
        val x = 100
        val xSum = x * (x + 1) / 2
        val publisher = Publisher<Int> { subscriber ->
            var requested = 0L
            var lastOutput = 0
            subscriber.onSubscribe(object: Subscription {

                override fun request(n: Long) {
                    requested += n
                    if (n <= 0) {
                        subscriber.onError(IllegalArgumentException())
                        return
                    }
                    while (lastOutput < x && lastOutput < requested) {
                        lastOutput += 1
                        subscriber.onNext(lastOutput)
                    }
                    if (lastOutput == x)
                        subscriber.onError(IllegalArgumentException(errorString))
                }

                override fun cancel() {
                    /** See the comment for the corresponding part of [testCollect]. */
                }

            })
        }
        var sum = 0
        try {
            publisher.collect {
                sum += it
            }
        } catch (e: IllegalArgumentException) {
            assertEquals(errorString, e.message)
        }
        assertEquals(xSum, sum)
        */
    }

    /** Tests the behavior of [collect] when the action throws. */
    // @Test
    void test_collect_throwing_action() /* = runTest */ {
        // TODO: implement coroutine suspension
        /*
        val errorString = "Too many elements produced"
        val x = 100
        val xSum = x * (x + 1) / 2
        val publisher = Publisher<Int> { subscriber ->
            var requested = 0L
            var lastOutput = 0
            subscriber.onSubscribe(object: Subscription {

                override fun request(n: Long) {
                    requested += n
                    if (n <= 0) {
                        subscriber.onError(IllegalArgumentException())
                        return
                    }
                    while (lastOutput < x && lastOutput < requested) {
                        lastOutput += 1
                        subscriber.onNext(lastOutput)
                    }
                }

                override fun cancel() {
                    assertEquals(x, lastOutput)
                    expect(x + 2)
                }

            })
        }
        var sum = 0
        try {
            expect(1)
            var i = 1
            publisher.collect {
                sum += it
                i += 1
                expect(i)
                if (sum >= xSum) {
                    throw IllegalArgumentException(errorString)
                }
            }
        } catch (e: IllegalArgumentException) {
            expect(x + 3)
            assertEquals(errorString, e.message)
        }
        finish(x + 4)
        */
    }
};

} // namespace jdk9
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement JFlow.Publisher<T> lambda constructor
// 2. Implement JFlow.Subscriber<T> interface
// 3. Implement JFlow.Subscription object expression
// 4. Implement onSubscribe(), onNext(), onComplete(), onError() methods
// 5. Implement request() and cancel() methods for Subscription
// 6. Implement collect() suspending function
// 7. Implement Publisher<T> (reactive streams) vs JFlow.Publisher<T>
// 8. Implement Subscription (reactive streams) interface
// 9. Handle mutable captured variables in lambdas
// 10. Implement IllegalArgumentException with message
// 11. Implement exception.message property
// 12. Handle try-catch blocks with specific exception types
// 13. Implement while loop with complex conditions
// 14. Implement Long type for request counts
