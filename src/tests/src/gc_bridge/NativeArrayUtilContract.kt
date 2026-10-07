@file:OptIn(kotlin.experimental.ExperimentalNativeApi::class, kotlin.native.runtime.NativeRuntimeApi::class, kotlin.native.internal.InternalForKotlinNative::class)
@file:Suppress("INVISIBLE_REFERENCE", "INVISIBLE_MEMBER")
// Actual Native standard-library execution. Function bodies are checked against
// the pinned source; installed Native 2.4.10 is a host verification toolchain.
import kotlin.collections.arrayOfUninitializedElements
import kotlin.collections.arrayCopy
import kotlin.collections.arrayFill
import kotlin.collections.copyOfUninitializedElements
import kotlin.collections.copyOfNulls
import kotlin.collections.resetAt
import kotlin.collections.resetRange
import kotlin.native.ref.WeakReference
import kotlin.native.runtime.GC
import kotlin.native.internal.GCUnsafeCall

@GCUnsafeCall("NativeArray_cppResetAt")
external fun cppResetAt(array: Array<*>, index: Int)
@GCUnsafeCall("NativeArray_cppResetRange")
external fun cppResetRange(array: Array<*>, from: Int, to: Int)
@GCUnsafeCall("NativeArray_cppFill")
external fun cppFill(array: Array<*>, from: Int, to: Int, value: Any?)
@GCUnsafeCall("NativeArray_cppCopy")
external fun cppCopy(array: Array<*>, from: Int, destination: Array<*>, to: Int, count: Int)
@GCUnsafeCall("NativeArray_cppGet")
external fun cppGet(array: Array<*>, index: Int): Any?

private fun numbers(): Array<Int?> = Array(4) { it + 10 }
private fun contents(array: Array<Int?>): String = array.joinToString(",") { it?.toString() ?: "null-or-uninitialized" }
private fun nullContents(array: Array<Int?>): String = array.joinToString(",") { it?.toString() ?: "null" }
private fun observe(messages: Boolean = false, operation: () -> String): String = try { operation() }
    catch (error: IndexOutOfBoundsException) { "IndexOutOfBoundsException" + if (messages) ":${error.message ?: ""}" else "" }
    catch (error: IllegalArgumentException) { "IllegalArgumentException" + if (messages) ":${error.message ?: ""}" else "" }

private fun referenceCopy(): Pair<Array<Array<Int>?>, WeakReference<Array<Int>>> {
    val source = arrayOfUninitializedElements<Array<Int>?>(1)
    source[0] = arrayOf(31)
    val observed = WeakReference(source[0]!!)
    val copied = source.copyOfUninitializedElements(1)
    check(copied[0] === source[0])
    source.resetAt(0)
    return copied to observed
}
private fun isAlive(observed: WeakReference<Array<Int>>): Boolean = observed.get() != null
private fun nullReferenceCopy(): Pair<Array<Array<Int>?>, WeakReference<Array<Int>>> {
    val retained = arrayOfNulls<Array<Int>>(1)
    retained[0] = arrayOf(31)
    val observed = WeakReference(retained[0]!!)
    val grown = retained.copyOf(5)
    check(grown[0] === retained[0] && (1 until 5).all { grown[it] == null })
    retained[0] = null
    return grown to observed
}
private fun nativeReturnAfterReset(): Any {
    val source = arrayOf(arrayOf(31))
    val returned = cppGet(source, 0)
    cppResetRange(source, 0, 1)
    return returned!!
}

