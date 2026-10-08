# Compiler metadata and collection dependencies

## Identity boundary

Compiler metadata under `src/kotlinx/coroutines/tools/kotlinc_native_ref/` follows
the pinned source under `tmp/kotlin`. `Name`, `FqName`, `FqNameUnsafe`, `ClassId`
and `CallableId` describe actual compiler names and declarations. These internal
value contracts do not impose Kotlin object storage or an `Any` superclass on
ordinary application C++ classes.

Names preserve UTF-16, quoted separators, root diagnostics, locality and structural
equality/hash behavior. Callable identity compares package, class and callable
name; its debug path and class-ID locality do not alter equality. Copy retains
the existing class ID and debug path; class replacement constructs the source's
new identity. Qualified text preserves package/class separator distinctions.

## Catalog representation

Primitive catalogs retain canonical immutable instances and borrowed getter
references. First-access property storage avoids C++ cross-unit initialization
hazards. Publication allows concurrent candidate computation, keeps the winning
property and destroys losing candidates. This is compiler metadata ownership.

`KonanPrimitiveType` preserves all ten source kinds and the explicit CHAR-to-SHORT
binary mapping. `BinaryType` separates primitive and reference families; reference
sequences remain lazy and retain their actual typed providers. Runtime annotation
names come from the actual name catalog rather than inferred strings.

Generated enum APIs, primitive sets, derived StandardClassIds collections and
NativeRuntimeNames atomic maps remain incomplete. Existing scalar getters do not
supply those collection-dependent APIs.

## Collections and scalar operations

Grouping destination operations preserve iterator/key/lookup/operation/store order,
distinguish missing keys from present-null accumulators and return the same
mutable destination. Public generic variance uses the existing collection codecs.
Integer counts and numeric operations preserve Kotlin wrapping through defined
C++ unsigned arithmetic and bit conversion.

Fresh-map Grouping operations require the actual mutableMapOf/LinkedHashMap
implementation. Native HashMap sizing helpers do not supply the concrete map,
its storage, equality, views or iterators. Arbitrary erased element equality/hash
must be translated at the actual collection boundary.

FqName segment-list operations, source collection bases, generated enum inputs
and byFqNameParts remain missing dependencies. InlineClassesSupport, actual IR
classification and structural type-cache consumers are also incomplete.
Replacement standard containers or LLVM pointer-shape classifiers would change
the source algorithm and are not acceptable substitutes.

## Measurement

Receiver extensions, companions, object properties and mixed provenance require
complete analyzer accounting. Unmatched implemented functions remain tool repair
work; unsupported emission and parser failures remain visible findings. Neither
symbol presence nor an isolated catalog test establishes whole-source parity.
