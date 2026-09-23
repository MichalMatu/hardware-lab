#include "BinaryLoggerHelpers.h"
#include "BinaryFormat.h"
#include "../system/Logging.h"

#include <time.h>

#undef LOG_TAG
#define LOG_TAG "BinLog"

#define DATA_DIR "/data"

namespace DATALOG {

String BinaryLoggerHelpers::getMonthDir() {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    char buffer[8];
    strftime(buffer, sizeof(buffer), "%Y-%m", timeinfo);
    return String(DATA_DIR) + "/" + String(buffer);
}

String BinaryLoggerHelpers::getFilePath() {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    char monthBuf[8], dateBuf[11];
    strftime(monthBuf, sizeof(monthBuf), "%Y-%m", timeinfo);
    strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", timeinfo);
    
    char path[48];
    snprintf(path, sizeof(path), "%s/%s/%s.bin", DATA_DIR, monthBuf, dateBuf);
    return String(path);
}

void BinaryLoggerHelpers::ensureDirectoryExists(const String& path) {
    if (!LittleFS.exists(path)) {
        LittleFS.mkdir(path);
        LOGI("Created dir: %s", path.c_str());
    }
}

bool BinaryLoggerHelpers::validateFileHeader(File& file) {
    if (file.size() < sizeof(BinaryFileHeader)) {
        return false;
    }
    
    BinaryFileHeader header;
    file.seek(0);
    size_t bytesRead = file.read(reinterpret_cast<uint8_t*>(&header), sizeof(header));
    
    if (bytesRead != sizeof(header)) {
        return false;
    }
    
    if (header.magic != BINARY_MAGIC) {
        LOGE("Invalid magic: 0x%08X (expected 0x%08X)", header.magic, BINARY_MAGIC);
        return false;
    }
    
    if (header.version != BINARY_VERSION) {
        LOGW("Version mismatch: %d (expected %d)", header.version, BINARY_VERSION);
        return false;
    }
    
    if (header.recordSize != BINARY_RECORD_SIZE) {
        LOGE("Record size mismatch: %d (expected %d)", header.recordSize, BINARY_RECORD_SIZE);
        return false;
    }
    
    return true;
}

void BinaryLoggerHelpers::writeFileHeader(File& file) {
    BinaryFileHeader header;
    header.magic = BINARY_MAGIC;
    header.version = BINARY_VERSION;
    header.recordSize = BINARY_RECORD_SIZE;
    header.reserved = 0;
    
    file.write(reinterpret_cast<const uint8_t*>(&header), sizeof(header));
    LOGD("Wrote file header (magic=0x%08X, version=%d, recordSize=%d)", 
         header.magic, header.version, header.recordSize);
}

}  // namespace DATALOG
