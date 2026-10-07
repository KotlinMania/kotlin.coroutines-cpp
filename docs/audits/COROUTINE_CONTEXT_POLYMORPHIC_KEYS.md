# Consumed coroutine-context polymorphic keys

Date: 2026-10-07. This closes the actual context dependency used by
CoroutineDispatcher.kt:65-67. The complete library dispatcher source,
CoroutineContextImpl.kt and ContinuationInterceptor.kt were read before changing
their C++ counterparts. The interceptor source was absent from the extracted
working snapshot; its exact blob was restored from the existing upstream Kotlin
checkout's HEAD, without fetching another revision or inventing a source contract.

`src/kotlinx/coroutines/context_impl.hpp:39` exposes the source generic
AbstractCoroutineContextKey<B, E>. Kotlin's abstract construction and element
subtype bound are preserved through a protected constructor and compile-time
checks. The check occurs when the constructor is instantiated, because the
CoroutineDispatcher nested companion refers to its still-incomplete enclosing
class in C++. An initial check at class instantiation failed to compile; the
final constructor check enforces the same constraint after the class is complete.

The concrete wildcard-key representation at `context_impl.hpp:15` is the C++
binding for Kotlin's AbstractCoroutineContextKey<*, *> runtime type test.
It does not add another key subobject. The constructor at `context_impl.cpp:12`
follows the source topmostKey expression, including nested polymorphic keys.
try_cast at :21 calls the actual supplied safeCast. is_sub_key at :27 tests only
this key or the topmost key, rather than accepting every intermediate key.
Base keys are borrowed; safeCast is retained as a callable. No raw key is adopted
into a shared owner.

The source extension branches live at `context_impl.cpp:32,41`:
get_polymorphic_element calls safeCast only when is_sub_key matches, otherwise
returns null. minus_polymorphic_key returns the canonical EmptyCoroutineContext
only on the matching non-null cast; rejected casts and unrelated keys preserve
the original element. The typed public get binding at `context_impl.hpp:66`
projects the source E cast. Concrete algorithms remain in .cpp; only generic
public bindings and the erased ABI layout are in the header.

`ContinuationInterceptor.cpp:14,26` now translates the source specialized get
and minusKey. Its normal-key branch uses the ContinuationInterceptor companion,
as the actual source specifies, rather than substituting Element's this.key
comparison. A rejected lookup returns null before acquiring a shared handle,
including on an ordinary borrowed C++ interceptor. A selected element uses its
existing shared ownership; no borrowed object is adopted. The source's genuine
empty releaseInterceptedContinuation remains unchanged and now references the
correct source range, :48-50.

`CoroutineDispatcher.hpp:74` and `CoroutineDispatcher.cpp:26,32` expose the
actual companion Key via the C++ Key type and KEY singleton. Its base is
ContinuationInterceptor and its safe cast is CoroutineDispatcher. The actual
element key remains ContinuationInterceptor, so dispatcher replacement and
interceptor-last ordering continue through the existing source context algebra.

`src/tests/src/suspend/test_channel_consumption.cpp:258` exercises root/self key
identity, nested topmost-key propagation, rejected intermediate and unrelated
keys, cast invocation counts and short-circuit order, null casts, exact thrown
exception identity, typed result identity, combined lookup/removal and original
context identity. It also checks actual dispatcher/subtype keys, a non-dispatcher
interceptor, replacement by that interceptor and ordinary Element's opt-in rule.
The source fixture uses existing shared objects and borrowed keys, without
another runtime or coroutine state machine.

The core archive and ten focused executables build; ten CTests finish with zero
failures. Receipts: build/ir-recovery/polymorphic-context-focused-{build,tests}.log.
The changed context/interceptor/dispatcher implementations, Native exception
implementation and full regression fixture were directly instrumented with
AddressSanitizer and UndefinedBehaviorSanitizer. Execution exits zero with no
diagnostics; receipts: polymorphic-context-sanitizer-{build,tests}.log.
An ordinary C++ application compiled without plugin flags, a Kotlin compiler,
JVM or Native runtime and executed dispatcher key lookup/removal plus typed
lookup. Its dynamic dependencies are libc++ and libSystem. Receipts:
polymorphic-context-standalone-{build,tests}.log; source:
build/ir-recovery/polymorphic_context_standalone.cpp.

Sixty-one ranged provenance references across six library files resolve to
existing sources with valid line bounds. No prohibited markers occur in those
files. Neither this check nor these bounded executable cases establish full
source-body parity, serialization, full stdlib translation or the required
Native/MLX shared-state-machine product acceptance paths.

Both full-root ast_distance --deep scans were refreshed after the final source
and fixture changes. The library report records 811/2918 matched function body
names, 359/560 types, average body similarity 0.26 and 123 scoring failures.
The previous report recorded 804 functions and 358 types. These counts do not
certify whole-body correspondence or completed files.

The compiler/prerequisite report still rejects CoroutineContextImpl and
ContinuationInterceptor. A direct comparison identifies the exact reason:
Kotlin package kotlin.coroutines versus the existing C++ kotlinx::coroutines
namespace. namespace_identity_matches in tools/ast_distance/include/codebase.hpp
requires identical declared namespace parts. This is an actual namespace
projection gap, not evidence that the inspected bodies are absent and not a
verified tool bug. The tool is not weakened to treat different namespaces as
identical. Existing generated findings remain visible. Receipts:
polymorphic-context-{library,compiler}-deep.log and
polymorphic-context-interceptor-identity.log. Source-first library repairs remain
the priority; these are consumed context dependencies rather than an unrelated
compiler translation project.
