// Transliterated from: reactive/kotlinx-coroutines-jdk9/test/PublisherBackpressureTest.cpp

// TODO: #include equivalent for kotlinx.coroutines.testing.*
// TODO: #include equivalent for kotlinx.coroutines.*
// TODO: #include equivalent for org.junit.*
// TODO: #include equivalent for java.util.concurrent.Flow as JFlow

namespace kotlinx {
namespace coroutines {
namespace jdk9 {

class PublisherBackpressureTest : public TestBase {
public:
    // @Test
    void test_cancel_while_bp_suspended() /* = runBlocking */ {
        // TODO: implement coroutine suspension
        /*
        expect(1)
        val observable = flowPublish(currentDispatcher()) {
            expect(5)
            send("A") // will not suspend, because an item was requested
            expect(7)
            send("B") // second requested item
            expect(9)
            try {
                send("C") // will suspend (no more requested)
            } finally {
                expect(12)
            }
            expectUnreached()
        }
        expect(2)
        var sub: JFlow.Subscription? = null
        observable.subscribe(object : JFlow.Subscriber<String> {
            override fun onSubscribe(s: JFlow.Subscription) {
                sub = s
                expect(3)
                s.request(2) // request two items
            }

            override fun onNext(t: String) {
                when (t) {
                    "A" -> expect(6)
                    "B" -> expect(8)
                    else -> error("Should not happen")
                }
            }

            override fun onComplete() {
                expectUnreached()
            }

            override fun onError(e: Throwable) {
                expectUnreached()
            }
        })
        expect(4)
        yield() // yield to observable coroutine
        expect(10)
        sub!!.cancel() // now unsubscribe -- shall cancel coroutine (& do not signal)
        expect(11)
        yield() // shall perform finally in coroutine
        finish(13)
        */
    }
};

} // namespace jdk9
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement runBlocking coroutine builder
// 2. Implement currentDispatcher() function
// 3. Implement flowPublish builder
// 4. Implement send() suspending function with backpressure
// 5. Implement JFlow.Subscription interface
// 6. Implement subscribe() method
// 7. Implement object expression for anonymous Subscriber
// 8. Implement nullable pointer handling (JFlow.Subscription?)
// 9. Implement !! null assertion operator
// 10. Implement request() method for backpressure control
// 11. Implement when expression for string matching
// 12. Implement error() function
// 13. Implement yield() suspending function
// 14. Implement cancel() method
// 15. Handle suspended send() with backpressure semantics
