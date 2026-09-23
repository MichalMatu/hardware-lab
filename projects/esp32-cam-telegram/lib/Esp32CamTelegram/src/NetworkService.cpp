#include "NetworkService.h"

#include <WiFi.h>
#include <esp_sntp.h>
#include <time.h>

#include "Logger.h"
#include "SafeString.h"

bool NetworkService::begin(AppConfig& config, AppState& state) {
  WiFi.persistent(false);
  WiFi.setSleep(false);

  if (hasText(config.wifiSsid)) {
    if (connectStation(config, state, 15000)) {
      syncTime(state);
      return true;
    }
    LOGW("WIFI", "station connect failed, starting AP+STA");
  }

  startAccessPoint(config, state);
  return false;
}

bool NetworkService::ensureConnected(AppConfig& config, AppState& state) {
  if (WiFi.status() == WL_CONNECTED) {
    state.wifiConnected = true;
    return true;
  }

  state.wifiConnected = false;
  const uint32_t now = millis();
  if (now - state.lastWifiAttemptMillis < 30000 || !hasText(config.wifiSsid)) {
    return false;
  }
  state.lastWifiAttemptMillis = now;
  return connectStation(config, state, 8000);
}

bool NetworkService::connectStation(AppConfig& config, AppState& state, uint32_t timeoutMs) {
  WiFi.mode(state.apMode ? WIFI_AP_STA : WIFI_STA);
  WiFi.setHostname(config.hostname);
  WiFi.begin(config.wifiSsid, config.wifiPass);
  LOGI("WIFI", "connecting station");

  const uint32_t started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < timeoutMs) {
    delay(250);
  }

  state.wifiConnected = WiFi.status() == WL_CONNECTED;
  if (state.wifiConnected) {
    LOGI("WIFI", WiFi.localIP().toString().c_str());
  }
  return state.wifiConnected;
}

void NetworkService::startAccessPoint(AppConfig& config, AppState& state) {
  char apName[48];
  snprintf(apName, sizeof(apName), "%s-setup", config.hostname);
  WiFi.mode(hasText(config.wifiSsid) ? WIFI_AP_STA : WIFI_AP);
  WiFi.softAP(apName);
  state.apMode = true;
  LOGI("WIFI", apName);
  LOGI("WIFI", WiFi.softAPIP().toString().c_str());
}

bool NetworkService::syncTime(AppState& state) {
  if (WiFi.status() != WL_CONNECTED) {
    state.timeSynced = false;
    return false;
  }

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  const uint32_t started = millis();
  while (millis() - started < 8000) {
    time_t now = time(nullptr);
    if (now > 1700000000) {
      state.timeSynced = true;
      LOGI("TIME", "synced");
      return true;
    }
    delay(200);
  }

  state.timeSynced = false;
  LOGW("TIME", "sync timeout");
  return false;
}
