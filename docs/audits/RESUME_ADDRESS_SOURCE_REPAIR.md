# Compiler resume-address source repair — 2026-10-07

Source commits d7feb4fb and a4e0c319 continue the active compiler state-machine
translation. The pinned LocalDeclarationPopupLowering.kt, CoroutinesVarSpillingLowering.kt
and consumed IrToBitcode.kt:2281-2337 were read with the frontend/frame importer,
Native lowering and mandatory LLVM injection. CMake plugin modules and the
Native-disabled target's actual compile commands were reviewed.

## Source repairs

NativeSuspendLowering.cpp:995 now rewrites a static declaration's initializer
through the actual retained binding map. Its C++ static storage duration and
initialization guard remain intact; parameters, automatic locals and receivers
refer to their actual frame bindings. The qualified_locals fixture adds varying
arguments, an automatic local used by grouped static initializers, one-time
initialization and stable static addresses alongside repeated suspension,
cv-qualified identity, completion, immediate/resumed failure and cancellation.
Existing binaries reproduce stale parameter/local names and duplicate static
emission; they do not contain either repair.

IrToBitcode.kt creates bbResume in code generation and resolves its suspension
identity to blockAddress(bbResume). The adapted frontend now emits ordinary C++
conditional marker branches instead of GNU label addresses. Suspend.hpp:25,27
adds declaration-only compiler contracts; :106,129 updates the actual macros.
NativeSuspendLowering.cpp:242,968,1118 updates suspension, handler-context and
unwind regions. The same generated frame, void* label and Continuation ABI remain.

CoroutineInjection.cpp:56,109 pairs each __kxs_suspend_site with a constant-ID
__kxs_resume_point conditional branch in that function. Its true successor is
an actual LLVM block. The injector creates its BlockAddress, stores it in the
exact supplied persistent field, registers it for indirectbr, replaces the
marker condition with false and removes both calls. IDs are compile-time pairing
data and are erased; no integer state or marker runtime is introduced. Signature,
direct-call/branch, unique/matched ID, persistent field, same field and local
address checks reject malformed contracts. The earlier explicit-address marker
remains accepted for existing LLVM inputs. KotlinxCoroutinePass.cpp:26 recognizes
all marker declarations before optimization. No compiler launcher or serialized
IR stage is added to production.

The existing test_kxs_inject suite gains repeated two-point, non-monotonic ID,
independent-frame address tests and malformed pairing/branch/field checks.
test_llvm_pass adds ordinary strict C++ compilation, optimized/unoptimized
sanitizer execution and emitted marker-erasure/address checks. These are pending
execution against fresh binaries; they do not establish complete Native or MLX
acceptance.

## Verification

- Strict frontend emission of the actual new standalone test source exits 0.
  Its emitted LLVM has the expected direct i1 marker condition and true branch.
- opt -passes=verify accepts the new LLVM test input (exit 0). This verifies input
  structure, not the changed injection implementation.
- Python test source syntax parses successfully.
- Strict actual test_suspension_core.cpp syntax check exits 1 on five dependency
  unused-parameter diagnostics. GNU label-address diagnostics are absent from
  the changed authoring macros.
- Fresh KotlinxSuspendPlugin and KotlinxCoroutinePass CMake builds each exit 2
  in KotlinxClassMetadataPlugin's LLVM/Clang dependency headers. No warning
  suppression, system-header workaround or runtime substitute was added.
- The new compiler-pass regression run against the older installed pass exits 1
  at link time with unresolved site/resume markers. It is evidence that the old
  pass cannot execute the new contract, not execution of the source repair.

Receipts under build/ir-recovery use static-binding- and resume-markers- prefixes.
Specific files: static-binding-fixture.log, static-binding-plugin-build.log,
resume-markers-frontend.log, standard-resume-markers.ll,
resume-markers-test-ir-verify.log, resume-markers-core-source.log,
resume-markers-plugin-build.log, resume-markers-ir-build.log and
resume-markers-existing-pass.log. No fresh runtime validates either source repair.

CMakeCache retains KOTLIN_NATIVE_RUNTIME_AVAILABLE=OFF. The actual core command
contains both the frontend plugin and -fpass-plugin. CMake production remains
in-compiler frontend/LLVM lowering; no CMake source change was needed this batch.
Both exact full-root scans exit 0 without concurrent source changes. Logs:
resume-markers-library-deep.log and resume-markers-compiler-deep.log. Reports
remain unchanged: library 832/2918 bodies, 359/560 types, similarity 0.26 with
123 scoring failures; compiler 592/7657 bodies, 174/1727 types, similarity 0.36
with 24 failures. Those coarse matches do not validate this compiler adaptation.

Local class/lambda lexical declaration integration remains incomplete. Strict
fresh dependency compilation still prevents building the changed plugins. Both
complete standalone C++/MLX and actual Native shared-frame GPU acceptance remain
unproven. The full transliteration/state-machine goal remains active.
