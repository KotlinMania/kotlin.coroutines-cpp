// Test fixture: assertions belong here, not in production reference operations.
#include "kotlinx/coroutines/KotlinGCBridge.hpp"
#include "kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/native/Runtime.hpp"
#include <cassert>
#include <cstdint>

extern "C" ObjHeader* NativeReference_create(std::int32_t index, ObjHeader** result);
extern "C" void NativeReference_collect();
extern "C" bool NativeReference_alive(std::int32_t index);
extern "C" std::int32_t NativeReference_hash(ObjHeader* object);
extern "C" std::int32_t Kotlin_Any_hashCode(const ObjHeader* object);

extern "C" std::int32_t NativeReference_cppHashes() {
  std::int32_t observations = 0;
  const auto compare = [&](ObjHeader* object) {
    const auto translated = kotlin::native::identity_hash_code(object);
    assert(translated == Kotlin_Any_hashCode(object));
    ++observations;
    assert(translated == NativeReference_hash(object));
    ++observations;
  };
  compare(nullptr);
  {
    ObjHolder a;
    ObjHolder b;
    NativeReference_create(0, a.slot());
    NativeReference_create(1, b.slot());
    assert(a.obj() != b.obj());
    compare(a.obj());
    compare(b.obj());
    const auto a_identity = kotlin::native::identity_hash_code(a.obj());
    const auto b_identity = kotlin::native::identity_hash_code(b.obj());
    NativeReference_collect();
    assert(NativeReference_alive(0) && NativeReference_alive(1));
    compare(a.obj());
    compare(b.obj());
    assert(kotlin::native::identity_hash_code(a.obj()) == a_identity);
    assert(kotlin::native::identity_hash_code(b.obj()) == b_identity);
  }
  NativeReference_collect();
  assert(!NativeReference_alive(0) && !NativeReference_alive(1));
  return observations;
}

extern "C" std::int32_t NativeReference_cppRoots() {
  FrameOverlay* const previous = getCurrentFrame();
  {
    ObjHolder a;
    FrameOverlay* const outer = getCurrentFrame();
    assert(outer->previous == previous && outer->parameters == 0);
    assert(outer->count == sizeof(ObjHolder) / sizeof(void*));
    NativeReference_create(0, a.slot());
    NativeReference_collect();
    assert(NativeReference_alive(0));
    {
      ObjHolder b(a.obj());
      assert(getCurrentFrame()->previous == outer);
      assert(b.obj() == a.obj());
    }
    assert(getCurrentFrame() == outer);
    try {
      ObjHolder nested(a.obj());
      throw 71;
    } catch (int value) { assert(value == 71); }
    assert(getCurrentFrame() == outer);
    a.clear();
    NativeReference_collect();
    assert(!NativeReference_alive(0));
  }
  assert(getCurrentFrame() == previous);
  {
    // A real shadow-stack root slot; this does not prove heap/global registration.
    ObjHolder location;
    ObjHolder a;
    ObjHolder b;
    ObjHolder old;
    NativeReference_create(1, a.slot());
    NativeReference_create(2, b.slot());
    UpdateVolatileHeapRef(location.slot(), a.obj());
    a.clear();
    NativeReference_collect();
    assert(NativeReference_alive(1));
    assert(!CompareAndSetVolatileHeapRef(location.slot(), nullptr, b.obj()));
    CompareAndSwapVolatileHeapRef(location.slot(), nullptr, b.obj(), old.slot());
    assert(old.obj() == location.obj());
    assert(CompareAndSetVolatileHeapRef(location.slot(), old.obj(), b.obj()));
    old.clear();
    NativeReference_collect();
    assert(!NativeReference_alive(1) && NativeReference_alive(2));
    GetAndSetVolatileHeapRef(location.slot(), nullptr, old.slot());
    assert(old.obj() == b.obj());
    b.clear();
    NativeReference_collect();
    assert(NativeReference_alive(2));
    old.clear();
    NativeReference_collect();
    assert(!NativeReference_alive(2));
  }
  assert(getCurrentFrame() == previous);
  Kotlin_mm_switchThreadStateNative();
  Kotlin_mm_switchThreadStateRunnable();
  Kotlin_mm_safePointFunctionPrologue();
  Kotlin_mm_safePointWhileLoopBody();
  assert(getCurrentFrame() == previous);
  return 10;
}

extern "C" ObjHeader* NativeReference_cppReturn(std::int32_t index, ObjHeader** result) {
  ObjHolder held;
  NativeReference_create(index, held.slot());
  NativeReference_collect();
  assert(NativeReference_alive(index));
  UpdateReturnRef(result, held.obj());
  return held.obj();
}
