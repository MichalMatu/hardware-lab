#include "SensorCommandQueue.h"

#include "../../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "Sensor"

namespace SENSORS {

QueueHandle_t SensorCommandQueue::_queue = nullptr;

bool SensorCommandQueue::ensureInitialized() {
    if (_queue) {
        return true;
    }

    _queue = xQueueCreate(5, sizeof(SensorTaskCommand));
    if (!_queue) {
        LOGE("CMDQ: failed to create queue");
        return false;
    }

    return true;
}

void SensorCommandQueue::send(SensorTaskCommand cmd) {
    if (!_queue) {
        return;
    }
    xQueueSend(_queue, &cmd, 0);
}

bool SensorCommandQueue::tryReceive(SensorTaskCommand& outCmd) {
    if (!_queue) {
        return false;
    }

    return xQueueReceive(_queue, &outCmd, 0) == pdTRUE;
}

}  // namespace SENSORS
