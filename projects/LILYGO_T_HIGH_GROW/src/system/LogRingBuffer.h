/**
 * @file LogRingBuffer.h
 * @brief Thread-safe ring buffer for in-memory log storage
 * 
 * Provides a fixed-size circular buffer for storing recent log entries
 * with mutex protection for concurrent access. Designed for ESP32 DRAM
 * constraints - avoids dynamic allocations in hot path.
 */

#ifndef LogRingBuffer_h
#define LogRingBuffer_h

#include <Arduino.h>
#include <array>
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace LOG {

/**
 * Single log entry (POD struct, no String to avoid fragmentation).
 * Fixed-size fields keep memory predictable across buffer resizes.
 */
struct Line {
    uint32_t timestampMs{0};
    char levelChar{'N'};           // E/W/I/D/V/N
    char tag[16]{};                // Tag (e.g. "Sensor", "WiFi") - fixed size to avoid heap fragmentation
    char message[192]{};           // Message buffer (fixed size)
};

/**
 * Thread-safe circular buffer for log lines.
 * Uses FreeRTOS mutex for concurrent access protection.
 */
class RingBuffer {
public:
    static constexpr size_t kFixedCapacity = 20;
    /**
     * Initialize ring buffer with mutex.
     * Must be called once during system initialization.
     * 
     * @param capacity Initial buffer capacity
     */
    static void begin(uint16_t capacity);

    /**
     * Append a new log line to the ring buffer.
     * Thread-safe. Non-blocking with short timeout.
     * 
     * @param levelLabel Single-char level label (E/W/I/D/V)
     * @param tag Log tag
     * @param message Message text (will be truncated if too long)
     */
    static void append(const char *levelLabel, const char *tag, const char *message);

    /**
     * Get last N lines from ring buffer.
     * Thread-safe. Returns oldest-to-newest order.
     * 
     * @param maxLines Maximum number of lines to return
     * @return Vector of log lines (may be fewer than maxLines)
     */
    static std::vector<Line> tail(size_t maxLines);

    /**
     * Clear all entries in the ring buffer.
     * Thread-safe.
     */
    static void clear();

    /**
     * Resize ring buffer capacity.
     * Forces rebuild+swap to release heap memory when shrinking.
     * Thread-safe.
     * 
     * @param newSize New capacity (0 = use default)
     */
    static void resize(uint16_t newSize);

    /**
     * Get current buffer size.
     * @return Number of entries stored (0 to capacity)
     */
    static size_t count() { return _count; }

    /**
     * Get buffer capacity.
     * @return Maximum number of entries
     */
    static size_t capacity() { return kFixedCapacity; }

private:
    static std::array<Line, kFixedCapacity> _buffer;
    static size_t _head;
    static size_t _count;
    static SemaphoreHandle_t _mutex;

    // No instances - static only
    RingBuffer() = delete;
};

}  // namespace LOG

#endif
