# Kotlin Coroutines C++ Documentation Wiki

Welcome to the central documentation wiki for the `kotlin.coroutines-cpp` project, a faithful transliteration and runtime port of [kotlinx.coroutines](https://github.com/Kotlin/kotlinx.coroutines) to C++.

This documentation is organized into five topic directories, each containing its own index and in-depth documents.

---

## 📚 Topic Directories

```
docs/
├── architecture/     # Architectural north star, docking ring, and roadmap
├── suspension/       # Suspend lowering specs, Clang DSL plugin, and IR contracts
├── runtime-and-gc/   # Kotlin/Native runtime integration and GC coordination
├── audits/           # API completeness, transliteration status, and AST metrics
├── guides/           # Upstream conceptual guides, tutorials, and debugging
└── images/           # Visual diagrams, sequence charts, and screenshots
```

---

## 🏛️ [Architecture](architecture/README.md)
Architectural blueprints, north star design principles, and technical roadmaps forming the bridge between Kotlin/Native and C++.

| Document | Purpose |
|:---|:---|
| [porting_north_star.md](architecture/porting_north_star.md) | Authoritative technical playbook for porting Kotlin `kotlinx.coroutines` and Kotlin/Native coroutine lowering to C++. Defines core invariants, ground truth mapping, and transliteration rules. |
| [docking_ring.md](architecture/docking_ring.md) | Architectural blueprint forming a "docking ring" between the Kotlin/Native runtime (GC + coroutine state machine lowering + continuation API) and C++. |
| [coroutines_primitives_north_star.md](architecture/coroutines_primitives_north_star.md) | Fundamental primitives required to implement Kotlin Coroutines in C++, distinguishing Compiler Intrinsics from Library Code. |
| [IMPLEMENTATION_ROADMAP.md](architecture/IMPLEMENTATION_ROADMAP.md) | Required `ast_distance --deep` workflow, generated status and gap inventories, and repair priorities. |
| [research_notes.md](architecture/research_notes.md) | Technical investigation into Kotlin compiler IR lowering (`NativeSuspendFunctionLowering.kt`) and `Cancellable` start machinery. |

---

## ⚡ [Suspension](suspension/README.md)
Lowering mechanics, Clang computed-goto macros, bitcode transformations, and compiler plugin designs.

| Document | Purpose |
|:---|:---|
| [SUSPEND_IMPLEMENTATION.md](suspension/SUSPEND_IMPLEMENTATION.md) | Design and runtime mechanics of the suspend function implementation using Clang computed-goto macros in `src/kotlinx/coroutines/dsl/Suspend.hpp`. |
| [IR_SUSPEND_LOWERING_SPEC.md](suspension/IR_SUSPEND_LOWERING_SPEC.md) | Specification for IR-level suspend lowering matching Kotlin/Native's continuation protocol, resume addresses, and persistent live values. |
| [CLANG_SUSPEND_EXTRACTION.md](suspension/CLANG_SUSPEND_EXTRACTION.md) | Catalog of minimal Clang and LLVM APIs required to extract and implement native `suspend` keyword support. |

---

## 🔄 [Runtime & GC](runtime-and-gc/README.md)
Garbage collection coordination, weak linking, and thread state management with the Kotlin/Native runtime.

| Document | Purpose |
|:---|:---|
| [KOTLIN_GC_BRIDGE_IMPLEMENTATION.md](runtime-and-gc/KOTLIN_GC_BRIDGE_IMPLEMENTATION.md) | Implementation summary of zero-overhead Kotlin Native GC integration for C++ coroutine libraries (`include/kotlinx/coroutines/KotlinGCBridge.hpp`). |
| [KOTLIN_NATIVE_GC_SPECIFICATION.md](runtime-and-gc/KOTLIN_NATIVE_GC_SPECIFICATION.md) | Detailed engineering specification covering thread states (`kRunnable` vs `kNative`), RAII guards, safepoint checking, and benchmarks. |

---

## 📊 [Audits & Status](audits/README.md)
API completeness, module-level parity tracking, namespace validation, and AST distance metrics.

| Category | Key Documents | Description |
|:---|:---|:---|
| **Overview & Lowering** | [IR_PORTING_PARITY_AUDIT.md](audits/IR_PORTING_PARITY_AUDIT.md)<br>[IR_HANDOFF_REVIEW.md](audits/IR_HANDOFF_REVIEW.md)<br>[COMPREHENSIVE_AUDIT_REPORT.md](audits/COMPREHENSIVE_AUDIT_REPORT.md) | Ground truth compiler lowering phases, parity defect analysis, and project completion metrics. |
| **AST Parity Reports** | [port_status_report.md](audits/port_status_report.md)<br>[high_priority_ports.md](audits/high_priority_ports.md)<br>[NEXT_ACTIONS.md](audits/NEXT_ACTIONS.md)<br>[port_lint_proposed_changes.md](audits/port_lint_proposed_changes.md) | Automated AST similarity scoring, symbol deficit tracking, and priority ranking by dependency fanout. |
| **API & Syntax Tracking** | [API_AUDIT.md](audits/API_AUDIT.md)<br>[API_COMPLETENESS_AUDIT.md](audits/API_COMPLETENESS_AUDIT.md)<br>[API_TRANSLATION.md](audits/API_TRANSLATION.md)<br>[NAMESPACE_STRUCTURE_AUDIT.md](audits/NAMESPACE_STRUCTURE_AUDIT.md)<br>[TRANSLITERATION_STATUS.md](audits/TRANSLITERATION_STATUS.md)<br>[SYNTAX_CLEANUP_STATUS.md](audits/SYNTAX_CLEANUP_STATUS.md) | Method-by-method coverage, namespace hierarchy verification, and syntax translation patterns. |
| **Block Audits (01–11)** | [audit_block_01_test_utils.md](audits/audit_block_01_test_utils.md) through [audit_block_11_benchmarks.md](audits/audit_block_11_benchmarks.md) | Deep module-by-module analysis spanning test utils, core common, native platforms, and debug modules. |
| **Action Planning** | [TODO_CHECKLIST.md](audits/TODO_CHECKLIST.md)<br>[CLEANUP_GUIDE.md](audits/CLEANUP_GUIDE.md)<br>[implementation_plan.md](audits/implementation_plan.md) | Prioritized checklists, refactoring guidelines, and milestone sequencing. |

---

## 🧭 [Guides & Tutorials](guides/README.md)
Transliterated upstream Kotlin coroutines conceptual guides, hands-on tutorials, and debugging walkthroughs.

| Section | Guides | Description |
|:---|:---|:---|
| **Foundations** | [coroutines-guide.md](guides/coroutines-guide.md)<br>[coroutines-basics.md](guides/coroutines-basics.md)<br>[coroutines-and-channels.md](guides/coroutines-and-channels.md) | Core concepts, `launch`, `runBlocking`, structured concurrency, and introductory tutorial. |
| **Asynchrony** | [cancellation-and-timeouts.md](guides/cancellation-and-timeouts.md)<br>[composing-suspending-functions.md](guides/composing-suspending-functions.md)<br>[coroutine-context-and-dispatchers.md](guides/coroutine-context-and-dispatchers.md)<br>[flow.md](guides/flow.md)<br>[channels.md](guides/channels.md) | Cooperative cancellation, `async` composition, thread dispatchers, cold Flows, and Channel pipelines. |
| **Coordination** | [exception-handling.md](guides/exception-handling.md)<br>[shared-mutable-state-and-concurrency.md](guides/shared-mutable-state-and-concurrency.md)<br>[select-expression.md](guides/select-expression.md) | Exception transparency, mutex synchronization, and clause selection. |
| **Tooling & Reference** | [debug-coroutines-with-idea.md](guides/debug-coroutines-with-idea.md)<br>[debug-flow-with-idea.md](guides/debug-flow-with-idea.md)<br>[debugging.md](guides/debugging.md)<br>[compatibility.md](guides/compatibility.md) | Debugger walkthroughs, stacktrace recovery, and API stability rules. |

---

## 🚀 Navigation & Getting Started

1. **New to the Project Architecture?**
   - Start with [Architecture North Star](architecture/porting_north_star.md) to understand non-negotiable invariants and file layout rules.
   - Review [The Docking Ring](architecture/docking_ring.md) for the compiler lowering and plugin plan.
2. **Implementing or Porting Coroutines?**
   - Review [IR Porting Parity Audit](audits/IR_PORTING_PARITY_AUDIT.md) for the 5 Golden Rules of transliteration.
   - Check [Immediate Actions](audits/NEXT_ACTIONS.md) and [High Priority Ports](audits/high_priority_ports.md) for blocking dependencies.
   - Consult [Suspend Implementation](suspension/SUSPEND_IMPLEMENTATION.md) for current state-machine patterns.
3. **Integrating with Kotlin/Native?**
   - Read the [Kotlin GC Bridge Specification](runtime-and-gc/KOTLIN_NATIVE_GC_SPECIFICATION.md) for thread states and RAII safety.
4. **Learning Coroutine Concepts?**
   - Explore the [Guides Index](guides/README.md) for comprehensive guides and tutorials.
