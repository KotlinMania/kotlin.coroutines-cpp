# Deep inventory and evidence

`--deep SOURCE SOURCE_LANGUAGE TARGET TARGET_LANGUAGE` keeps its dependency,
missing-file, implemented-function, type, test, documentation and lint reports.
It also prints and saves `deep_symbol_inventory.txt`, containing every extracted
source type, function/API declaration, property or constant, enum entry and type
alias, including symbols in unmatched files. Target symbols without source
namesakes are listed separately. C++ headers and implementation files belonging
to the same logical unit are inspected together.

The inventory identifies names and owners, with original files and source lines.
`PRESENT` means a matching symbol definition was found. `DECLARATION_ONLY` means
the named API exists but its implementation was not found in that logical unit.
`MISSING_SYMBOL` means no matching name/owner/kind was extracted. Kotlin property
names can match a C++ member or getter; this does not validate mutability,
visibility, accessor signatures or values. Parameter/body similarity and missing
implemented overloads remain separate function-report evidence. A matching name
is never a certificate of API or algorithm equivalence.

Kotlin-to-C++ type coverage now uses the correct grammars in both directions and
includes C++ `using`/`typedef` aliases. Repeated implemented function names count
individually, rather than collapsing all `command` methods into one entry.
Known incompatible class owners are rejected; companion/static and namespace
lowering follow the shared callable-owner normalization.

Small files, boilerplate-only translation units and low source/target line
ratios are review evidence, not proven stubs. Explicit placeholder bodies remain
subject to existing scoring checks. Roots are validated and directory exclusions
apply below the requested root, so a source tree inside a directory named
`build` or `target` is scanned. Missing roots fail instead of reporting zero files
as a clean comparison.

Parser errors and missing grammar nodes are reported with file and line ranges.
Unsupported grammar constructs can still make inventories incomplete. Native
macros, platform declarations, implicit members and language-specific lowering
need source review; the scanner does not implement a compiler's symbol resolver.

`deep_inventory_cli` verifies missing files/types/functions/enum entries,
declaration-only functions, inline companion implementations, properties,
constants and aliases, as well as failure on a missing root. It also checks the
combined Rust-to-Kotlin production and test symbol totals. Those totals include
both primary definitions and supplementary constants/type aliases; the original
subtotal bookkeeping remains intact and is now consumed by the summary.

The implementation-review command also inspects companion headers before
listing an include-only C++ translation unit as unresolved. Its JSON
`implementation_locations` array and terminal section retain those locations as
information; they are excluded from unresolved `findings`. This means finding a
body or type definition in a header locates implementation evidence, not that the
entire logical unit passes source parity. Parser errors remain unresolved review
findings even when a companion contains definitions.
