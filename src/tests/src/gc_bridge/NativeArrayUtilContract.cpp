// Real Native objects cross these test entry points; assertions stay in tests.
#include "kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/collections/NativeArrayUtil.hpp"
#include <cassert>

extern "C" void NativeArray_cppResetAt(ObjHeader* array, std::int32_t index) {
  ObjHolder owner(array);
  kotlin::collections::reset_at(owner.obj(), index);
}
extern "C" void NativeArray_cppResetRange(ObjHeader* array, std::int32_t from, std::int32_t to) {
  ObjHolder owner(array);
  kotlin::collections::reset_range(owner.obj(), from, to);
}
extern "C" void NativeArray_cppFill(ObjHeader* array, std::int32_t from,
                                    std::int32_t to, ObjHeader* value) {
  ObjHolder owner(array);
  ObjHolder held_value(value);
  Kotlin_Array_fillImpl(owner.obj(), from, to, held_value.obj());
}
extern "C" void NativeArray_cppCopy(ObjHeader* array, std::int32_t from,
                                    ObjHeader* destination, std::int32_t to, std::int32_t count) {
  ObjHolder owner(array);
  ObjHolder held_destination(destination);
  Kotlin_Array_copyImpl(owner.obj(), from, held_destination.obj(), to, count);
}
extern "C" ObjHeader* NativeArray_cppGet(ObjHeader* array, std::int32_t index, ObjHeader** result) {
  ObjHolder owner(array);
  ObjHolder value;
  assert(Kotlin_Array_getArrayLength(owner.obj()) > index);
  Kotlin_Array_get(owner.obj(), index, value.slot());
  UpdateReturnRef(result, value.obj());
  return value.obj();
}
