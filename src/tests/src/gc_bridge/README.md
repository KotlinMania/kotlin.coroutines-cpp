# Native reference ABI contract

This fixture executes real Kotlin/Native objects and runtime calls. The production
boundary is `src/kotlinx/coroutines/KotlinGCBridge.hpp/.cpp`. Assertions belong only
in NativeReferenceContract.cpp and the Kotlin caller. No standalone substitute runs
when a toolchain, compilation or runtime link is missing.

The current verification host is macOS ARM64 with Native 2.4.10 and that compiler's
LLVM 21 Clang. The production target remains Kotlin/Native on bare metal. This host
test does not establish MLX/GPU execution or shared coroutine-frame compatibility.

From the repository root, supply actual toolchain paths:

```bash
bash src/tests/src/gc_bridge/build_and_test_gc_bridge.sh \
  /path/to/kotlin-native/bin/konanc \
  /path/to/native-matching-llvm/bin/clang++ \
  build/native-reference-contract
```

To build through CMake, set KOTLIN_NATIVE_RUNTIME_AVAILABLE=ON,
KOTLINX_KONANC and KOTLINX_NATIVE_CLANGXX. Build native_reference_contract, then run
`ctest --test-dir build/ir-recovery -R '^native_reference_contract$' --output-on-failure`.
The option controls test inclusion only; production runtime symbols remain mandatory.

Expected fixture output after actual execution:

```text
native-roots=10
native-return-slot=1
```

Ten contract groups verify real stack-root lifetime/release, nested frames and
C++ unwinding, compare/exchange results and returned-root lifetimes, plus actual
Native/Runnable transitions and safe points. A separate Kotlin caller checks an
object returned through the actual Native result slot after C++ exits and GC runs.
Atomic operations use shadow-stack rooted slots, not assumed object fields.
Heap/global registration, concurrent Native reference objects and coroutine frames
remain unverified. No throughput or pause-time threshold is asserted.

The source is pinned to fee29910d8dddd2b1f7b44036c00533cee493351. The installed
Native runtime differs: it defines InitAndRegisterGlobal where the pinned source
requires RegisterGlobal. Global-root execution needs a matching pinned runtime;
the production declaration remains faithful and has no compatibility alias.
Internal Native annotation access produces a compiler warning recorded with the
build. The fixture does not claim a supported public C interop API.

See [the current runtime specification](../../../../docs/runtime-and-gc/KOTLIN_NATIVE_GC_SPECIFICATION.md)
and `build/ir-recovery/native-references/` for evidence and remaining work.

## Actual Native class-name lookup

The optional native_type_names_contract target compiles the unchanged pinned
TypeInfoNames.kt and its translated C++ getters. It compares simple/qualified/full
names on twelve actual Native objects before/after GC, including nested/inner/local,
anonymous/lambda, Unicode, String/Int and default-package classes: 72 observations.
Kotlin also checks simple/qualified results against its actual KClass getters.
Actual metadata identity and root-stack restoration are checked in the C++ fixture.

```bash
cmake --build build/ir-recovery --target native_type_names_contract -j4
ctest --test-dir build/ir-recovery \
  -R '^(native_type_names_contract|native_reference_contract)$' --output-on-failure
```

Explicit toolchains may be supplied to build_native_type_names_contract.sh
KONANC NATIVE_CLANGXX BUILD_DIR. The compiled Kotlin source is not rewritten.
Its internal-annotation opt-in, friend stdlib and explicit packaged entry point
are fixture compilation requirements. Native 2.4.10/LLVM 21 provides actual host
machine-code evidence, not pinned full-runtime or shared-frame/MLX acceptance.
The Native-only TypeInfo/UTF-16 ABI adaptation does not provide C++ compiler-object
metadata, NativePtr/value-class boxing or implicit Any ancestry. It is absent from
Native-OFF application dependencies. Receipts:
build/ir-recovery/ir-native-type-names/ and the IR identity dependency ledger.

## Native array utilities and handoff contract

The additional native_array_contract target links real C++ array reset bodies and
the actual Native array get/set/fill/copy runtime. It compares 862 source-algorithm
observations with kxs_array_util_test, then checks direct Native/C++ array identity,
both overlap directions, reset/fill and rooted returns separately. Compiler-owned
C++ arrays and real Native arrays remain distinct. The source-only comparison groups
null and implementation-dependent uninitialized reads and normalizes runtime bounds
errors by category where messages are unspecified.

Build kxs_array_util_test and native_array_contract, then run:

```bash
ctest --test-dir build/ir-recovery \
  -R '^(native_array_contract|kxs_array_util_contract|kxs_array_iterator_contract)$' \
  --output-on-failure
```

With explicit toolchains, the Native executable can also be built using
build_native_array_contract.sh KONANC NATIVE_CLANGXX BUILD_DIR. The Python comparison
runner src/tests/ir/test_native_array_util.py requires both real executables; it never
executes a substitute on Native compilation/link/runtime failure. Native 2.4.10 and
its matching LLVM 21 provide bounded macOS ARM64 evidence. Pinned runtime and bare-metal
shared-frame acceptance remain required. See the runtime specification and
build/ir-recovery/native-map-dependencies/ for source identities, logs and receipts.


## Native integer-array and capacity contract

kxs_int_array_test runs actual translated IntArray/iterator/generated utility and
AbstractList companion bodies. native_int_array_contract compares 5,367 observations
against actual Native stdlib, then separately exercises get/set/length/copy, both
overlap directions and fill on real Native-owned integer arrays from C++.

```bash
cmake --build build/ir-recovery --target kxs_int_array_test native_int_array_contract -j4
ctest --test-dir build/ir-recovery \
  -R '^(native_int_array_contract|kxs_int_array_contract)$' --output-on-failure
```

The Native build uses build_native_int_array_contract.sh KONANC NATIVE_CLANGXX BUILD_DIR
and the same explicit matching toolchains as the object-array contract. The friend
stdlib option permits source-internal helper calls in this proof fixture. Native
compiler warnings about internal-visibility suppression are retained in receipts.
Compiler-owned C++ arrays are not Native objects. Bundled source bodies are checked
against the pinned source; installed Native 2.4.10 provides bounded macOS ARM64
execution, not pinned full-runtime or bare-metal acceptance. See
build/ir-recovery/native-int-array/ and the identity dependency ledger.
