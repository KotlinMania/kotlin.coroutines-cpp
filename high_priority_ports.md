# High Priority Ports - Action Plan

## Top 20 Files by Impact (Priority Score = Deps × (1 - Similarity))

| Rank | Source | Target | Similarity | Deps | Priority |
|------|--------|--------|------------|------|----------|
| 1 | `intrinsics.Intrinsics` | `intrinsics.Intrinsics` | 0.53 | 1 | 0.5 |
| 2 | `ContinuationInterceptor` | `ContinuationInterceptor` | 0.59 | 1 | 0.4 |
| 3 | `Continuation` | `CancellableContinuation` | 0.65 | 1 | 0.3 |
| 4 | `CoroutineContext` | `native.CoroutineContext` | 0.73 | 1 | 0.3 |
| 5 | `CoroutinesH` | `dsl.Coroutines` | 0.51 | 0 | 0.0 |
| 6 | `CoroutineContextImpl` | `CoroutineContext` | 0.68 | 0 | 0.0 |
| 7 | `CoroutinesIntrinsicsH` | `Coroutine` | 0.39 | 0 | 0.0 |

## Critical Issues (Similarity < 0.60 with Dependencies)

These files need immediate attention:

- **intrinsics.Intrinsics** → `intrinsics.Intrinsics`
  - Similarity: 0.53
  - Dependencies: 1

- **ContinuationInterceptor** → `ContinuationInterceptor`
  - Similarity: 0.59
  - Dependencies: 1

