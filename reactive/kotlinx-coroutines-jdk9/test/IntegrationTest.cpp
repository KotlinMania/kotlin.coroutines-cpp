// Transliterated from: reactive/kotlinx-coroutines-jdk9/test/IntegrationTest.cpp

// TODO: #include equivalent for kotlinx.coroutines.testing.*
// TODO: #include equivalent for kotlinx.coroutines.*
// TODO: #include equivalent for org.junit.Test
// TODO: #include equivalent for kotlinx.coroutines.flow.flowOn
// TODO: #include equivalent for kotlinx.coroutines.testing.exceptions.*
// TODO: #include equivalent for org.junit.runner.*
// TODO: #include equivalent for org.junit.runners.*
// TODO: #include equivalent for java.util.concurrent.Flow as JFlow
// TODO: #include equivalent for kotlin.coroutines.*
// TODO: #include equivalent for kotlin.test.*

namespace kotlinx {
namespace coroutines {
namespace jdk9 {

// @RunWith(Parameterized::class)
class IntegrationTest : public TestBase {
private:
    Ctx ctx_;
    bool delay_;

public:
    enum class Ctx {
        kMain,        // { override fun invoke(context: CoroutineContext): CoroutineContext = context.minusKey(Job) }
        kDefault,     // { override fun invoke(context: CoroutineContext): CoroutineContext = Dispatchers.Default }
        kUnconfined   // { override fun invoke(context: CoroutineContext): CoroutineContext = Dispatchers.Unconfined }

        // abstract operator fun invoke(context: CoroutineContext): CoroutineContext
    };

    IntegrationTest(Ctx ctx, bool delay) : ctx_(ctx), delay_(delay) {}

    // companion object {
    //     @Parameterized.Parameters(name = "ctx={0}, delay={1}")
    //     @JvmStatic
    //     fun params(): Collection<Array<Any>> = Ctx.values().flatMap { ctx ->
    //         listOf(false, true).map { delay ->
    //             arrayOf(ctx, delay)
    //         }
    //     }
    // }

    // @Test
    void test_empty() /* : Unit = runBlocking */ {
        // TODO: implement coroutine suspension
        /*
        val pub = flowPublish<String>(ctx(coroutineContext)) {
            if (delay) delay(1)
            // does not send anything
        }
        assertFailsWith<NoSuchElementException> { pub.awaitFirst() }
        assertEquals("OK", pub.awaitFirstOrDefault("OK"))
        assertNull(pub.awaitFirstOrNull())
        assertEquals("ELSE", pub.awaitFirstOrElse { "ELSE" })
        assertFailsWith<NoSuchElementException> { pub.awaitLast() }
        assertFailsWith<NoSuchElementException> { pub.awaitSingle() }
        var cnt = 0
        pub.collect { cnt++ }
        assertEquals(0, cnt)
        */
    }

    // @Test
    void test_single() /* = runBlocking */ {
        // TODO: implement coroutine suspension
        /*
        val pub = flowPublish(ctx(coroutineContext)) {
            if (delay) delay(1)
            send("OK")
        }
        assertEquals("OK", pub.awaitFirst())
        assertEquals("OK", pub.awaitFirstOrDefault("!"))
        assertEquals("OK", pub.awaitFirstOrNull())
        assertEquals("OK", pub.awaitFirstOrElse { "ELSE" })
        assertEquals("OK", pub.awaitLast())
        assertEquals("OK", pub.awaitSingle())
        var cnt = 0
        pub.collect {
            assertEquals("OK", it)
            cnt++
        }
        assertEquals(1, cnt)
        */
    }

    // @Test
    void test_numbers() /* = runBlocking */ {
        // TODO: implement coroutine suspension
        /*
        val n = 100 * stressTestMultiplier
        val pub = flowPublish(ctx(coroutineContext)) {
            for (i in 1..n) {
                send(i)
                if (delay) delay(1)
            }
        }
        assertEquals(1, pub.awaitFirst())
        assertEquals(1, pub.awaitFirstOrDefault(0))
        assertEquals(n, pub.awaitLast())
        assertEquals(1, pub.awaitFirstOrNull())
        assertEquals(1, pub.awaitFirstOrElse { 0 })
        assertFailsWith<IllegalArgumentException> { pub.awaitSingle() }
        checkNumbers(n, pub)
        val flow = pub.asFlow()
        checkNumbers(n, flow.flowOn(ctx(coroutineContext)).asPublisher())
        */
    }

    // @Test
    void test_cancel_without_value() /* = runTest */ {
        // TODO: implement coroutine suspension
        /*
        val job = launch(Job(), start = CoroutineStart.UNDISPATCHED) {
            flowPublish<String> {
                hang {}
            }.awaitFirst()
        }

        job.cancel()
        job.join()
        */
    }

    // @Test
    void test_empty_single() /* = runTest(unhandled = listOf { e -> e is NoSuchElementException}) */ {
        // TODO: implement coroutine suspension
        /*
        expect(1)
        val job = launch(Job(), start = CoroutineStart.UNDISPATCHED) {
            flowPublish<String> {
                yield()
                expect(2)
                // Nothing to emit
            }.awaitFirst()
        }

        job.join()
        finish(3)
        */
    }

private:
    void check_numbers(int n, /* JFlow.Publisher<Int> */ auto pub) {
        // TODO: implement coroutine suspension
        /*
        var last = 0
        pub.collect {
            assertEquals(++last, it)
        }
        assertEquals(n, last)
        */
    }
};

} // namespace jdk9
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement parameterized test framework (@RunWith(Parameterized::class))
// 2. Implement Ctx enum with function operator overload
// 3. Implement CoroutineContext.minusKey() method
// 4. Implement Dispatchers.Default and Dispatchers.Unconfined
// 5. Implement companion object equivalent (static methods)
// 6. Implement test parameter generation
// 7. Implement runBlocking coroutine builder
// 8. Implement flowPublish<T>() builder
// 9. Implement all await* functions
// 10. Implement assertFailsWith<T>() template function
// 11. Implement assertEquals() template function
// 12. Implement assertNull() function
// 13. Implement stressTestMultiplier constant
// 14. Implement asFlow() extension
// 15. Implement flowOn() operator
// 16. Implement asPublisher() extension
// 17. Implement launch() with Job parameter
// 18. Implement hang() test helper
// 19. Implement yield() suspending function
// 20. Implement runTest with unhandled exception handler
