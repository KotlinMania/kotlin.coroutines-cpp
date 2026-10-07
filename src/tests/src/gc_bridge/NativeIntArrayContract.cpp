// Real Native objects cross these test entry points without storage conversion.
#include "kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/collections/NativeArrayUtil.hpp"
extern "C" std::int32_t NativeIntArray_cppGet(ObjHeader* array,std::int32_t index) {
  ObjHolder owner(array); return Kotlin_IntArray_get(owner.obj(),index);
}
extern "C" void NativeIntArray_cppSet(ObjHeader* array,std::int32_t index,std::int32_t value) {
  ObjHolder owner(array); Kotlin_IntArray_set(owner.obj(),index,value);
}
extern "C" std::int32_t NativeIntArray_cppLength(ObjHeader* array) {
  ObjHolder owner(array); return Kotlin_IntArray_getArrayLength(owner.obj());
}
extern "C" void NativeIntArray_cppFill(ObjHeader* array,std::int32_t from,std::int32_t to,std::int32_t value) {
  ObjHolder owner(array); Kotlin_IntArray_fillImpl(owner.obj(),from,to,value);
}
extern "C" void NativeIntArray_cppCopy(ObjHeader* array,std::int32_t from,ObjHeader* destination,std::int32_t to,std::int32_t count) {
  ObjHolder owner(array); ObjHolder held_destination(destination);
  Kotlin_IntArray_copyImpl(owner.obj(),from,held_destination.obj(),to,count);
}
