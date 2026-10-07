@file:OptIn(kotlin.experimental.ExperimentalNativeApi::class, kotlin.native.internal.InternalForKotlinNative::class, kotlin.native.runtime.NativeRuntimeApi::class)
@file:Suppress("INVISIBLE_REFERENCE", "INVISIBLE_MEMBER")
// Test fixture for the internal Native runtime ABI; not a library implementation.
import kotlin.native.internal.ExportForCppRuntime
import kotlin.native.internal.GCUnsafeCall
import kotlin.native.ref.WeakReference
import kotlin.native.runtime.GC
import kotlin.native.identityHashCode

private class Payload(val index: Int)
private val observed = arrayOfNulls<WeakReference<Payload>>(4)

@ExportForCppRuntime("NativeReference_create")
fun createPayload(index: Int): Any {
    val value = Payload(index)
    observed[index] = WeakReference(value)
    return value
}
@ExportForCppRuntime("NativeReference_collect")
fun collectPayloads() { GC.collect() }
@ExportForCppRuntime("NativeReference_alive")
fun isPayloadAlive(index: Int): Boolean = observed[index]?.get() != null
@ExportForCppRuntime("NativeReference_hash")
fun hashPayload(value: Any?): Int = value.identityHashCode()
@GCUnsafeCall("NativeReference_cppHashes")
external fun cppHashContract(): Int
@GCUnsafeCall("NativeReference_cppRoots")
external fun cppRootContract(): Int
@GCUnsafeCall("NativeReference_cppReturn")
external fun cppReturnContract(index: Int): Any

fun main() {
    check(cppRootContract() == 10)
    println("native-roots=10")
    check(cppHashContract() == 10)
    println("native-hashes=10")
    val returned = cppReturnContract(3)
    GC.collect()
    check(returned is Payload && returned.index == 3)
    check(isPayloadAlive(3))
    println("native-return-slot=1")
}
