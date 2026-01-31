# Immediate Actions - High-Value Files

Based on AST analysis, here are the concrete next steps.

## Summary

- **Current Progress:** 234.2% (260/111 files)
- **Matched Files:** 111
- **Average Similarity:** 0.59
- **Critical Issues:** 45 files with <0.60 similarity

## Priority 1: Fix Incomplete High-Dependency Files

### 1. flow.Channels
- **Similarity:** 0.69 (needs 16% improvement)
- **Dependencies:** 14
- **Priority Score:** 4.4
- **Action:** Review and complete missing sections

### 2. flow.Flow
- **Similarity:** 0.73 (needs 12% improvement)
- **Dependencies:** 13
- **Priority Score:** 3.5
- **Action:** Review and complete missing sections

## Priority 2: Port Missing High-Value Files

Critical missing files (>10 dependencies):

## Success Criteria

For each file to be considered "complete":
- **Similarity ≥ 0.85** (Excellent threshold)
- All public APIs ported
- All tests ported
- Documentation ported
- port-lint header present

## Next Commands

```bash
# Initialize task queue for systematic porting
cd tools/ast_distance
./ast_distance --init-tasks ../../tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src kotlin ../../src/kotlinx/coroutines cpp tasks.json ../../AGENTS.md

# Get next high-priority task
./ast_distance --assign tasks.json <agent-id>
```
