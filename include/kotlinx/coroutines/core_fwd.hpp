#pragma once
#include <exception>
#include <functional>
#include <optional>
#include <string>

namespace kotlinx { namespace coroutines {

// Canonical forward declarations
class CoroutineContext;
class CoroutineScope;
class Job;
class DeferredBase; // if some files used Deferred under the hood
class ContinuationBase; // generic continuation stub
class CancellationException : public std::exception {};

// Select machinery (common across many files)
class SelectClause; // base marker

template <typename T>
class SelectInstance; // generic select holder

// Common function type aliases used by agents
using ProcessResultFunction = void* (*)(void* /*clause*/, void* /*param*/, void* /*internalResult*/);
using RegistrationFunction = void (*)(void* /*clause*/, void* /*param*/, SelectInstance<void*>* /*select*/);

}} // namespace kotlinx::coroutines
