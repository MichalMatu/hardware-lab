/**
 * @file LogRingBuffer.cpp
 * @brief Implementation of thread-safe ring buffer for logs
 */

#include "LogRingBuffer.h"
#include <string.h>

namespace LOG {

// Static member initialization
std::array<Line, RingBuffer::kFixedCapacity> RingBuffer::_buffer;
size_t RingBuffer::_head = 0;
size_t RingBuffer::_count = 0;
SemaphoreHandle_t RingBuffer::_mutex = nullptr;

void RingBuffer::begin(uint16_t capacity) {
    (void)capacity;
    if (!_mutex) {
        _mutex = xSemaphoreCreateMutex();
    }

    // Ensure deterministic state on boot.
    _head = 0;
    _count = 0;
    _buffer.fill(Line{});
}

void RingBuffer::append(const char *levelLabel, const char *tag, const char *message) {
    if (!_mutex) {
        return;
    }

    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
        return;  // Skip if can't acquire quickly
    }

    Line line{};
    line.timestampMs = millis();
    line.levelChar = (levelLabel && levelLabel[0]) ? levelLabel[0] : 'N';
    
    if (tag) {
        strncpy(line.tag, tag, sizeof(line.tag) - 1);
        line.tag[sizeof(line.tag) - 1] = '\0';
    } else {
        line.tag[0] = '\0';
    }

    if (message) {
        strncpy(line.message, message, sizeof(line.message) - 1);
        line.message[sizeof(line.message) - 1] = '\0';
    } else {
        line.message[0] = '\0';
    }

    // Store in ring buffer
    _buffer[_head] = line;
    _head = (_head + 1) % _buffer.size();
    if (_count < _buffer.size()) {
        _count++;
    }

    xSemaphoreGive(_mutex);
}

std::vector<Line> RingBuffer::tail(size_t maxLines) {
    std::vector<Line> out;
    if (!_mutex || maxLines == 0) {
        return out;
    }

    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) != pdTRUE) {
        return out;
    }

    size_t toRead = (_count < maxLines) ? _count : maxLines;
    out.reserve(toRead);

    // Calculate start position (oldest entry)
    size_t start = (_head + _buffer.size() - _count) % _buffer.size();

    for (size_t i = 0; i < toRead; i++) {
        size_t idx = (start + i) % _buffer.size();
        out.push_back(_buffer[idx]);
    }

    xSemaphoreGive(_mutex);
    return out;
}

void RingBuffer::clear() {
    if (!_mutex) {
        return;
    }

    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        _head = 0;
        _count = 0;
        _buffer.fill(Line{});
        xSemaphoreGive(_mutex);
    }
}

void RingBuffer::resize(uint16_t newSize) {
    (void)newSize;
    // Fixed-capacity buffer (no heap). Intentionally a no-op.
}

}  // namespace LOG
