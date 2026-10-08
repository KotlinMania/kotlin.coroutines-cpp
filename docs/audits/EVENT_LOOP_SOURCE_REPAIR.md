# EventLoop source repair — 2026-10-07

The complete common EventLoop Kotlin source and C++ header/source were read before
editing. The Native EventLoop source and thread-local counterparts were also read.
The changes translate selected base and thread-local algorithms; EventLoopImplBase
and its Native factory are not complete.

- EventLoop.common.cpp:25 removes the actual queued task before invoking it,
  retaining the task through execution. The source nullable, lazily allocated
  queue and private use-count/shared fields are represented in EventLoop.hpp.
- EventLoop.common.cpp:65 restores the source zero-count assertion and shutdown
  branch. Source comments explain the unconfined and shared-loop distinction.
- EventLoop.common.cpp:87-105 uses one owning thread-local slot for get, current,
  set and reset. Previously get had a separate default slot and set retained only
  a raw pointer. Replacement/reset now release the old owner, and current reports
  the same object returned by get. Borrowed objects are not adopted.
- EventLoop.common.cpp:115,122 translates delay conversion, nonpositive inputs,
  the maximal-millisecond overflow threshold and signed division.

All seven KDoc blocks belonging to the selected EventLoop base are retained.
The whole common Kotlin file contains sixteen blocks; this is not whole-file
documentation coverage. C++ documentation references now use process_next_event,
CoroutineDispatcher::is_dispatch_needed, Dispatchers::get_unconfined, LLONG_MAX
and the actual run_blocking(context, block) spelling. Narrative is preserved.
The source internal-API notice remains ordinary prose. Twenty-nine provenance
ranges were checked for existing source files and valid bounds; that establishes
reference validity, not algorithmic equivalence.

Verification:

- EventLoop.common.cpp strict syntax compilation with Clang C++20 and
  -Wall -Wextra -Wpedantic -Werror: exit 0.
- A fresh bounded runtime fixture compiled the changed EventLoop.common.cpp and
  linked the existing core archive for unchanged dependencies: build and execution
  exit 0. It checks thread-local identity/isolation, retained owners and release
  on replacement/reset, queued task/resource release, nested unconfined counts,
  delay overflow boundaries and signed conversion. Receipts:
  build/ir-recovery/event-loop-retained-check-build.log and
  build/ir-recovery/event-loop-retained-check-run.log.
- Actual coroutine-start fixture strict syntax remains unsuccessful on three
  unused dependency parameters in JobSupport and CancellableContinuationImpl.
  Receipt: build/ir-recovery/event-loop-start-fixture-strict.log.
- Full-root deep evidence is refreshed under project-wide/library and compiler.
  EventLoop.common has 13/38 function bodies and 2/10 types matched, body similarity
  0.19. Symbol presence and this bounded runtime fixture do not establish completion.

Remaining source gaps include the invented empty base dispatch, construction of
a plain EventLoop instead of the actual Native EventLoopImpl factory, the custom
BlockingEventLoop implementation, missing limited_parallelism override, and the
Native worker/queue/timer algorithms. The empty dispatch's existing claim of Native
source equivalence is false. These are implementation defects, not intentional
source-empty hooks. Whole-core strict rebuilding, complete compiler lowering and
both standalone/Native MLX GPU acceptance paths remain unverified.
