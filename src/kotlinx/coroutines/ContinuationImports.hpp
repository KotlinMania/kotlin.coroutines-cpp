#pragma once
// NOTE(port): Import the consumed kotlin.coroutines declarations into the library.
namespace kotlinx::coroutines {
using kotlin::coroutines::resume;
using kotlin::coroutines::resume_with_exception;
using kotlin::coroutines::make_continuation;
using kotlin::coroutines::to_void_continuation;
}
