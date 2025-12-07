// Original file: kotlinx-coroutines-debug/test/Example.kt
// TODO: Convert imports to C++ includes
// TODO: Implement suspend functions
// TODO: Implement coroutineScope, async, Deferred
// TODO: Implement delay
// TODO: Implement runBlocking
// TODO: Implement DebugProbes API

// suspend
std::string compute_value();

// suspend
std::string combine_results(void* one, void* two);

// suspend
std::string compute_one();

// suspend
std::string compute_two();

void main_example();

// suspend
std::string compute_value() {
    // TODO: return coroutineScope {
    //     auto one = async { computeOne() }
    //     auto two = async { computeTwo() }
    //     combineResults(one, two)
    // }
    return "";
}

// suspend
std::string combine_results(void* one, void* two) {
    // TODO: return one.await() + two.await()
    return "";
}

// suspend
std::string compute_one() {
    // TODO: delay(5000)
    return "4";
}

// suspend
std::string compute_two() {
    // TODO: delay(5000)
    return "2";
}

void main_example() {
    // TODO: runBlocking {
    //     DebugProbes.install()
    //     auto deferred = async { computeValue() }
    //     // Delay for some time
    //     delay(1000)
    //     // Dump running coroutines
    //     DebugProbes.dumpCoroutines()
    //     println("\nDumping only deferred")
    //     DebugProbes.printJob(deferred)
    // }
}
