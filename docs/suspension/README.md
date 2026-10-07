# Suspension Documentation Index

This directory documents the coroutine suspension subsystem, compiler lowering specifications, Clang plugin design, and AST/IR transformations.

---

## ⚡ Suspension Documents

### [SUSPEND_IMPLEMENTATION.md](SUSPEND_IMPLEMENTATION.md)
**Purpose:** Design and runtime mechanics of the suspend function implementation.  
**Contents:**
- Clang computed-goto macros (`src/kotlinx/coroutines/dsl/Suspend.hpp`)
- Runtime handoffs: continuation passing, `COROUTINE_SUSPENDED` sentinel, frame state transitions
- Persistent frame state: retaining live state across suspension points vs stack locals
- Mandatory in-compiler LLVM lowering (`KotlinxCoroutinePass`, `-fpass-plugin`)

### [IR_SUSPEND_LOWERING_SPEC.md](IR_SUSPEND_LOWERING_SPEC.md)
**Purpose:** Specification for IR-level suspend lowering matching Kotlin/Native's continuation protocol.  
**Contents:**
- Resume addresses vs tooling IDs (`blockaddress`, `indirectbr`, labels-as-values)
- Kotlin compiler ground truth contracts (`NativeSuspendFunctionLowering.kt`, `CoroutinesVarSpillingLowering.kt`, `IrToBitcode.kt`, `ContinuationImpl.kt`)
- Continuation routing, frame layouts, and atomic decision state
- Verification and testing contracts for lowered bitcode

### [CLANG_SUSPEND_EXTRACTION.md](CLANG_SUSPEND_EXTRACTION.md)
**Purpose:** Catalog of minimal Clang and LLVM APIs required to extract and implement native `suspend` keyword support.  
**Contents:**
- LLVM IR generation: `CreateIndirectBr`, `BlockAddress`, `CreatePHI`, `CreateStructGEP`, control flow
- Clang CFG and analysis: CFG construction, block iteration, statement inspection
- Clang AST visitors and rewrite hooks for coroutine transformation
