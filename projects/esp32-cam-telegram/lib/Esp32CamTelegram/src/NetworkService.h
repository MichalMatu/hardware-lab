#pragma once

#include <Arduino.h>
#include "AppTypes.h"

class NetworkService {
 public:
  bool begin(AppConfig& config, AppState& state);
  bool ensureConnected(AppConfig& config, AppState& state);
  bool syncTime(AppState& state);

 private:
  bool connectStation(AppConfig& config, AppState& state, uint32_t timeoutMs);
  void startAccessPoint(AppConfig& config, AppState& state);
};
