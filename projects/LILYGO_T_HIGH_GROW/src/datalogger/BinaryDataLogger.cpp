#include "BinaryDataLogger.h"
#include "BinaryFormat.h"
#include "BinaryLoggerHelpers.h"
#include "../system/Logging.h"

#include <time.h>

#undef LOG_TAG
#define LOG_TAG "BinLog"

#define DATA_DIR "/data"
#define MIN_FREE_SPACE 51200  // 50KB

namespace DATALOG {

void BinaryDataLogger::begin() {
    bool fsReady = false;

    // Avoid remount warnings: probe first, mount only if needed
    {
        File probe = LittleFS.open("/");
        if (probe) {
            fsReady = true;
            probe.close();
        }
    }

    if (!fsReady) {
        if (!LittleFS.begin(true)) {
            LOGE("LittleFS mount failed");
            return;
        }
    }

    LOGI("LittleFS mounted");
    
    BinaryLoggerHelpers::ensureDirectoryExists(DATA_DIR);
    checkRotate();
}

void BinaryDataLogger::logSensorData(float temp, float humid, float lux, uint8_t soil, uint16_t salt, float batVolt, uint8_t batPerc) {
    String monthDir = BinaryLoggerHelpers::getMonthDir();
    BinaryLoggerHelpers::ensureDirectoryExists(monthDir);
    
    String filepath = BinaryLoggerHelpers::getFilePath();
    bool isNewFile = !LittleFS.exists(filepath);
    
    File file = LittleFS.open(filepath, "a");
    if (!file) {
        LOGE("Failed to open: %s", filepath.c_str());
        return;
    }
    
    // Write header for new files only
    if (isNewFile) {
        BinaryLoggerHelpers::writeFileHeader(file);
    }
    // Note: File opened in append mode ("a") is already at EOF position
    // No need to seek or validate on every write (pre-release, trusted FS)
    
    // Prepare binary record
    BinaryLogRecord record;
    record.timestamp = static_cast<uint32_t>(time(nullptr));
    record.temp_10x = floatToInt16_10x(temp);
    record.humid_10x = floatToUInt16_10x(humid);
    record.lux = static_cast<uint16_t>(lux);
    record.soil = soil;
    record.salt = salt;
    record.batVolt_mv = floatToMillivolts(batVolt);
    record.batPerc = batPerc;
    
    // Write binary record
    size_t bytesWritten = file.write(reinterpret_cast<const uint8_t*>(&record), sizeof(record));
    file.close();
    
    if (bytesWritten == sizeof(record)) {
        LOGD("Logged to: %s (timestamp=%lu, temp=%d, humid=%u, lux=%u, soil=%u, salt=%u, bat=%u mV/%u%%)",
             filepath.c_str(), record.timestamp, record.temp_10x, record.humid_10x, 
             record.lux, record.soil, record.salt, record.batVolt_mv, record.batPerc);
    } else {
        LOGE("Write failed: wrote %d bytes (expected %d)", bytesWritten, sizeof(record));
    }
}

void BinaryDataLogger::checkRotate() {
    size_t total = LittleFS.totalBytes();
    size_t used = LittleFS.usedBytes();
    size_t free = total - used;
    
    LOGI("Storage: %lu KB total, %lu KB used, %lu KB free",
         total / 1024, used / 1024, free / 1024);
    
    if (free < MIN_FREE_SPACE) {
        LOGW("Low space - rotation needed");
        
        // Find and delete oldest directory
        File root = LittleFS.open(DATA_DIR);
        if (!root) {
            LOGE("Failed to open data dir");
            return;
        }
        
        File file = root.openNextFile();
        String oldest = "";
        
        while (file) {
            if (file.isDirectory()) {
                String name = String(file.name());
                if (oldest == "" || name < oldest) {
                    oldest = name;
                }
            }
            file = root.openNextFile();
        }
        root.close();
        
        if (oldest != "") {
            LOGW("Deleting oldest: %s", oldest.c_str());
            
            // Delete all files in directory
            File dir = LittleFS.open(oldest);
            if (dir) {
                File f = dir.openNextFile();
                while (f) {
                    String path = String(f.name());
                    f.close();
                    if (LittleFS.remove(path)) {
                        LOGD("Deleted: %s", path.c_str());
                    }
                    f = dir.openNextFile();
                }
                dir.close();
            }
            
            if (LittleFS.rmdir(oldest)) {
                LOGI("Removed dir: %s", oldest.c_str());
            }
        }
    }
}

}  // namespace DATALOG
