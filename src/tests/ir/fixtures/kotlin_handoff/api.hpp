#pragma once
#include <kotlinx/coroutines/ContinuationImpl.hpp>
#include <kotlinx/coroutines/dsl/Suspend.hpp>
#include <string>
using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::dsl;
[[clang::annotate("suspend")]] void* kotlin_value(int value, std::shared_ptr<Continuation<void*>> completion);
void* cpp_value(int value, std::shared_ptr<Continuation<void*>> completion);
