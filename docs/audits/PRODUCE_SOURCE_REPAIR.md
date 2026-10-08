# Produce source builder and awaitClose repair

Date: 2026-10-07. Source continuation of the Flow Channels/ChannelFlow dependency closure, under Kanban t_8700df29 and source umbrella t_1834dcec. Both cards remain open. Their blocked board state is a dispatch reservation; this active chat remains the sole writer.

The complete pinned common channels/Produce.kt and ChannelCoroutine.kt and the complete C++ Produce pair and ProducerScope.hpp were read before their edits. Native CoroutineContext.kt was also read for the consumed newCoroutineContext call.

## Source changes

- Produce.hpp:159 mirrors Produce.kt:269-283: construct Channel, call the actual new_coroutine_context, construct ProducerCoroutine, register non-null on_completion, start with the actual receiver and return it. The repeated context/dispatcher construction and separate synchronous builder algorithm are removed. Ordinary nonsuspending C++ blocks at :234 enter this same suspend builder.
- Produce.hpp:187,200,211,220 expose the source public forwarding overloads and defaults. Block-first bindings accommodate C++ default-argument rules; their reordered signatures are explicitly documented. The block is required, as it is in Kotlin; the previous null-block shortcut is removed.
- ProducerCoroutine at Produce.hpp:36 keeps the source true/true constructor arguments, isActive forwarding and completion/cancellation branches. Its get_channel at :105 now returns the actual producer through ChannelCoroutine::channel(), as ChannelCoroutine.kt:13 requires. Returning the underlying buffer changed observable object identity.
- Produce.hpp:289 supplies the actual receiver Job, source close registration, cleanup and retained caller to a concrete private frame in Produce.cpp:12. Produce.cpp:58 checks current coroutineContext[Job] identity before entering the source try/finally. An outside call throws without invoking cleanup.
- The source wait at Produce.cpp:25 uses suspend_cancellable_coroutine<void> and the actual retained cancellable continuation. The channel's one-shot close callback consumes/releases that reference when invoked. The source finally runs only after immediate completion, resumed completion or failure, including prompt cancellation. A cleanup exception replaces the previous failure, matching Kotlin finally precedence. Terminating spills release cleanup captures and existing shared receiver owners; borrowed receivers are not adopted.
- The frame uses the existing Native ContinuationImpl, erased Result ABI, persistent address label and mandatory LLVM injection. It introduces no separate decision or resume-dispatch protocol. Generic bindings remain in headers; the concrete suspended algorithm lives in .cpp.
- ProducerScope.hpp now carries canonical source provenance for Produce.kt:10-19 and its channel property. The final deep inventory includes the existing interface in its real source unit; no analyzer code or matching/scoring criteria were changed.

## Regression evidence

Committed before-control f329df9a builds but exits 1 at its deferred-cleanup assertion (produce-before-tests.log). The old stack FinallyGuard ran cleanup while returning COROUTINE_SUSPENDED. The corrected fixture at test_channel_consumption.cpp:945 covers open/already-closed channels, successful/throwing cleanup, exact failure identity, suspended capture retention/release, outside-context rejection, duplicate registration failure and queued prompt cancellation after close is ready. Cancellation cleanup runs exactly once; its exception takes precedence when present.

The fixture at :1056 checks completion registration before start, actual dispatcher identity, suspended rendezvous send, value identity, completion count, source public/default overloads and producer channel identity. The additional identity before-control 9c981813 builds and exits 1 (produce-identity-before-tests.log): its body rejects the old buffer reference, then its completion-order check reports the failed body. b15ede81 restores source receiver identity. Existing ChannelFlow surface and lambda fixtures now start at :1104 and :1127; the earlier audit receipts retain their historical line numbers.

The full core plus test_native_exception_namespace, test_continuation_dispatch, test_channel_as_flow_smoke, test_channel_consumption, test_select_arguments, test_sync and test_sharing_suspension build. All seven CTest executables complete with zero failures in 2.05 seconds. Receipts: build/ir-recovery/produce-final-build.log and produce-final-tests.log.

The expanded fixture and actual Produce.cpp, ChannelFlow.cpp and common/internal/OnUndeliveredElement.cpp compile and execute under AddressSanitizer/UndefinedBehaviorSanitizer with stack-use-after-return detection enabled and no diagnostics. The dependency archive is freshly built but not wholly instrumented. Receipts: produce-sanitizer-build.log and produce-sanitizer-tests.log. The final ProducerScope change is provenance/comment-only and does not alter those executable bodies.

Twenty-seven ranged references in the Produce pair resolve to valid Kotlin source bounds; the pair contains no prohibited comment markers. This verifies provenance bounds, not complete body equivalence.

## Current measured limits

Both final full-root --deep scans complete with exit zero. Receipts: produce-final-library-deep.log and produce-final-compiler-deep.log. Library: 820/2918 function names, 359/560 types, average body similarity 0.26, 123 scoring failures. Produce: 6/6 function names, 2/2 source types, body similarity 0.27. Compiler/prerequisites: 591/7657 functions, 174/1727 types, body similarity 0.36, 24 scoring failures.

These measurements leave the full source objective incomplete. Channel/Flow opaque and erased value text, Native scheduling and other current oracle gaps remain unresolved. The current Continuation ABI adapter and injected frame do not establish complete automatic compiler spilling or shared Native/C++ frame layout. Neither full standalone/MLX GPU acceptance nor actual Native/C++ shared-frame MLX GPU acceptance is established by these focused checks. Source-first work remains the immediate priority.