fun main() {
    for (size in -1..5) println("allocate:$size=" + observe(true) {
        val array = arrayOfUninitializedElements<Int?>(size)
        "${array.size}:${contents(array)}"
    })
    for (self in listOf(false, true)) for (from in -1..5) for (to in -1..5) for (count in -1..5) {
        val source = numbers()
        val destination = if (self) source else Array<Int?>(4) { 90 }
        val beforeSource = contents(source)
        val beforeDestination = contents(destination)
        val result = observe {
            arrayCopy(source as Array<Any?>, from, destination as Array<Any?>, to, count)
            "${contents(source)}|${contents(destination)}"
        }
        if (result == "IndexOutOfBoundsException") check(contents(source) == beforeSource && contents(destination) == beforeDestination)
        println("copy:$self:$from:$to:$count=$result")
    }
    for (from in -1..5) for (to in -1..5) {
        var array = numbers()
        println("fill:$from:$to=" + observe(true) { arrayFill(array, from, to, 71); contents(array) })
        array = numbers()
        println("reset-range:$from:$to=" + observe(true) { array.resetRange(from, to); contents(array) })
        println("slice:$from:$to=" + observe { contents(numbers().copyOfUninitializedElements(from, to)) })
    }
    for (index in -1..4) {
        val array = numbers()
        println("reset-at:$index=" + observe { array.resetAt(index); contents(array) })
    }
    for (size in -1..7) println("resize:$size=" + observe { contents(numbers().copyOfUninitializedElements(size)) })
    val source = numbers()
    source.resetAt(1)
    val copied = source.copyOfUninitializedElements(6)
    source[0] = 99
    check(copied[0] == 10 && copied[1] == null && copied[4] == null)
    println("copy-independent=${contents(copied)}")
    val destination = numbers()
    val returned = numbers().copyInto(destination)
    check(returned === destination)
    println("default-copy=${contents(destination)}:${returned === destination}")
    println("overflow-copy=" + observe { source.copyInto(destination, 0, -1, Int.MAX_VALUE); contents(destination) })
    println("overflow-slice=" + observe(true) { contents(source.copyOfUninitializedElements(-1, Int.MAX_VALUE)) })
    // The C++ side additionally uses the genuine translated compiler Name,
    // whose constructor cannot be defaulted. Native validates its slot operation.
    val values = arrayOfUninitializedElements<Array<Int>>(1)
    values[0] = arrayOf(31)
    val valueCopy = values.copyOfUninitializedElements(3)
    values.resetAt(0)
    check(valueCopy[0][0] == 31)
    println("non-default-constructed-value=true")
    val (references, observed) = referenceCopy()
    GC.collect()
    check(isAlive(observed))
    println("copy-retains-reference=true")
    references.resetRange(0, 1)
    GC.collect()
    check(!isAlive(observed))
    println("reset-releases-reference=true")
    for (size in -1..5) println("allocate-null:$size=" + observe(true) {
        val array = arrayOfNulls<Int>(size)
        "${array.size}:${nullContents(array)}"
    })
    for (from in -1..5) for (to in -1..5) println("slice-null:$from:$to=" + observe(true) {
        nullContents(numbers().copyOfNulls(from, to))
    })
    for (size in -1..7) println("resize-null:$size=" + observe(true) {
        nullContents(numbers().copyOf(size))
    })
    println("overflow-null-slice=" + observe(true) { nullContents(numbers().copyOfNulls(-1, Int.MAX_VALUE)) })
    val nonnullable = Array(4) { it + 10 }
    val nullableCopy = nonnullable.copyOf(6)
    nonnullable[0] = 99
    check(nullableCopy[0] == 10)
    println("nonnull-to-nullable=${nullContents(nullableCopy)}")
    val (grown, retainedObserver) = nullReferenceCopy()
    GC.collect()
    check(isAlive(retainedObserver))
    grown[0] = null
    GC.collect()
    check(!isAlive(retainedObserver))
    println("null-growth-reference=identity,readable-null-tail,retain,release")
    val nativeSource = Array(4) { arrayOf(it) }
    val nativeDestination = arrayOfNulls<Array<Int>>(4)
    cppCopy(nativeSource, 0, nativeDestination, 0, 4)
    check(nativeDestination.indices.all { nativeDestination[it] === nativeSource[it] })
    cppCopy(nativeDestination, 0, nativeDestination, 1, 3)
    check(nativeDestination[0] === nativeSource[0] && nativeDestination[1] === nativeSource[0] &&
          nativeDestination[2] === nativeSource[1] && nativeDestination[3] === nativeSource[2])
    cppCopy(nativeDestination, 1, nativeDestination, 0, 3)
    check(nativeDestination[0] === nativeSource[0] && nativeDestination[1] === nativeSource[1] &&
          nativeDestination[2] === nativeSource[2] && nativeDestination[3] === nativeSource[2])
    cppResetAt(nativeDestination, 1)
    cppResetRange(nativeDestination, 2, 4)
    check(nativeDestination[0] === nativeSource[0] && nativeDestination.drop(1).all { it == null })
    cppFill(nativeDestination, 1, 4, nativeSource[3])
    check(nativeDestination.drop(1).all { it === nativeSource[3] })
    val nativeReturned = cppGet(nativeDestination, 2)
    cppResetRange(nativeDestination, 0, 4)
    GC.collect()
    check(nativeReturned === nativeSource[3])
    val independentReturn = nativeReturnAfterReset()
    GC.collect()
    check((independentReturn as Array<*>)[0] == 31)
    println("native-array-handoff=copy,overlap,reset,fill,rooted-get")
    val (nativeReferences, nativeObserved) = referenceCopy()
    cppResetRange(nativeReferences, 0, 1)
    GC.collect()
    check(!isAlive(nativeObserved))
    println("native-cpp-reset-releases-reference=true")
}
