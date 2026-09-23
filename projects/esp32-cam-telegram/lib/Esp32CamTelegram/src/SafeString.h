#pragma once

#include <Arduino.h>
#include <stddef.h>

inline void copyText(char* dest, size_t destLen, const char* src) {
  if (destLen == 0) {
    return;
  }
  if (src == nullptr) {
    dest[0] = '\0';
    return;
  }
  strlcpy(dest, src, destLen);
}

inline void copyText(char* dest, size_t destLen, const String& src) {
  copyText(dest, destLen, src.c_str());
}

inline bool hasText(const char* value) {
  return value != nullptr && value[0] != '\0';
}
