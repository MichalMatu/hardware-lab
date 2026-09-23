#pragma once

#include <Arduino.h>

class StorageService {
 public:
  bool begin(bool enabled);
  bool mounted() const;
  bool savePhoto(const char* category, const uint8_t* data, size_t length, char* outPath, size_t outPathLen);
  bool readFile(const char* path, Stream& out);

 private:
  bool ensureDir(const char* path);
  void makePhotoPath(const char* category, char* out, size_t outLen);

  bool mounted_ = false;
};
