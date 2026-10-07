@file:OptIn(kotlin.experimental.ExperimentalNativeApi::class, kotlin.native.internal.InternalForKotlinNative::class)
@file:Suppress("INVISIBLE_REFERENCE", "INVISIBLE_MEMBER")
// Execute the actual Native standard library. macOS execution is a host proof;
// it does not establish the pinned compiler/runtime or bare-metal target ABI.
import kotlin.collections.arrayCopy
import kotlin.collections.copyOfUninitializedElements
import kotlin.native.internal.GCUnsafeCall

@GCUnsafeCall("NativeIntArray_cppGet")
external fun cppIntGet(array: IntArray, index: Int): Int
@GCUnsafeCall("NativeIntArray_cppSet")
external fun cppIntSet(array: IntArray, index: Int, value: Int)
@GCUnsafeCall("NativeIntArray_cppLength")
external fun cppIntLength(array: IntArray): Int
@GCUnsafeCall("NativeIntArray_cppFill")
external fun cppIntFill(array: IntArray, from: Int, to: Int, value: Int)
@GCUnsafeCall("NativeIntArray_cppCopy")
external fun cppIntCopy(array: IntArray, from: Int, destination: IntArray, to: Int, count: Int)
private fun numbers() = IntArray(4) { it + 10 }
private fun contents(array: IntArray) = array.joinToString(",")
private fun observe(messages: Boolean = false, operation: () -> String): String = try { operation() }
    catch(error: IndexOutOfBoundsException) { "IndexOutOfBoundsException" + if(messages) ":${error.message ?: ""}" else "" }
    catch(error: IllegalArgumentException) { "IllegalArgumentException" + if(messages) ":${error.message ?: ""}" else "" }
    catch(error: NoSuchElementException) { "NoSuchElementException:${error.message ?: ""}" }
private fun emit(key: String, result: String) = println("$key=$result")
fun main() {
    for(size in 0..5) emit("zero:$size", contents(IntArray(size)))
    for(self in listOf(false,true)) for(from in -1..5) for(to in -1..5) for(count in -1..5) {
        val source=numbers(); val destination=if(self) source else IntArray(4) {90}
        val beforeSource=contents(source); val beforeDestination=contents(destination)
        val result=observe { arrayCopy(source,from,destination,to,count); "${contents(source)}|${contents(destination)}" }
        if(result=="IndexOutOfBoundsException") check(contents(source)==beforeSource && contents(destination)==beforeDestination)
        emit("copy:$self:$from:$to:$count",result)
    }
    for(from in -1..5) for(to in -1..5) {
        val a=numbers(); emit("fill:$from:$to",observe(true) {a.fill(71,from,to);contents(a)})
        emit("range:$from:$to",observe(true) {contents(numbers().copyOfRange(from,to))})
        emit("slice:$from:$to",observe {contents(numbers().copyOfUninitializedElements(from,to))})
        emit("range-check:$from:$to",observe(true) {AbstractList.checkRangeIndexes(from,to,4);"ok"})
        emit("bounds-check:$from:$to",observe(true) {AbstractList.checkBoundsIndexes(from,to,4);"ok"})
    }
    for(i in -1..5) {
        emit("get:$i",observe {numbers()[i].toString()})
        val a=numbers(); emit("set:$i",observe {a[i]=73;contents(a)})
        emit("element-check:$i",observe(true) {AbstractList.checkElementIndex(i,4);"ok"})
        emit("position-check:$i",observe(true) {AbstractList.checkPositionIndex(i,4);"ok"})
    }
    for(size in -1..7) {
        emit("resize:$size",observe {contents(numbers().copyOf(size))})
        emit("resize-uninitialized:$size",observe {contents(numbers().copyOfUninitializedElements(size))})
    }
    val order=mutableListOf<Int>(); val a=IntArray(3) {order.add(it);it+10}
    check(order==listOf(0,1,2)); emit("init-order",order.joinToString(","))
    val iterator=a.iterator(); val widened: Iterator<Any> = iterator
    check(widened===iterator); emit("iterator-same-object","true")
    emit("iterator-first",iterator.nextInt().toString()); a[1]=77
    emit("iterator-widened-second",widened.next().toString());emit("iterator-third",iterator.next().toString())
    check(!iterator.hasNext())
    emit("iterator-exhaustion",observe {iterator.nextInt().toString()})
    emit("iterator-exhaustion-again",observe {widened.next().toString()})
    val retained=IntArray(1){31}.iterator();emit("iterator-retained",retained.nextInt().toString())
    val empty=IntArray(0).iterator();check(!empty.hasNext());emit("iterator-empty",observe {empty.nextInt().toString()})
    val original=numbers();val copied=original.copyOf();original[0]=99;check(copied[0]==10);emit("copy-independent",contents(copied))
    val dest=numbers();val returned=numbers().copyInto(dest);check(returned===dest);emit("default-copy","${contents(dest)}:true")
    dest.fill(72);emit("default-fill",contents(dest));dest.fill(73,2);emit("default-fill-from",contents(dest))
    emit("overflow-copy",observe {original.copyInto(dest,0,-1,Int.MAX_VALUE);contents(dest)})
    emit("overflow-slice",observe(true) {contents(original.copyOfUninitializedElements(-1,Int.MAX_VALUE))})
    for(old in 0..64) for(minimum in 0..64) emit("capacity:$old:$minimum",AbstractList.newCapacity(old,minimum).toString())
    val max=Int.MAX_VALUE
    val extremes=intArrayOf(0,1,8,1024,1431655760,1431655765,1431655766,max-9,max-8,max-7,max-1,max)
    for(old in extremes) for(minimum in extremes) emit("capacity:$old:$minimum",AbstractList.newCapacity(old,minimum).toString())
    val source=numbers();val destination=IntArray(4)
    check(cppIntLength(source)==4 && cppIntGet(source,2)==12)
    cppIntSet(source,2,77);check(source[2]==77)
    cppIntCopy(source,0,destination,0,4);check(contents(destination)=="10,11,77,13")
    cppIntCopy(destination,0,destination,1,3);check(contents(destination)=="10,10,11,77")
    cppIntCopy(destination,1,destination,0,3);check(contents(destination)=="10,11,77,77")
    cppIntFill(destination,1,4,72);check(contents(destination)=="10,72,72,72")
    println("native-int-array-handoff=get,set,length,copy,overlap,fill")
}
