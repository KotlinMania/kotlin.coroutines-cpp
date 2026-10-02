# IR and coroutine handoff review

Reviewed the clean checkout at `5e9895a100cfdd7b26d2683d39650df706890a3e`,
including Iris's `cd7a14af` standalone transform and `5e9895a1` wrapper/spec
changes. The docking-ring goal is preserved: Kotlin/Native address dispatch
plus retained live state and exact immediate/suspended/resumed result handling.

## Findings repaired

1. **Native tool guessed the frame layout.** `kxs_inject.cpp` treated argument 0
   as the label field, moved entry instructions into a new block, and added
   resume edges without spill or Result lowering. A polymorphic frame's offset
   zero can hold its vtable; ordinary marked functions may have no arguments.
   The tool now verifies the module and removes direct no-op markers only.
2. **CMake compiled both source and transformed object.** The old helper added
   `.kxs.o` to the existing sources, dropped definitions/standard/include flags,
   and used basename-only paths. A compiler launcher now wraps the real command
   and retains CMake's object names, settings, generated sources and dependency
   graph. LLVM optimization/instrumentation runs after cleanup, once.
3. **Text chunking was not LLVM parsing.** Semicolon escaping and regex `define`
   boundaries could not establish a function's frame, dispatch, liveness or
   validity. Cleanup now recognizes complete canonical marker instructions,
   preserves all other bytes, and retains declarations and unsupported uses.
   Quoted/multiline strings, unwind edges, metadata and function-pointer uses
   have explicit regressions.
4. **Documentation claimed nonexistent spill generation and full ABI parity.**
   The IR spec, suspend guide, docking ring and porting north star now distinguish
   the tested cleanup/macro contracts from compiler-driven automatic spilling,
   GC/ownership interop and the experimental DSL generator. Examples route
   suspended operations back to the current frame instead of bypassing it.
5. **Installed LLVM was ignored.** CMake now prefers an installed LLVM package
   and keeps vendored sources as a fallback. The monolithic LLVM target avoids
   duplicate component-library link warnings. The no-op marker is `noexcept`.
6. **Package loading executed the standalone script without inputs.** The
   package config no longer includes the `cmake -P` entry point at import time.
   Helpers are installed beside the module and headers use their actual source
   layout. The relocated-module regression checks paths with spaces.

## Ground truth

- Kotlin compiler checkout: `tmp/kotlin`, commit
  `fee29910d8dddd2b1f7b44036c00533cee493351`.
- Compiler build metadata: Kotlin/Native `2.5.0-dev-5907`, LLVM 21.
- Separate vendored library: `tmp/kotlinx.coroutines/gradle.properties` says
  `1.10.2-SNAPSHOT`, Kotlin build version `2.1.0`. It is not a separate Git
  checkout. This sparse compiler tree does not establish the compiler
  distribution's bundled kotlinx.coroutines artifact version.
- `NativeSuspendFunctionLowering.kt:253-335`: save state/address, normal call
  value or sentinel, resume with restore plus `getOrThrow`.
- `CoroutinesVarSpillingLowering.kt:68-105`: replace spill intrinsics with fields.
- `IrToBitcode.kt:2289-2348`: per-function blockaddress/indirectbr and merge paths.
- Native runtime `ContinuationImpl.kt:21-45`: stop on suspension, propagate a
  completed value or failure to the parent, and release interception on exit.
- `SafeContinuationNative.kt` and library `CancellableContinuationImpl.kt`:
  atomic resume-before-return decisions are distinct from the saved label and
  result payload. IR cleanup leaves those paths untouched.

## Validation

The regression suites cover ten macro/runtime scenarios, including immediate
values, resumed values/failures, repeated suspension, exact completion counts
and independent frames. The cleanup build is compared with the ordinary build
under Debug and optimized AddressSanitizer settings. CMake checks include
source/transitive flags, generated inputs, duplicate basenames, existing
launchers, paths with spaces, and header/helper-triggered rebuilds. Tests account
for macOS Make's whole-second timestamps without changing dependency semantics.

LLVM parser tests also cover zero-argument functions, quoted coroutine names,
retained frame-field accesses, unwind edges, remaining symbol uses, invalid SSA,
wrong marker signatures, bitcode roundtrips and the actual runtime test module.
The locally installed LLVM 23.1.2 and Ninja 1.13.2 enable those tests; they do not
change the compiler snapshot's LLVM version.

Run `test_suspension_core`, `test_ir_pipeline` and `test_kxs_inject` through CTest.
The latter requires the optional native tool to be built with an available LLVM
package. Configure Homebrew LLVM with
`-DLLVM_DIR=/opt/homebrew/opt/llvm/lib/cmake/llvm`.

Applied-checkout validation passed all five selected CTests: suspension core,
IR pipeline, native tool, collect/reduce and combine/zip. The fresh core-library,
native-tool and test build completed without compiler/linker warnings. The
pipeline suite passed all eight checks (including Ninja); the native suite
passed all seven. The Python regression sources are explicitly allowed by
`.gitignore` so the general `test_*` executable rule cannot hide them.

## Remaining boundary

Automatic spill/frame generation, arbitrary suspend-expression/argument and
RAII lowering, full Kotlin/Native binary/GC ABI, and cancellation/dispatcher
parity still require compiler-driven work and upstream parity tests. These
repairs preserve existing state machines rather than claiming those phases are
complete. Xcode and C++ module/BMI workflows have not been validated.
