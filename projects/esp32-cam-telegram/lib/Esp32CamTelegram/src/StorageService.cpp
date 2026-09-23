#include "StorageService.h"

#include <FS.h>
#include <SD_MMC.h>
#include <time.h>

#include "Logger.h"
#include "SafeString.h"

bool StorageService::begin(bool enabled) {
  mounted_ = false;
  if (!enabled) {
    LOGI("SD", "disabled by config");
    return false;
  }

  if (!SD_MMC.begin("/sdcard", true)) {
    LOGW("SD", "mount failed");
    return false;
  }
  if (SD_MMC.cardType() == CARD_NONE) {
    LOGW("SD", "no card");
    SD_MMC.end();
    return false;
  }

  mounted_ = true;
  ensureDir("/photos");
  ensureDir("/photos/manual");
  ensureDir("/photos/motion");
  ensureDir("/logs");
  LOGI("SD", "mounted in 1-bit mode");
  return true;
}

bool StorageService::mounted() const {
  return mounted_;
}

bool StorageService::savePhoto(const char* category, const uint8_t* data, size_t length, char* outPath, size_t outPathLen) {
  if (outPathLen > 0) {
    outPath[0] = '\0';
  }
  if (!mounted_ || data == nullptr || length == 0) {
    return false;
  }

  char path[96];
  makePhotoPath(category, path, sizeof(path));

  File file = SD_MMC.open(path, FILE_WRITE);
  if (!file) {
    LOGW("SD", "open photo failed");
    return false;
  }
  const size_t written = file.write(data, length);
  file.close();
  if (written != length) {
    LOGW("SD", "short photo write");
    return false;
  }

  copyText(outPath, outPathLen, path);
  LOGI("SD", path);
  return true;
}

bool StorageService::readFile(const char* path, Stream& out) {
  if (!mounted_ || !hasText(path)) {
    return false;
  }
  File file = SD_MMC.open(path, FILE_READ);
  if (!file) {
    return false;
  }

  uint8_t buffer[512];
  while (file.available()) {
    const int readBytes = file.read(buffer, sizeof(buffer));
    if (readBytes <= 0) {
      break;
    }
    out.write(buffer, static_cast<size_t>(readBytes));
  }
  file.close();
  return true;
}

bool StorageService::ensureDir(const char* path) {
  if (SD_MMC.exists(path)) {
    return true;
  }
  return SD_MMC.mkdir(path);
}

void StorageService::makePhotoPath(const char* category, char* out, size_t outLen) {
  const char* folder = (strcmp(category, "motion") == 0) ? "motion" : "manual";
  time_t now = time(nullptr);
  struct tm timeinfo = {};
  if (now > 1700000000 && localtime_r(&now, &timeinfo) != nullptr) {
    snprintf(out, outLen, "/photos/%s/%04d%02d%02d_%02d%02d%02d.jpg",
             folder,
             timeinfo.tm_year + 1900,
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday,
             timeinfo.tm_hour,
             timeinfo.tm_min,
             timeinfo.tm_sec);
    return;
  }
  snprintf(out, outLen, "/photos/%s/%lu.jpg", folder, millis());
}
