# Code Port - Progress Report

**Generated:** 2026-02-01
**Source:** tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src
**Target:** src/kotlinx/coroutines

## Executive Summary

| Metric | Count | Percentage |
|--------|-------|------------|
| Total source files | 111 | 100% |
| Target units (paired) | 205 | - |
| Target files (total) | 273 | - |
| Porting progress | 111 | 100.0% (matched) |
| Missing files | 0 | 0.0% |

## Port Quality Analysis

**Average Similarity:** 0.61

**Quality Distribution:**
- Excellent (≥0.85): 0 files (0.0% of matched)
- Good (0.60-0.84): 73 files (65.8% of matched)
- Critical (<0.60): 38 files (34.2% of matched)

### Excellent Ports (Similarity ≥ 0.85)

These files are well-ported and likely complete:


### Critical Ports (Similarity < 0.60)

These files need significant work:

- `operators.Lint` → `flow.Lint` (0.57)
- `Exceptions.common` → `Exceptions` (0.44)
- `Annotations` → `Annotations` (0.22)
- `CoroutineName` → `CoroutineName` (0.56)
- `JobSupport` → `JobImpl` (0.40)
- `NonCancellable` → `NonCancellable` (0.54)
- `sync.Semaphore` → `sync.Semaphore` (0.59)
- `internal.SystemProps.common` → `internal.SystemProps` (0.59)
- `internal.DispatchedTask` → `internal.DispatchedTask` (0.56)
- `Dispatchers.common` → `MultithreadedDispatchers` (0.38)
- `internal.NullSurrogate` → `internal.NullSurrogate` (0.47)
- `internal.Synchronized.common` → `internal.SynchronizedObject` (0.56)
- `operators.Share` → `flow.Share` (0.51)
- `terminal.Collect` → `flow.Collect` (0.52)
- `flow.Migration` → `flow.Migration` (0.23)
- `Builders.common` → `Builders.common` (0.43)
- `selects.SelectOld` → `selects.SelectOld` (0.20)
- `selects.WhileSelect` → `selects.WhileSelect` (0.30)
- `SchedulerTask.common` → `SchedulerTask.common` (0.38)
- `intrinsics.Undispatched` → `intrinsics.Undispatched` (0.30)
- `channels.Channels.common` → `channels.Channels.common` (0.19)
- `internal.NamedDispatcher` → `internal.NamedDispatcher` (0.25)
- `CloseableCoroutineDispatcher` → `native.CloseableCoroutineDispatcher` (0.37)
- `channels.ChannelCoroutine` → `channels.ChannelCoroutine` (0.57)
- `Await` → `Await` (0.57)
- `flow.FlowCollector` → `flow.FlowCollector` (0.54)
- `internal.AbstractSharedFlow` → `internal.AbstractSharedFlow` (0.59)
- `Guidance` → `Guidance` (0.55)
- `internal.ThreadContext.common` → `internal.ThreadContext.common` (0.55)
- `internal.Scopes` → `internal.Scopes` (0.56)
- `internal.InternalAnnotations.common` → `internal.InternalAnnotations.common` (0.21)
- `Unconfined` → `Unconfined` (0.56)
- `internal.LockFreeLinkedList.common` → `internal.LockFreeLinkedList.common` (0.54)
- `CoroutineContext.common` → `CoroutineContext.common` (0.57)
- `internal.SafeCollector.common` → `internal.SafeCollector.common` (0.19)
- `EventLoop.common` → `EventLoop.common` (0.52)
- `internal.FlowExceptions.common` → `internal.FlowExceptions.common` (0.23)
- `channels.Deprecated` → `channels.Deprecated` (0.18)

## High Priority Missing Files

Files with highest dependency counts:


## Documentation Gaps

**Documentation coverage:** 8639 / 10210 lines (85%)

Files with significant documentation gaps (>80%):

- `EventLoop.common` - 94% gap (112 → 7 lines)
- `Builders.common` - 85% gap (75 → 11 lines)
- `CoroutineExceptionHandler` - 88% gap (67 → 8 lines)
- `CompletableDeferred` - 100% gap (51 → 0 lines)
- `channels.Deprecated` - 100% gap (48 → 0 lines)
- `internal.SafeCollector.common` - 82% gap (40 → 7 lines)
- `internal.SystemProps.common` - 81% gap (32 → 6 lines)
- `internal.LocalAtomics.common` - 100% gap (8 → 0 lines)
- `channels.Broadcast` - 100% gap (6 → 0 lines)

