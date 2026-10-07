/*
 * Copyright 2010-2018 JetBrains s.r.o.
 * Licensed under the Apache License, Version 2.0.
 */
// port-lint: source kotlin-native/runtime/src/main/cpp/Memory.h
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:281-311
#include "KotlinGCBridge.hpp"

// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:283-285
ObjHolder::ObjHolder() : obj_(nullptr) {
  EnterFrame(frame(), 0, sizeof(*this) / sizeof(void*));
}
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:287-290
ObjHolder::ObjHolder(const ObjHeader* obj) {
  EnterFrame(frame(), 0, sizeof(*this) / sizeof(void*));
  ::UpdateStackRef(slot(), obj);
}
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:292-294
ObjHolder::~ObjHolder() {
  LeaveFrame(frame(), 0, sizeof(*this) / sizeof(void*));
}
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:296-296
ObjHeader* ObjHolder::obj() { return obj_; }
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:298-298
const ObjHeader* ObjHolder::obj() const { return obj_; }
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:300-302
ObjHeader** ObjHolder::slot() { return &obj_; }
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:304-304
void ObjHolder::clear() { ::ZeroStackRef(&obj_); }
// Transliterated from: kotlin-native/runtime/src/main/cpp/Memory.h:307-307
ObjHeader** ObjHolder::frame() { return reinterpret_cast<ObjHeader**>(&frame_); }
