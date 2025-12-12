// Original file: kotlinx-coroutines-debug/test/RecoveryExample.kt
// TODO: Convert @file:Suppress annotation
// TODO: Convert imports to C++ includes
// TODO: Implement object as singleton/namespace
// TODO: Implement CoroutineScope delegation
// TODO: Implement suspend functions

// @file:Suppress("PackageDirectoryMismatch")
namespace example {

// object PublicApiImplementation : CoroutineScope by CoroutineScope(CoroutineName("Example"))
namespace public_api_implementation {
    // TODO: Implement CoroutineScope delegation

    int do_work() {
        // TODO: error("Internal invariant failed")
        throw std::runtime_error("Internal invariant failed");
    }

    int asynchronous_work() {
        return do_work() + 1;
    }

    // public suspend
    void await_asynchronous_work_in_main_thread() {
        // TODO: auto task = async(Dispatchers.Default) {
        //     asynchronousWork()
        // }
        //
        // task.await()
    }
} // namespace public_api_implementation

// suspend
void main_recovery() {
    // Try to switch debug mode on and off to see the difference
    // TODO: PublicApiImplementation.awaitAsynchronousWorkInMainThread()
}

} // namespace example
