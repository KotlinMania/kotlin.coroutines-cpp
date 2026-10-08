# C++ suspend expression lowering

## Implementation boundary

The frontend lives in `src/kotlinx/coroutines/tools/clang_suspend_plugin/`.
`SuspendFunctionAnalyzer.cpp` discovers evaluated suspension occurrences and
liveness; `NativeSuspendLowering.cpp` emits retained expressions and storage;
`CompilerFrameLowering.cpp` imports actual declarations and dependencies.

Kotlin contracts come from `NativeSuspendFunctionLowering.kt`,
`CoroutinesVarSpillingLowering.kt`, `AbstractFunctionReferenceLowering.kt` and
`DefaultArgumentStubGenerator.kt` under `tmp/kotlin`. Clang-specific handling is
an adaptation of C++ syntax and lifetime rules, with explicit source provenance.
It does not replace Kotlin IR identity or the coroutine state machine.

## Declaration and occurrence identity

Bindings use actual resolved declarations, qualified types and cv-qualification.
Captured fields preserve the source variable's type. Local aliases, constants,
structured bindings and callable references retain their originating declaration.
Address and reference uses refer to the stored object rather than a copied value.

A selected default expression is identified together with its enclosing use path.
Separate calls sharing a declaration AST have separate suspension occurrences.
Semantic initializer lists supply evaluated member defaults and array fillers.
Unevaluated operands do not become executable dependencies; resolved nondependent
`noexcept` queries retain their original result.

Callable imports retain complete enclosing declarations and lexical context.
Sibling closures are distinguished by original source identity as well as their
invoke declarations. Dependency traversal follows actual calls, constructors,
destructors, referenced function addresses and referenced variable initializers.

## Expression order and lifetime

Ordinary nonsuspending expressions retain native full-expression lifetime.
Suspending siblings require earlier evaluated operands to survive the suspension.
Receiver-before-value assignment follows the Kotlin source ordering contract.
Overloaded operators remain actual calls. Built-in comma containers retain prefix
effects, while temporary owners can require a frame even for a tail operand.

Owned prvalues are constructed in owning storage, including immovable values.
Borrowed glvalues retain their original ownership. Aggregate referents and copied
arrays require construction order and reverse cleanup. Jump cleanup compares the
actual target's lexical objects and catch scopes with the current scope.

## Implementation limits

Complete optimized spill allocation, local/dependent declaration integration,
suspending aggregate and array initialization, immovable subobjects and
initializer-list backing storage remain unfinished. The presence of individual
adapters does not establish the full expression emitter or Native frame ABI.

The production pipeline and complete executable requirements are defined in
[docking_ring.md](docking_ring.md). Timing or runtime failures in unfinished
features must not redirect source translation into an open-ended debugging chain.
