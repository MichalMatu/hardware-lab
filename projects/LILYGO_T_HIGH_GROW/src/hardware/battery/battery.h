#ifndef HARDWARE_BATTERY_H
#define HARDWARE_BATTERY_H

#include <Arduino.h>

uint32_t readBatteryADC();
float calcBattery(uint16_t AdcVolt);
float calcBatteryDays();

#endif
