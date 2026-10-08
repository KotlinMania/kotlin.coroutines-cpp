# kotlinx.coroutines-cpp Implementation Roadmap

## Current status and work order

`ast_distance` is the oracle for the porting inventory, measured transliteration
criteria, gaps, and repair priorities. Current status comes from a fresh CLI run
against the vendored Kotlin sources and C++ port. Do not maintain independent
completion percentages, fixed subsystem verdicts, or calendar estimates here.

Run from `docs/audits/` so generated reports replace the existing reports:

```sh
../../tools/ast_distance/ast_distance --deep \
  ../../tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src kotlin \
  ../../src/kotlinx/coroutines cpp
```

This scope covers the common library sources. For work on other source sets,
run the same command with their corresponding Kotlin and C++ roots and record
that scope explicitly. Compiler/runtime ground truth under `tmp/kotlin` is
separate from the library inventory under `tmp/kotlinx.coroutines`.

Read the generated evidence together:

- [Port status](../audits/port_status_report.md): measured function/type parity,
  body similarity, unmatched files, and scoring findings.
- [Priority ladder](../audits/high_priority_ports.md): dependency fanout,
  symbol deficits, and measured similarity determine the repair order.
- [Next actions](../audits/NEXT_ACTIONS.md): concrete work items and the tool's
  current acceptance checklist.
- [Symbol inventory](../audits/deep_symbol_inventory.txt): complete extracted
  declarations, implementations, identities, and unmatched symbols.
- [Transliteration evidence](../audits/deep_transliteration_evidence.txt): emitted
  buffers, rule coverage, source mappings, differing logic, and limitations.
- [Provenance proposals](../audits/port_lint_proposed_changes.md): proposed
  identity corrections requiring inspection against the actual Kotlin source.

These are generated snapshots, not timeless declarations. Regenerate after
relevant source or tool changes. Do not manually alter generated scores or
priority rows to imply acceptance.

## Required repair workflow

1. Run the local `tools/ast_distance/ast_distance --deep` command and inspect the
   generated priority ladder and complete evidence for the selected work item.
2. Open the matching Kotlin source, C++ header/source pair, and relevant compiler
   or runtime dependencies. Preserve provenance and upstream API structure.
3. Implement the missing behavior. TODO comments, stubs, and placeholder
   implementations are prohibited. A gap recorded in an audit remains a gap.
4. Meet the applicable `ast_distance` criteria. Investigate parser, identity, or
   unsupported-rule findings explicitly; repair the tool when its evidence
   demonstrates a tool defect rather than dismissing the required check.
5. Validate the changed behavior with the relevant coroutine/IR checks and
   upstream parity cases. Record commands, observed results, and remaining
   limits; similarity alone does not establish runtime behavior.
6. Regenerate the reports and use the resulting priorities for the next repair.

## Architectural contracts

The generated work order operates within these contracts:

- [Porting North Star](porting_north_star.md): faithful transliteration, API
  structure, naming, ownership, required criteria, and provenance.
- [Docking Ring](docking_ring.md): Kotlin/Native-compatible continuation and
  compiler-lowering direction.
- [Suspension specification](../suspension/IR_SUSPEND_LOWERING_SPEC.md): current
  frame routing, persistent state, immediate/suspended/resumed results, and the
  marker-cleanup boundary.
- [Suspension implementation](../suspension/SUSPEND_IMPLEMENTATION.md): current
  macro authoring and compiler-launcher mechanics.
- [IR handoff review](../audits/IR_HANDOFF_REVIEW.md): dated repair evidence and
  validation scope, to be rechecked when the relevant implementation changes.

Compiler-driven frame/spill generation, tail-suspend lowering, platform dispatch,
Flow/Channel/Select behavior, cancellation, and GC interoperability must be
assessed against current source and generated evidence. Their mention here does
not assign a completion state or override the tool's repair priorities.
