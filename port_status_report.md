# Code Port - Progress Report

**Generated:** 2026-02-01
**Source:** tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src
**Target:** src/kotlinx/coroutines

## Executive Summary

| Metric | Count | Percentage |
|--------|-------|------------|
| Total source files | 111 | 100% |
| Ported to target | 261 | 235.1% |
| Matched files | 111 | 100.0% |
| Missing files | 0 | 0.0% |

## Port Quality Analysis

**Average Similarity:** 0.60

**Quality Distribution:**
- Excellent (≥0.85): 0 files (0.0% of matched)
- Good (0.60-0.84): 69 files (62.2% of matched)
- Critical (<0.60): 42 files (37.8% of matched)

### Excellent Ports (Similarity ≥ 0.85)

These files are well-ported and likely complete:


### Critical Ports (Similarity < 0.60)

These files need significant work:

- `JobSupport` → `JobSupport` (0.37)
- `Exceptions.common` → `Exceptions` (0.44)
- `Annotations` → `Annotations` (0.21)
- `CoroutineName` → `CoroutineName` (0.56)
- `sync.Semaphore` → `sync.Semaphore` (0.58)
- `flow.SharingStarted` → `flow.SharingStarted` (0.59)
- `internal.SystemProps.common` → `internal.SystemProps` (0.58)
- `Yield` → `Yield` (0.60)
- `internal.DispatchedTask` → `internal.DispatchedTask` (0.56)
- `Dispatchers.common` → `MultithreadedDispatchers` (0.38)
- `internal.NullSurrogate` → `internal.NullSurrogate` (0.48)
- `internal.Synchronized.common` → `internal.SynchronizedObject` (0.56)
- `terminal.Logic` → `flow.Logic` (0.22)
- `operators.Share` → `flow.Share` (0.51)
- `terminal.Count` → `flow.Count` (0.23)
- `terminal.Collect` → `flow.Collect` (0.52)
- `flow.Migration` → `flow.Migration` (0.23)
- `Builders.common` → `Builders.common` (0.43)
- `terminal.Collection` → `flow.Collection` (0.26)
- `selects.SelectOld` → `selects.SelectOld` (0.20)
- `selects.WhileSelect` → `selects.WhileSelect` (0.30)
- `SchedulerTask.common` → `SchedulerTask.common` (0.38)
- `selects.SelectUnbiased` → `selects.SelectUnbiased` (0.18)
- `intrinsics.Undispatched` → `intrinsics.Undispatched` (0.30)
- `channels.Channels.common` → `channels.Channels.common` (0.19)
- `internal.NamedDispatcher` → `internal.NamedDispatcher` (0.25)
- `CloseableCoroutineDispatcher` → `native.CloseableCoroutineDispatcher` (0.37)
- `channels.ChannelCoroutine` → `channels.ChannelCoroutine` (0.57)
- `internal.AbstractSharedFlow` → `internal.AbstractSharedFlow` (0.59)
- `Await` → `Await` (0.44)
- `internal.FlowExceptions.common` → `internal.FlowExceptions.common` (0.23)
- `EventLoop.common` → `EventLoop.common` (0.52)
- `Guidance` → `Guidance` (0.55)
- `Unconfined` → `Unconfined` (0.56)
- `internal.InternalAnnotations.common` → `internal.InternalAnnotations.common` (0.21)
- `internal.ThreadContext.common` → `internal.ThreadContext.common` (0.55)
- `channels.Deprecated` → `channels.Deprecated` (0.18)
- `internal.SafeCollector.common` → `internal.SafeCollector.common` (0.19)
- `internal.Scopes` → `internal.Scopes` (0.56)
- `CoroutineContext.common` → `CoroutineContext.common` (0.57)
- `internal.LockFreeLinkedList.common` → `internal.LockFreeLinkedList.common` (0.54)
- `operators.Lint` → `flow.Lint` (0.56)

## High Priority Missing Files

Files with highest dependency counts:


## Documentation Gaps

**Documentation coverage:** 7889 / 10210 lines (77%)

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

