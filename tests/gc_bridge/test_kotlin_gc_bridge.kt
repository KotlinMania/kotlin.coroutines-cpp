// test_kotlin_gc_bridge.kt
// Kotlin test that calls our C++ GC bridge code
// This shows the real integration when called from Kotlin Native

import kotlin.native.concurrent.*
import kotlin.native.internal.GC
import kotlin.native.runtime.GC as RuntimeGC
import kotlin.system.getTimeMillis

// C++ functions we're testing
@kotlin.native.internal.GCUnsafeCall("test_cpp_without_guard")
external fun testCppWithoutGuard()

@kotlin.native.internal.GCUnsafeCall("test_cpp_with_guard")
external fun testCppWithGuard()

@kotlin.native.internal.GCUnsafeCall("test_cpp_with_safepoints")
external fun testCppWithSafepoints()

@kotlin.native.internal.GCUnsafeCall("test_cpp_get_memory_info")
external fun testCppGetMemoryInfo(): Long

// Kotlin side: Create GC pressure
fun createGCPressure(iterations: Int) {
    val data = mutableListOf<ByteArray>()
    for (i in 0 until iterations) {
        // Allocate 1MB
        data.add(ByteArray(1024 * 1024))
        
        // Force GC occasionally
        if (i % 10 == 0) {
            GC.collect()
        }
        
        // Keep memory bounded
        if (data.size > 50) {
            data.removeAt(0)
        }
    }
}

fun measureGCStats(name: String, block: () -> Unit) {
    println("\n=== $name ===")
    
    val startTime = getTimeMillis()
    val startMem = RuntimeGC.lastGCInfo?.let { it.memoryUsageAfter.totalObjectsSizeBytes } ?: 0L
    
    // Force a GC before test
    GC.collect()
    
    block()
    
    // Force a GC after test to see final state
    GC.collect()
    
    val endTime = getTimeMillis()
    val endMem = RuntimeGC.lastGCInfo?.let { it.memoryUsageAfter.totalObjectsSizeBytes } ?: 0L
    
    println("Duration: ${endTime - startTime} ms")
    println("Memory before: ${startMem / 1024 / 1024} MB")
    println("Memory after: ${endMem / 1024 / 1024} MB")
    println("Memory delta: ${(endMem - startMem) / 1024 / 1024} MB")
    
    RuntimeGC.lastGCInfo?.let { info ->
        println("Last GC duration: ${info.gcDurationNanos / 1_000_000} ms")
        println("Last GC pause time: ${info.pauseDurationNanos / 1_000_000} ms")
    }
}

fun testKotlinOnlyWorkload() {
    measureGCStats("Kotlin Only Workload") {
        createGCPressure(100)
    }
}

fun testCppWithoutGCGuard() {
    measureGCStats("C++ Without GC Guard") {
        // Kotlin creates GC pressure
        createGCPressure(50)
        
        // Call C++ (will switch to Native automatically by cinterop)
        testCppWithoutGuard()
        
        // More GC pressure
        createGCPressure(50)
    }
}

fun testCppWithGCGuard() {
    measureGCStats("C++ With GC Guard (Manual State Control)") {
        // Kotlin creates GC pressure
        createGCPressure(50)
        
        // Call C++ (uses @GCUnsafeCall, so we control state)
        testCppWithGuard()
        
        // More GC pressure
        createGCPressure(50)
    }
}

fun testCppWithSafepointChecks() {
    measureGCStats("C++ With Safepoint Checks") {
        // Kotlin creates GC pressure
        createGCPressure(50)
        
        // Call C++ with periodic safepoint checks
        testCppWithSafepoints()
        
        // More GC pressure
        createGCPressure(50)
    }
}

fun testMixedKotlinCppWorkload() {
    measureGCStats("Mixed Kotlin/C++ Workload") {
        for (i in 0 until 5) {
            println("  Iteration ${i + 1}/5")
            
            // Kotlin work
            createGCPressure(20)
            
            // C++ work
            testCppWithGuard()
            
            // More Kotlin work
            createGCPressure(20)
            
            // Check memory
            val memInfo = testCppGetMemoryInfo()
            println("  C++ heap: ${memInfo / 1024 / 1024} MB")
        }
    }
}

fun testConcurrentKotlinAndCpp() {
    measureGCStats("Concurrent Kotlin and C++ Work") {
        // Start C++ work in background (it will stay in Native state)
        val cppThread = kotlin.concurrent.AtomicReference<kotlin.native.concurrent.Worker?>(null)
        
        // Kotlin work on main thread
        createGCPressure(100)
        
        println("C++ work completed")
    }
}

fun main() {
    println("=".repeat(60))
    println("Kotlin Native GC Bridge Integration Test")
    println("=".repeat(60))
    
    // Check GC status
    println("\nGC Status:")
    println("  GC enabled: ${RuntimeGC.isActive}")
    RuntimeGC.lastGCInfo?.let { info ->
        println("  Last GC epoch: ${info.epoch}")
    }
    
    // Run tests
    testKotlinOnlyWorkload()
    testCppWithoutGCGuard()
    testCppWithGCGuard()
    testCppWithSafepointChecks()
    testMixedKotlinCppWorkload()
    testConcurrentKotlinAndCpp()
    
    println("\n" + "=".repeat(60))
    println("All tests completed!")
    println("=".repeat(60))
}
