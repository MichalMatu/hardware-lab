#ifndef RTCTask_h
#define RTCTask_h

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**
 * @brief FreeRTOS task for RTC DS3231 management
 * 
 * Features:
 * - Lekka pętla sync: reaguje na nowe SNTP sync i zapisuje RTC nie częściej niż co 60s
 * - Opcjonalne debug logi z poziomu kodu (readAndPrint dostępne ręcznie)
 * - Non-blocking, runs on dedicated FreeRTOS task
 * - Initialized via begin() like other tasks (ButtonTask, SensorLoggingTask)
 */
class RTCTask {
public:
    /**
     * @brief Initialize and start RTC management task
     * 
    * Tworzy zadanie FreeRTOS, które:
    * - obserwuje zakończone sync SNTP i aktualizuje RTC (min. co 60s, start >=30s)
    * - nie wykonuje stałych odczytów/printów czasu w tle (tylko przy sync)
     * 
     * @return true if task created successfully, false otherwise
     */
    static bool begin();
    
    /**
     * @brief Stop RTC task (cleanup)
     */
    static void stop();

private:
    static TaskHandle_t _taskHandle;
    
    /**
     * @brief Main task loop
     * Handles periodic RTC reads and NTP synchronization
     */
    static void taskLoop(void* parameter);
};

#endif
