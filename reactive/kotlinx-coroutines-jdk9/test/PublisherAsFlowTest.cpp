// Transliterated from: reactive/kotlinx-coroutines-jdk9/test/PublisherAsFlowTest.cpp

// TODO: #include equivalent for kotlinx.coroutines.testing.*
// TODO: #include equivalent for kotlinx.coroutines.*
// TODO: #include equivalent for kotlinx.coroutines.channels.*
// TODO: #include equivalent for kotlinx.coroutines.flow.*
// TODO: #include equivalent for kotlinx.coroutines.testing.flow.*
// TODO: #include equivalent for kotlin.test.*

namespace kotlinx {
namespace coroutines {
namespace jdk9 {

class PublisherAsFlowTest : public TestBase {
public:
    // @Test
    void test_cancellation() /* = runTest */ {
        // TODO: implement coroutine suspension
        /*
        var onNext = 0
        var onCancelled = 0
        var onError = 0

        val publisher = flowPublish(currentDispatcher()) {
            coroutineContext[Job]?.invokeOnCompletion {
                if (it is CancellationException) ++onCancelled
            }

            repeat(100) {
                send(it)
            }
        }

        publisher.asFlow().launchIn(CoroutineScope(Dispatchers.Unconfined)) {
            onEach {
                ++onNext
                throw RuntimeException()
            }
            catch<Throwable> {
                ++onError
            }
        }.join()


        assertEquals(1, onNext)
        assertEquals(1, onError)
        assertEquals(1, onCancelled)
        */
    }

    // @Test
    void test_buffer_size1() /* = runTest */ {
        // TODO: implement coroutine suspension
        /*
        val publisher = flowPublish(currentDispatcher()) {
            expect(1)
            send(3)

            expect(2)
            send(5)

            expect(4)
            send(7)
            expect(6)
        }

        publisher.asFlow().buffer(1).collect {
            expect(it)
        }

        finish(8)
        */
    }

    // @Test
    void test_buffer_size_default() /* = runTest */ {
        // TODO: implement coroutine suspension
        /*
        val publisher = flowPublish(currentDispatcher()) {
            repeat(64) {
                send(it + 1)
                expect(it + 1)
            }
            assertFalse { trySend(-1).isSuccess }
        }

        publisher.asFlow().collect {
            expect(64 + it)
        }

        finish(129)
        */
    }

    // @Test
    void test_default_capacity_is_properly_overwritten() /* = runTest */ {
        // TODO: implement coroutine suspension
        /*
        val publisher = flowPublish(currentDispatcher()) {
            expect(1)
            send(3)
            expect(2)
            send(5)
            expect(4)
            send(7)
            expect(6)
        }

        publisher.asFlow().flowOn(wrapperDispatcher()).buffer(1).collect {
            expect(it)
        }

        finish(8)
        */
    }

    // @Test
    void test_buffer_size10() /* = runTest */ {
        // TODO: implement coroutine suspension
        /*
        val publisher = flowPublish(currentDispatcher()) {
            expect(1)
            send(5)

            expect(2)
            send(6)

            expect(3)
            send(7)
            expect(4)
        }

        publisher.asFlow().buffer(10).collect {
            expect(it)
        }

        finish(8)
        */
    }

    // @Test
    void test_conflated() /* = runTest */ {
        // TODO: implement coroutine suspension
        /*
        val publisher = flowPublish(currentDispatcher()) {
            for (i in 1..5) send(i)
        }
        val list = publisher.asFlow().conflate().toList()
        assertEquals(listOf(1, 5), list)
        */
    }

    // @Test
    void test_produce() /* = runTest */ {
        // TODO: implement coroutine suspension
        /*
        val flow = flowPublish(currentDispatcher()) { repeat(10) { send(it) } }.asFlow()
        check((0..9).toList(), flow.produceIn(this))
        check((0..9).toList(), flow.buffer(2).produceIn(this))
        check((0..9).toList(), flow.buffer(Channel.UNLIMITED).produceIn(this))
        check(listOf(0, 9), flow.conflate().produceIn(this))
        */
    }

private:
    void check(/* expected: List<Int>, channel: ReceiveChannel<Int> */ auto expected, auto channel) {
        // TODO: implement coroutine suspension
        /*
        val result = ArrayList<Int>(10)
        channel.consumeEach { result.add(it) }
        assertEquals(expected, result)
        */
    }

public:
    // @Test
    void test_produce_cancellation() /* = runTest */ {
        // TODO: implement coroutine suspension
        /*
        expect(1)
        // publisher is an async coroutine, so it overproduces to the channel, but still gets cancelled
        val flow = flowPublish(currentDispatcher()) {
            expect(3)
            repeat(10) { value ->
                when (value) {
                    in 0..6 -> send(value)
                    7 -> try {
                        send(value)
                    } catch (e: CancellationException) {
                        expect(5)
                        throw e
                    }
                    else -> expectUnreached()
                }
            }
        }.asFlow().buffer(1)
        assertFailsWith<TestException> {
            coroutineScope {
                expect(2)
                val channel = flow.produceIn(this)
                channel.consumeEach { value ->
                    when (value) {
                        in 0..4 -> {}
                        5 -> {
                            expect(4)
                            throw TestException()
                        }
                        else -> expectUnreached()
                    }
                }
            }
        }
        finish(6)
        */
    }
};

} // namespace jdk9
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement currentDispatcher() function
// 2. Implement coroutineContext access
// 3. Implement CoroutineContext[Job] indexing operator
// 4. Implement invokeOnCompletion() for Job
// 5. Implement repeat() function
// 6. Implement launchIn() extension
// 7. Implement CoroutineScope class
// 8. Implement onEach() flow operator
// 9. Implement catch<T>() flow operator
// 10. Implement buffer() flow operator
// 11. Implement assertFalse() function
// 12. Implement trySend() channel method
// 13. Implement wrapperDispatcher() test helper
// 14. Implement conflate() flow operator
// 15. Implement toList() flow terminal operator
// 16. Implement listOf() utility
// 17. Implement produceIn() flow operator
// 18. Implement Channel.UNLIMITED constant
// 19. Implement ReceiveChannel<T> interface
// 20. Implement consumeEach() extension
// 21. Implement ArrayList<T> class
// 22. Implement coroutineScope builder
// 23. Implement when expression with range patterns
