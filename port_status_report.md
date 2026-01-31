# Code Port - Progress Report

**Generated:** 2026-01-30
**Source:** tmp/kotlin/libraries/stdlib/src/kotlin/coroutines
**Target:** src/kotlinx/coroutines

## Executive Summary

| Metric | Count | Percentage |
|--------|-------|------------|
| Total source files | 8 | 100% |
| Ported to target | 260 | 3250.0% |
| Matched files | 7 | 87.5% |
| Missing files | 1 | 12.5% |

## Port Quality Analysis

**Average Similarity:** 0.58

**Quality Distribution:**
- Excellent (≥0.85): 0 files (0.0% of matched)
- Good (0.60-0.84): 3 files (42.9% of matched)
- Critical (<0.60): 4 files (57.1% of matched)

### Excellent Ports (Similarity ≥ 0.85)

These files are well-ported and likely complete:


### Critical Ports (Similarity < 0.60)

These files need significant work:

- `intrinsics.Intrinsics` → `intrinsics.Intrinsics` (0.53, 1 deps)
- `ContinuationInterceptor` → `ContinuationInterceptor` (0.59, 1 deps)
- `CoroutinesH` → `dsl.Coroutines` (0.51)
- `CoroutinesIntrinsicsH` → `Coroutine` (0.39)

## High Priority Missing Files

Files with highest dependency counts:

1. **cancellation.CancellationExceptionH** (0 deps)

## Documentation Gaps

**Documentation coverage:** 502 / 267 lines (188%)

Files with significant documentation gaps (>80%):

- `ContinuationInterceptor` - 100% gap (39 → 0 lines)

