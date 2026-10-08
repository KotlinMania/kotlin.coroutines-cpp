// NOTE(port): Clang regression for defaults and continuation ownership.
#include "kotlinx/coroutines/dsl/Coroutines.hpp"
#include <cassert>
#include <iostream>
using namespace kotlinx::coroutines;
const int* pending_value;
std::shared_ptr<Continuation<void*>> pending_frame;
[[clang::annotate("suspend")]] void* external_ref(const int& value, std::shared_ptr<Continuation<void*>> completion) {
 pending_value = &value; pending_frame = std::move(completion);
 return intrinsics::get_COROUTINE_SUSPENDED();
}
namespace defaults {
int calls=0; int make(){++calls;return 41;} int identity(int value){return value;}
[[clang::annotate("suspend"), clang::annotate("kxs_implicit_continuation")]]
__attribute__((error("implicit_value requires suspend lowering"))) void* implicit_value(int value = [] { const int local=41; const int made=make(); return identity(made)+local-41; }());
void* implicit_value(int value, std::shared_ptr<Continuation<void*>> completion) {
 static int retained_value;
 retained_value=value; pending_value=&retained_value;
 pending_frame=std::move(completion); return intrinsics::get_COROUTINE_SUSPENDED();
}
}
[[suspend]] void* reference_tail(std::shared_ptr<Continuation<void*>> completion) {
 void* raw = defaults::implicit_value();
 return raw;
}
struct Done : Continuation<void*> {
 int calls=0; int value=0;
 std::shared_ptr<CoroutineContext> get_context() const override { return EmptyCoroutineContext::instance(); }
 void resume_with(Result<void*> result) override {
  std::unique_ptr<int> owned(static_cast<int*>(result.get_or_throw())); value=*owned; ++calls;
 }
};
int main() {
 auto done=std::make_shared<Done>();
 auto outcome=reference_tail(done);
 assert(intrinsics::is_coroutine_suspended(outcome));
 assert(pending_frame.get()!=done.get() && done->calls==0);
 auto frame=std::move(pending_frame);
 frame->resume_with(Result<void*>::success(new int(*pending_value+1)));
 assert(done->calls==1 && done->value==42);
 assert(defaults::calls==1);
 std::cout<<"direct implicit continuation:42\n";
}
