# Flow collection and ownership

## Source contract

`tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/flow/` supplies
`Channels.kt`, `Builders.kt`, `operators/Collect.kt`, `operators/Limit.kt`,
`operators/Transform.kt`, `operators/Merge.kt` and `internal/ChannelFlow.kt`.
The C++ counterparts live under `src/kotlinx/coroutines/flow/`.
Branch order, cancellation, exception precedence and diagnostic text follow those
sources. Similar-purpose algorithms are insufficient for source parity.

## ChannelFlow collection

The producer callable invokes `collect_to` directly. Operator collection uses
its source sending collector. Undispatched collection obtains the collector for
the original context and invokes the source context operation. Scoped collection
calls `emit_all` with the actual channel returned by `produce_impl`.

The owning `emit_all` overload forwards that channel into `emit_all_impl` with
`consume=true`. Consumption and channel cleanup belong to that source loop.
The collection entry retains an existing flow owner. Raw receivers and raw
collectors remain borrowed. Retaining a pointer does not adopt its pointee.

`collect_channel_flow` has no remaining production references and is removed.
Internal Merge consumers call their source operations directly. The generic
collection bodies remain in headers where C++ template instantiation requires it;
concrete implementations belong in their matching source files.

## Source authoring and result ownership

Translated collection bodies use the existing Continuation ABI and suspend DSL.
Compiler lowering supplies suspension state and retained locals. Source code must
not introduce a second handwritten coroutine frame to bypass incomplete lowering.

Erased operation results require an explicit unbox/free policy. Transform
collectors consume owned operation-result boxes before emission and keep each
collection's accumulator or buffer separate. Existing shared owners remain shared;
borrowed arguments remain borrowed. These rules also apply to captured callables,
producer scopes and cancellation handlers.

## Implementation limits

Typed callable, scope and context adaptations still need source reconciliation.
Public APIs, comments and examples require comparison with their complete Kotlin
counterparts; removing handwritten frames alone does not complete translation.
Local collector declarations and annotated calls inside scope lambdas encounter
frontend integration diagnostics. Generated frame parsing and dependency
warnings also prevent successful compilation of affected consumers.

The current source does not establish runtime retention, cleanup, resumed failure
or cancellation for every collection path. Compiler diagnostics do not authorize
changing upstream algorithms. Missing source remains a translation task.
