#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
// #include "kotlinx/coroutines/DispatchedContinuation.hpp" // Cycle breaker

// This file now aggregates the implementation details that were previously inline.
