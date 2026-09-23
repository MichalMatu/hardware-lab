#include "ButtonTask.h"
#include "../../config/AppConfig.h"
#include "../../system/Logging.h"

#undef LOG_TAG
#define LOG_TAG "Button"

using namespace BTN;

// Static members
TaskHandle_t ButtonTask::_taskHandle = nullptr;
QueueHandle_t ButtonTask::_eventQueue = nullptr;
uint8_t ButtonTask::_pin = 0;
ButtonCallback ButtonTask::_callback = nullptr;
volatile unsigned long ButtonTask::_lastInterruptTime = 0;
volatile bool ButtonTask::_buttonPressed = false;

void IRAM_ATTR ButtonTask::handleInterrupt() {
    unsigned long now = millis();
    
    // Hardware debounce in interrupt
    if (now - _lastInterruptTime < DEBOUNCE_MS) {
        return;
    }
    _lastInterruptTime = now;
    
    // Read current state (active LOW)
    bool pressed = (digitalRead(_pin) == LOW);
    _buttonPressed = pressed;
    
    // Wake up task to process event
    if (_taskHandle) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(_taskHandle, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

void ButtonTask::begin(uint8_t pin, ButtonCallback callback) {
    _pin = pin;
    _callback = callback;
    
    // Configure button pin
    pinMode(_pin, INPUT);  // TTGO has external pull-up
    
    // Create event queue
    _eventQueue = xQueueCreate(5, sizeof(ButtonEvent));
    if (!_eventQueue) {
        LOGE("Failed to create event queue");
        return;
    }
    
    // Create task
    BaseType_t result = xTaskCreatePinnedToCore(
        taskLoop,
        "ButtonTask",
        3072,  // Slightly larger stack to accommodate logging
        nullptr,
        2,     // Higher priority than sensor task
        &_taskHandle,
        1      // Core 1
    );
    
    if (result != pdPASS) {
        LOGE("Failed to create task");
        return;
    }
    
    // Attach interrupt (FALLING edge for active-LOW button)
    attachInterrupt(digitalPinToInterrupt(_pin), handleInterrupt, CHANGE);

    LOGI("Initialized on pin %d (GPIO interrupt enabled)", _pin);
}

bool ButtonTask::isPressed() {
    return _buttonPressed;
}

ButtonEvent ButtonTask::getLastEvent() {
    ButtonEvent event = BTN_NONE;
    xQueueReceive(_eventQueue, &event, 0);  // Non-blocking
    return event;
}

void ButtonTask::taskLoop(void* parameter) {
    unsigned long pressStart = 0;
    bool wasPressed = false;
    unsigned long lastHoldLog = 0;
    ButtonEvent lastEvent = BTN_NONE;
    bool longPressFired = false;
    
    LOGI("Task started");
    
    while (true) {
        // Wait for notification from interrupt (or timeout)
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));
        
        bool currentlyPressed = (digitalRead(_pin) == LOW);
        
        // Detect press start
        if (currentlyPressed && !wasPressed) {
            pressStart = millis();
            wasPressed = true;
            LOGI("Press detected");
            longPressFired = false;
            lastHoldLog = 0;
        }
        // While holding, emit periodic heartbeat logs (every ~2s)
        if (currentlyPressed && wasPressed) {
            unsigned long now = millis();
            unsigned long held = now - pressStart;
            if (now - lastHoldLog >= BTN::HOLD_LOG_INTERVAL_MS) {
                LOGI("Press in progress (%lu ms, threshold %lu ms)", held, static_cast<unsigned long>(LONG_PRESS_MS));
                lastHoldLog = now;
            }

            // Trigger long-press as soon as threshold is crossed (once)
            if (!longPressFired && held >= LONG_PRESS_MS) {
                longPressFired = true;
                lastEvent = BTN_LONG_PRESS;
                LOGI("Long press fired while holding (%lu ms >= %lu ms threshold)", held, static_cast<unsigned long>(LONG_PRESS_MS));
                xQueueSend(_eventQueue, &lastEvent, 0);
                if (_callback) {
                    _callback(lastEvent);
                }
            }
        }
        // Detect release
        else if (!currentlyPressed && wasPressed) {
            unsigned long pressDuration = millis() - pressStart;
            wasPressed = false;
            lastHoldLog = 0;
            
            // Determine event type
            if (longPressFired) {
                // Already fired while holding; no duplicate event on release
                lastEvent = BTN_LONG_PRESS;
                LOGI("Long press already fired while holding (%lu ms total)", pressDuration);
            } else if (pressDuration >= LONG_PRESS_MS) {
                lastEvent = BTN_LONG_PRESS;
                LOGI("Long press (%lu ms >= %lu ms threshold)", pressDuration, static_cast<unsigned long>(LONG_PRESS_MS));
                xQueueSend(_eventQueue, &lastEvent, 0);
                if (_callback) {
                    _callback(lastEvent);
                }
            } else {
                lastEvent = BTN_SHORT_PRESS;
                LOGI("Short press (%lu ms)", pressDuration);
                xQueueSend(_eventQueue, &lastEvent, 0);
                if (_callback) {
                    _callback(lastEvent);
                }
            }
        }
    }
}
