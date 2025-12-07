#pragma once
#include <exception>
#include <functional>
#include <optional>
#include <string>

namespace kotlinx { namespace coroutines {

struct Unit {};

// Canonical forward declarations
struct CoroutineContext;
struct CoroutineScope;
struct Job;
struct JobSupport;
struct CoroutineDispatcher;
struct CancellableContinuationBase;
template <typename T> struct CancellableContinuation;

// Channels
template <typename E> struct SendChannel;
template <typename E> struct ReceiveChannel;
template <typename E> struct Channel;
struct ChannelIterator;

template <typename T> class Deferred; // canonical Deferred
class DeferredBase; // if some files used Deferred under the hood
class ContinuationBase; // generic continuation stub
class CompletionHandler;
class DisposableHandle; // Handler for cancellation/diposal
template <typename T> class Result; // Kotlin Result wrapper
class CancellationException;
using Throwable = std::exception;

// Select machinery (common across many files)
class SelectClause; // base marker
class SelectClause0;
template <typename Q> class SelectClause1;
template <typename P, typename Q> class SelectClause2;
template <typename T> class SelectInstance; // generic select holder

// Common function type aliases used by agents
using ProcessResultFunction = void* (*)(void* /*clause*/, void* /*param*/, void* /*internalResult*/);
using RegistrationFunction = void (*)(void* /*clause*/, void* /*param*/, SelectInstance<void*>* /*select*/);

}} // namespace kotlinx::coroutines
