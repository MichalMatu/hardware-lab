#ifndef ButtonTask_h
#define ButtonTask_h

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// Button event types
enum ButtonEvent {
    BTN_NONE = 0,
    BTN_SHORT_PRESS = 1,
    BTN_LONG_PRESS = 2
};

// Callback type for button events
typedef void (*ButtonCallback)(ButtonEvent event);

class ButtonTask {
public:
    // Initialize button task with GPIO interrupt
    static void begin(uint8_t pin, ButtonCallback callback = nullptr);
    
    // Check if button is currently pressed (for external use)
    static bool isPressed();
    
    // Get last event (non-blocking)
    static ButtonEvent getLastEvent();

private:
    static TaskHandle_t _taskHandle;
    static QueueHandle_t _eventQueue;
    static uint8_t _pin;
    static ButtonCallback _callback;
    static volatile unsigned long _lastInterruptTime;
    static volatile bool _buttonPressed;
    
    // GPIO interrupt handler (IRAM_ATTR)
    static void IRAM_ATTR handleInterrupt();
    
    // Task loop - processes debounced events
    static void taskLoop(void* parameter);
};

#endif
