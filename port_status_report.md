# Code Port - Progress Report

**Generated:** 2026-02-02
**Source:** /Volumes/stuff/Projects/kotlin.coroutines-cpp/tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src
**Target:** /Volumes/stuff/Projects/kotlin.coroutines-cpp/src/kotlinx/coroutines

## Executive Summary

| Metric | Count | Percentage |
|--------|-------|------------|
| Total source files | 111 | 100% |
| Target units (paired) | 190 | - |
| Target files (total) | 275 | - |
| Porting progress | 111 | 100.0% (matched) |
| Missing files | 0 | 0.0% |

## Port Quality Analysis

**Average Similarity:** 0.62

**Quality Distribution:**
- Excellent (≥0.85): 0 files (0.0% of matched)
- Good (0.60-0.84): 77 files (69.4% of matched)
- Critical (<0.60): 34 files (30.6% of matched)

### Excellent Ports (Similarity ≥ 0.85)

These files are well-ported and likely complete:


### Critical Ports (Similarity < 0.60)

These files need significant work:

- `operators.Lint` → `flow.Lint` (0.57)
- `Exceptions.common` → `Exceptions` (0.43)
- `Annotations` → `Annotations` (0.22)
- `Dispatchers.common` → `Dispatchers` (0.54)
- `JobSupport` → `JobImpl` (0.40)
- `CoroutineName` → `CoroutineName` (0.56)
- `NonCancellable` → `NonCancellable` (0.54)
- `sync.Semaphore` → `sync.Semaphore` (0.59)
- `internal.DispatchedTask` → `internal.DispatchedTask` (0.56)
- `internal.Synchronized.common` → `internal.SynchronizedObject` (0.56)
- `internal.NullSurrogate` → `internal.NullSurrogate` (0.47)
- `operators.Share` → `flow.Share` (0.51)
- `terminal.Collect` → `flow.Collect` (0.52)
- `SchedulerTask.common` → `SchedulerTask.common` (0.38)
- `flow.Migration` → `flow.Migration` (0.23)
- `selects.SelectOld` → `selects.SelectOld` (0.20)
- `selects.WhileSelect` → `selects.WhileSelect` (0.30)
- `intrinsics.Undispatched` → `intrinsics.Undispatched` (0.30)
- `internal.NamedDispatcher` → `internal.NamedDispatcher` (0.25)
- `CloseableCoroutineDispatcher` → `native.CloseableCoroutineDispatcher` (0.37)
- `EventLoop.common` → `EventLoop.common` (0.55)
- `internal.AbstractSharedFlow` → `internal.AbstractSharedFlow` (0.59)
- `internal.LockFreeLinkedList.common` → `internal.LockFreeLinkedList.common` (0.53)
- `channels.ChannelCoroutine` → `channels.ChannelCoroutine` (0.57)
- `CoroutineContext.common` → `CoroutineContext.common` (0.53)
- `Await` → `Await` (0.57)
- `internal.SafeCollector.common` → `internal.SafeCollector.common` (0.58)
- `Guidance` → `Guidance` (0.55)
- `internal.Scopes` → `internal.Scopes` (0.56)
- `Unconfined` → `Unconfined` (0.56)
- `channels.Deprecated` → `channels.Deprecated` (0.18)
- `internal.InternalAnnotations.common` → `internal.InternalAnnotations.common` (0.21)
- `internal.ThreadContext.common` → `internal.ThreadContext` (0.50)
- `Runnable.common` → `Runnable` (0.58)

## High Priority Missing Files

Files with highest dependency counts:


## Documentation Gaps

**Documentation coverage:** 8963 / 10210 lines (88%)

Files with significant documentation gaps (>80%):

- `EventLoop.common` - 87% gap (112 → 15 lines)
- `CoroutineExceptionHandler` - 88% gap (67 → 8 lines)
- `CompletableDeferred` - 100% gap (51 → 0 lines)
- `channels.Deprecated` - 100% gap (48 → 0 lines)
- `internal.LocalAtomics.common` - 100% gap (8 → 0 lines)
- `channels.Broadcast` - 100% gap (6 → 0 lines)

