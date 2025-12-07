// Original file: kotlinx-coroutines-debug/test/TestRuleExample.kt
// TODO: Convert imports to C++ includes
// TODO: Convert @Ignore annotation
// TODO: Implement @JvmField, @Rule annotations
// TODO: Implement CoroutinesTimeout rule
// TODO: Implement runBlocking and coroutine builders

// @Ignore // do not run it on CI
class TestRuleExample {
public:
    // @JvmField
    // @Rule
    // TODO: CoroutinesTimeout timeout = CoroutinesTimeout.seconds(1)

private:
    // suspend
    void some_function_deep_in_the_stack() {
        // TODO: withContext(Dispatchers.IO) {
        //     delay(Long.MAX_VALUE)
        //     println("This line is never executed")
        // }
        //
        // println("This line is never executed as well")
    }

public:
    // @Test
    void hanging_test() {
        // TODO: runBlocking {
        //     auto job = launch {
        //         someFunctionDeepInTheStack()
        //     }
        //
        //     println("Doing some work...")
        //     job.join()
        // }
    }

    // @Test
    void successful_test() {
        // TODO: runBlocking {
        //     launch {
        //         delay(10)
        //     }.join()
        // }
    }
};
