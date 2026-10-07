/*
 * Copyright 2010-2018 JetBrains s.r.o.
 * Licensed under the Apache License, Version 2.0.
 */
// port-lint: source kotlin-native/runtime/src/main/cpp/Memory.h
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:141-312
#pragma once

#include <cstdint>

// Native runtime object/type identities; definitions belong to that runtime.
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:45-46
struct ObjHeader;
// Transliterated from: kotlin-native/runtime/src/main/cpp/TypeInfo.h:20-20
struct TypeInfo;
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:141-141
struct FrameOverlay;

// NOTE(port): These are the actual Native C ABI names. Runtime definitions are
// mandatory at link time. Object-returning entries take the caller's root slot;
// opaque forward declarations do not substitute C++ storage for Native objects.
extern "C" {
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:180-180
ObjHeader* AllocInstance(const TypeInfo* type_info, ObjHeader** result) __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:185-185
void RegisterGlobal(ObjHeader** location) __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:214-214
void ZeroStackRef(ObjHeader** location) __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:216-216
void UpdateStackRef(ObjHeader** location, const ObjHeader* object) __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:218-218
void UpdateHeapRef(ObjHeader** location, const ObjHeader* object) __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:220-220
void UpdateVolatileHeapRef(ObjHeader** location, const ObjHeader* object) __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:221-221
ObjHeader* CompareAndSwapVolatileHeapRef(ObjHeader** location, ObjHeader* expected_value,
    ObjHeader* new_value, ObjHeader** result) __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:222-222
bool CompareAndSetVolatileHeapRef(ObjHeader** location, ObjHeader* expected_value,
    ObjHeader* new_value) __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:223-223
ObjHeader* GetAndSetVolatileHeapRef(ObjHeader** location, ObjHeader* new_value,
    ObjHeader** result) __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:227-227
void UpdateReturnRef(ObjHeader** return_slot, const ObjHeader* object) __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:229-229
void EnterFrame(ObjHeader** start, int parameters, int count) __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:231-231
void LeaveFrame(ObjHeader** start, int parameters, int count) __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:233-233
void SetCurrentFrame(ObjHeader** start) __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:234-234
FrameOverlay* getCurrentFrame() __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:254-254
void Kotlin_mm_switchThreadStateNative() __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:256-256
void Kotlin_mm_switchThreadStateRunnable() __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:263-263
void Kotlin_mm_safePointFunctionPrologue() __attribute__((nothrow));
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:264-264
void Kotlin_mm_safePointWhileLoopBody() __attribute__((nothrow));
}

// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:272-277
struct FrameOverlay {
  FrameOverlay* previous;
  // As they go in pair, sizeof(FrameOverlay) % sizeof(void*) == 0 is always held.
  std::int32_t parameters;
  std::int32_t count;
};

// Class holding reference to an object, holding object during C++ scope.
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:281-311
class ObjHolder {
 public:
  // Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:283-285
  ObjHolder();
  // Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:287-290
  explicit ObjHolder(const ObjHeader* obj);
  // Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:292-294
  ~ObjHolder();
  // Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:296-296
  ObjHeader* obj();
  // Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:298-298
  const ObjHeader* obj() const;
  // Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:300-302
  ObjHeader** slot();
  // Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:304-304
  void clear();
 private:
  // Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:307-307
  ObjHeader** frame();
  FrameOverlay frame_;
  ObjHeader* obj_;
};
