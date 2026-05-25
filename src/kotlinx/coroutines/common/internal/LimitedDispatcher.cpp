/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/LimitedDispatcher.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Limited-parallelism dispatcher: wraps an underlying dispatcher and caps concurrent
 * task execution at `parallelism`, while emulating fairness.
 *
 * The full LimitedDispatcher class — its LockFreeTaskQueue<Runnable> backing store, the
 * Worker inner class that cooperatively preempts, the runningWorkers counter, and the
 * dispatch / dispatchYield / limitedParallelism overrides — lives in the matching header
 * (LimitedDispatcher.hpp). This translation unit is the inventory companion for the file
 * pair; the class is header-defined so the underlying-dispatcher pointer and the queue's
 * template instantiation can stay inline.
 */

#include "kotlinx/coroutines/internal/LimitedDispatcher.hpp"
