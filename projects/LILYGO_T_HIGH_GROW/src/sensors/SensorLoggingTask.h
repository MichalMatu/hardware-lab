#ifndef SensorLoggingTask_h
#define SensorLoggingTask_h

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "model/SensorTypes.h"

class SensorLoggingTask {
public:
    static void begin();
    static void sendCommand(SensorTaskCommand cmd);

    // Single-shot flow (no task): init HW if needed, read sensors, log once
    static bool singleShotReadAndLog();
    
    // Thread-safe snapshot access
    static SensorSnapshot getSnapshot();
    static PhaseStatus getLastReadStatus();
    static PhaseStatus getLastWriteStatus();
    static SensorSnapshot getLastGoodSnapshot();
    static ErrorInfo getLastErrorInfo();
    
    // Global FS mutex for logger + API handlers
    static SemaphoreHandle_t getFsMutex();

private:
    static void taskLoop(void* parameter);
    static bool ensureInitialized();

    static TaskHandle_t _taskHandle;
    static uint32_t _lastReadTime_ms;
    static uint32_t _lastLogTime_ms;
    static bool _initialized;
};

#endif
