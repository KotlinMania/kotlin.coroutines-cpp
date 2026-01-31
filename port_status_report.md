# Code Port - Progress Report

**Generated:** 2026-01-31
**Source:** tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src
**Target:** src/kotlinx/coroutines

## Executive Summary

| Metric | Count | Percentage |
|--------|-------|------------|
| Total source files | 111 | 100% |
| Ported to target | 260 | 234.2% |
| Matched files | 111 | 100.0% |
| Missing files | 0 | 0.0% |

## Port Quality Analysis

**Average Similarity:** 0.59

**Quality Distribution:**
- Excellent (≥0.85): 0 files (0.0% of matched)
- Good (0.60-0.84): 66 files (59.5% of matched)
- Critical (<0.60): 45 files (40.5% of matched)

### Excellent Ports (Similarity ≥ 0.85)

These files are well-ported and likely complete:


### Critical Ports (Similarity < 0.60)

These files need significant work:

- `internal.CoroutineExceptionHandlerImpl.common` → `common.CoroutineExceptionHandlerImpl.common` (0.30)
- `sync.Mutex` → `sync.Mutex` (0.46)
- `JobSupport` → `JobSupport` (0.37)
- `Exceptions.common` → `Exceptions` (0.44)
- `Annotations` → `Annotations` (0.21)
- `CoroutineName` → `CoroutineName` (0.56)
- `CoroutineScope` → `CoroutineScope` (0.51)
- `sync.Semaphore` → `sync.Semaphore` (0.46)
- `flow.SharingStarted` → `flow.SharingStarted` (0.45)
- `internal.SystemProps.common` → `internal.SystemProps` (0.58)
- `internal.DispatchedTask` → `internal.DispatchedTask` (0.56)
- `Dispatchers.common` → `MultithreadedDispatchers` (0.38)
- `internal.Synchronized.common` → `internal.SynchronizedObject` (0.56)
- `internal.NullSurrogate` → `internal.NullSurrogate` (0.48)
- `internal.AbstractSharedFlow` → `internal.AbstractSharedFlow` (0.59)
- `terminal.Count` → `flow.Count` (0.23)
- `operators.Share` → `flow.Share` (0.51)
- `terminal.Logic` → `flow.Logic` (0.22)
- `terminal.Collect` → `flow.Collect` (0.52)
- `flow.Migration` → `flow.Migration` (0.24)
- `Builders.common` → `Builders.common` (0.43)
- `terminal.Collection` → `flow.Collection` (0.26)
- `selects.SelectOld` → `selects.SelectOld` (0.22)
- `selects.WhileSelect` → `selects.WhileSelect` (0.32)
- `SchedulerTask.common` → `SchedulerTask.common` (0.40)
- `selects.SelectUnbiased` → `selects.SelectUnbiased` (0.20)
- `intrinsics.Undispatched` → `intrinsics.Undispatched` (0.31)
- `channels.Channels.common` → `channels.Channels.common` (0.19)
- `internal.NamedDispatcher` → `internal.NamedDispatcher` (0.27)
- `CloseableCoroutineDispatcher` → `native.CloseableCoroutineDispatcher` (0.37)
- `channels.ChannelCoroutine` → `channels.ChannelCoroutine` (0.57)
- `Yield` → `Yield` (0.60)
- `Await` → `Await` (0.44)
- `internal.FlowExceptions.common` → `internal.FlowExceptions.common` (0.25)
- `internal.SafeCollector.common` → `internal.SafeCollector.common` (0.20)
- `internal.LockFreeLinkedList.common` → `internal.LockFreeLinkedList.common` (0.54)
- `channels.Deprecated` → `channels.Deprecated` (0.20)
- `internal.InternalAnnotations.common` → `internal.InternalAnnotations.common` (0.21)
- `CoroutineContext.common` → `CoroutineContext.common` (0.57)
- `internal.Scopes` → `internal.Scopes` (0.56)
- `EventLoop.common` → `EventLoop.common` (0.52)
- `internal.ThreadContext.common` → `internal.ThreadContext.common` (0.55)
- `Guidance` → `Guidance` (0.55)
- `Unconfined` → `Unconfined` (0.56)
- `operators.Lint` → `flow.Lint` (0.56)

## High Priority Missing Files

Files with highest dependency counts:


## Documentation Gaps

**Documentation coverage:** 7608 / 10210 lines (75%)

Files with significant documentation gaps (>80%):

- `EventLoop.common` - 94% gap (112 → 7 lines)
- `terminal.Logic` - 88% gap (77 → 9 lines)
- `Builders.common` - 85% gap (75 → 11 lines)
- `CoroutineExceptionHandler` - 88% gap (67 → 8 lines)
- `CompletableDeferred` - 100% gap (51 → 0 lines)
- `channels.Deprecated` - 100% gap (48 → 0 lines)
- `operators.Lint` - 84% gap (55 → 9 lines)
- `internal.SafeCollector.common` - 82% gap (40 → 7 lines)
- `internal.Combine` - 100% gap (31 → 0 lines)
- `internal.SystemProps.common` - 81% gap (32 → 6 lines)

