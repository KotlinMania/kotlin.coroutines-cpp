# Architecture documents

Read the document relevant to the selected source pair. This index is not a
continuation queue, and its linked documents must not be loaded as a bundle.
Current translation priorities and missing code come from Kanban and the generated
`ast_distance --deep` inventories. Faithful library transliteration has priority
over compiler integration and timing validation.

| Document | Contract |
| --- | --- |
| [Flow collection and ownership](flow_collection_ownership.md) | Direct source collection, channel consumption, result boxes and borrowed/owned lifetimes |
| [C++ suspend expressions](cpp_suspend_expression_lowering.md) | Declaration identity, evaluation order, defaults and retained C++ objects |
| [LLVM code generation](llvm_codegen_contracts.md) | Typed contexts, resume addresses, signatures, imports and missing connected consumers |
| [Compiler metadata and collections](compiler_metadata_and_collections.md) | Names, primitive catalogs, lazy sequences and concrete collection dependencies |
| [Native time and workers](native_time_and_worker_contracts.md) | Saturated arithmetic, mark ownership, scheduling and unfinished source boundaries |
| [Docking ring](docking_ring.md) | Standalone C++ and explicitly linked Native interoperability requirements |
| [IR identity and scopes](ir_identity_and_scopes.md) | Detailed compiler declaration and scope design |
| [Porting north star](porting_north_star.md) | Source mapping, Continuation ABI and transliteration invariants |
| [Coroutine primitives](coroutines_primitives_north_star.md) | Compiler intrinsics, continuation APIs and library primitives |
| [Measurement workflow](IMPLEMENTATION_ROADMAP.md) | Required generated inventories and source repair criteria |
| [Compiler research](research_notes.md) | Lowering and cancellable-start reference material |

The focused contract documents describe implemented structure and explicit missing
pieces. They contain no commit chronology or instructions to restart a dependency
chain. Consult actual source and current generated evidence before making changes.
