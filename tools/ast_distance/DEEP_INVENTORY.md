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

## File identity and emitted algorithm evidence

Kotlin/C++ pairing compares declared package/namespace components exactly,
normalizing `.` versus `::` separators without erasing component boundaries,
case or underscores. Provenance identifies the actual relative source path or
an exact component-boundary suffix of its physical path. A conflicting marker
cannot be rescued by a similar basename. Unmarked files require the same
normalized basename and declared namespace; equal candidates in distinct source
sets remain unmatched with an ambiguity diagnostic requiring provenance.

C++ namespace identity follows declaration scopes rather than the first empty
namespace in a file. Classic/C++17 nested namespace syntax is equivalent. Empty
helper scopes cannot claim declarations elsewhere. Unrelated scopes and
conflicting companion namespaces/provenance produce identity diagnostics and
are excluded from pairing. Include-only translation units may inherit their
companion header's identity. A common namespace enclosing helper subscopes is
recorded; this is file identity evidence, not a compiler symbol-resolution proof.
Direct file comparisons also reject conflicting identity before scoring.

For Kotlin -> C++, `--deep` adds a separate emitted-transliteration metrics table
and saves `deep_transliteration_evidence.txt`. That receipt contains all metrics,
parser/fallback diagnostics, top-level source/emitted byte maps, callable
locations, missing/extra functions, full differing ordered logic sequences and
emitted C++ buffers. Physical files and concatenation order are listed because
paired header/source locations in this receipt refer to the combined buffer.
The existing function score and complete symbol inventory remain distinct from
this bounded rule-pack score. Unsupported classes/suspend lowering and other
constructs are visible fallbacks; a zero emitted score can indicate missing
normalization rules and is not itself a proven port defect.

Empty companion namespace chains retain their deepest declared scope when they
contain no declarations. That permits an implementation in a matching header to
pair with its empty classic nested namespace companion, without letting an empty
scope override declarations in an unrelated scope. Short and full companion
provenance markers must each resolve to the actual selected upstream source,
including when comparing C++ to Kotlin in the reverse direction.

Generic method owners and explicit template specializations retain their names.
Kotlin top-level extension functions can pair with C++ namespace functions only
when the first C++ parameter provides the corresponding receiver type. Known
smart-pointer receiver carriers are unwrapped for this identity check; unrelated
wrappers and class-member namesakes do not satisfy it. Generic arguments, receiver
parameters, continuation arguments and body logic remain in the scoring input.
These matching rules do not establish equivalent coroutine suspension behavior.


`--deep` uses positional exact-token cosine for its emitted Kotlin/C++ distance.
It also adds a suspension lowering review table to stdout and the persisted
`deep_transliteration_evidence.txt`. Each explicit suspend declaration or suspend
callback signature lists matching source/target physical-file locations, await
macro lines, begin macros, identity/IR markers and parsed indirect-goto lines.
Comments, strings and preprocessor definitions do not supply function evidence.
Nested frame bodies are included in their owning operator's evidence.

`NO_LOCAL_LOWERING_REVIEW` and `MARKER_ONLY_REVIEW` are investigation leads,
not defect verdicts. `TAIL_OR_HELPER_REVIEW` identifies a sole returned call whose
lowering may live elsewhere. `LOWERING_SYNTAX_PRESENT` still needs compiled IR
and lifetime/result/cancellation checks. Multiple candidate overloads and parse
limitations remain visible. Inferred suspension and external call resolution are
not established by this syntactic inventory. Review rows never alter scores.
