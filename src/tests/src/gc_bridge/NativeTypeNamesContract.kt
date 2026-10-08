@file:OptIn(kotlin.experimental.ExperimentalNativeApi::class, kotlinx.cinterop.ExperimentalForeignApi::class, kotlin.native.internal.InternalForKotlinNative::class, kotlin.native.runtime.NativeRuntimeApi::class)
@file:Suppress("INVISIBLE_REFERENCE", "INVISIBLE_MEMBER")
package docking.metadata

// Test fixture: objects and metadata come from the actual Native compiler.
import UnpackagedName
import kotlin.native.internal.ExportForCppRuntime
import kotlin.native.internal.GCUnsafeCall
import kotlin.native.internal.NativePtr
import kotlin.native.internal.TypeInfoNames
import kotlin.native.runtime.GC

private class Payload
private class Outer {
    inner class Inner
    class Nested { class Deeper }
}
private object Singleton
private class `Δ雪😀`

@ExportForCppRuntime("NativeTypeNames_create")
fun create(index: Int): Any = when (index) {
    0 -> Payload()
    1 -> Outer().Inner()
    2 -> Outer.Nested()
    3 -> Singleton
    4 -> object {}
    5 -> { class Local; Local() }
    6 -> ({ value: Int -> value + 1 })
    7 -> "Native string"
    8 -> 42
    9 -> `Δ雪😀`()
    10 -> Outer.Nested.Deeper()
    11 -> UnpackagedName()
    else -> error("Unexpected fixture index")
}

@GCUnsafeCall("Kotlin_Any_getTypeInfo")
private external fun objectTypeInfo(value: Any): NativePtr

@ExportForCppRuntime("NativeTypeNames_expected")
fun expected(value: Any, kind: Int): String? {
    // TypeInfoNames is the unchanged pinned source compiled into this fixture.
    val names = TypeInfoNames(objectTypeInfo(value))
    return when (kind) {
        0 -> names.simpleName.also { check(it == value::class.simpleName) }
        1 -> names.qualifiedName.also { check(it == value::class.qualifiedName) }
        2 -> names.fullName
        else -> error("Unexpected name kind")
    }
}

@ExportForCppRuntime("NativeTypeNames_collect")
fun collect() { GC.collect() }

@GCUnsafeCall("NativeTypeNames_cppContract")
external fun cppContract(): Int

fun main() {
    check(cppContract() == 72)
    println("native-type-name-observations=72")
}
