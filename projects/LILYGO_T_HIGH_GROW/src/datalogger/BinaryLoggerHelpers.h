#pragma once

#include <Arduino.h>
#include <LittleFS.h>

namespace DATALOG {

/**
 * Helper functions for binary logger file operations
 */
class BinaryLoggerHelpers {
public:
    // Path generation
    static String getMonthDir();
    static String getFilePath();
    static void ensureDirectoryExists(const String& path);
    
    // Header operations
    static bool validateFileHeader(File& file);
    static void writeFileHeader(File& file);
};

}  // namespace DATALOG
