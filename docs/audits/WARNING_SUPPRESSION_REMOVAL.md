# Warning suppression removal

The user explicitly requested removal of all warning suppression. Commit
fb86a72 checkpoints the clean checkout before editing. The removal preserves
the warning-as-error policy and the mandatory coroutine compiler pipeline.

Root CMake retains -Wall -Wextra -Wpedantic -Werror and removes
-Wno-unknown-attributes. Core, debugging and test targets lose all unused and
GNU-label warning-disable flags. IR/LLVM and Native regression commands lose
their warning-disable flags too. Compiler-tool include directories are ordinary
includes; CMAKE_NO_SYSTEM_FROM_IMPORTED prevents imported dependency targets
from restoring system-header treatment. The six parser targets lose blanket
-w options. Vendored parser missing-initializer pragmas, array-header warning
pragmas and lexer unused attributes are removed. Source maybe_unused attributes
and the deprecated-API test pragma are removed. Test instructions no longer
recommend lint suppression. Residual integration and four Native ABI fixtures
lose Suppress annotations.

NativeTypeNamesContract.kt was an ignored source input already used by CMake.
Its complete original contents were committed at 94a552b4 before removing its
suppression. Other ignored build artifacts and historical verification trees
are preserved. Pinned upstream sources under tmp and quoted audit evidence are
ground truth, rather than active warning controls, and are not rewritten.

The final source scan covers 993 implementation/build-definition files,
including ignored source inputs and excluding generated build trees. It finds
zero active warning suppression controls. Suppression-detection strings in
ast_distance and quoted/commented Kotlin provenance are not compiler controls.
Sixty-four current configured CMake target flag files contain no -Wno, -w or
-isystem controls. Historical nested test builds are not classified as freshly
configured targets. Receipts: build/ir-recovery/warning-controls-final-scan.log
and warning-flags-final-check.log. The five edited Python regression scripts
also pass syntax compilation.

The first unsuppressed full CMake build exits 2. Its captured diagnostics include
unused parameters in the installed Clang/LLVM 23.1.2 headers and genuine
deprecated AtomicInt compatibility calls. No global or per-file suppression is
restored, and no error policy is downgraded. Installed compiler headers are not
modified. Receipt: warnings-enabled-full-build.log under build/ir-recovery.
The subsequent full build after the bounded source cleanup is recorded in
warnings-enabled-final-build.log; new build success is not inferred from older
executables.

The separate analyzer rebuild enables -Wall -Wextra -Wpedantic -Werror for C and
C++ and also exits 2. It exposes missing inherited-field initializers in
generated parser tables, scanner prototype/parameter diagnostics and other
vendored C diagnostics. These are recorded rather than masked. Receipts:
warnings-ast-configure.log and warnings-ast-build.log under build/ir-recovery.

Bounded exposed issues are corrected without altering algorithms: dead static
analyzer functions and unused local bookkeeping are removed. The consumed
Native Arrays.kt:85,91 bodies truly ignore their reference/collection-size
arguments. Their public argument types remain unchanged; the concrete C++
definitions omit unnecessary local parameter names and retain the actual
allocation/identity bodies. The original source was read in full, and
NOTE(port) comments explain these bindings at collections/Arrays.cpp:12 and
Arrays.hpp:27. No invented argument use, fallback algorithm or warning
suppression replaces those Native bodies.

The Native reference and type-name targets both rebuild after removal of their
Suppress annotations. Their fresh CTest executables run with zero failures.
The compiler's JVM native-access warnings remain visible in the build logs.
Receipts: warnings-native-contract-build.log, warnings-native-reference-tests.log,
warnings-native-type-names-build.log and warnings-native-type-names-tests.log
under build/ir-recovery. These bounded Native checks do not establish the
complete coroutine/MLX GPU acceptance paths.

Both full-root deep scans exit zero after the final source changes. They use the
previously built source-tree ast_distance executable: rebuilding that executable
with the newly enabled diagnostics is currently unsuccessful. No scoring or
namespace rule is weakened. Library measurements remain 825/2918 functions,
359/560 types, body similarity 0.26 and 123 scoring failures. Compiler
measurements remain 592/7657 functions, 174/1727 types, body similarity 0.36 and
24 scoring failures. Receipts: warnings-enabled-library-deep.log and
warnings-enabled-compiler-deep.log. The complete translation remains active;
this removal does not establish a warning-clean or successfully rebuilt C++
library/compiler toolchain.
