#pragma once
#include <Arduino.h>

// Lightweight, static variable registry for debug/inspection.
// No heap-heavy std::vector/std::function to keep ESP32 stable.

typedef String (*VarGetter)();

struct VarItem {
  const char* key;       // JSON/table key
  VarGetter   get;       // returns a JSON-ready value string (already quoted or raw)
  bool        secret;    // true => mask value in UI (API already masked)
  const char* group;     // simple grouping label
};

extern const VarItem VARS[];
extern const size_t VARS_COUNT;