#include "WebPanel.h"

#include <string.h>

#include <SD_MMC.h>
#include <WiFi.h>

#include "Logger.h"
#include "SafeString.h"

void WebPanel::begin(AppConfig& config, AppState& state, ConfigManager& configManager, CaptureCallback capture, RebootCallback reboot, ArmStateCallback armState, void* context) {
  config_ = &config;
  state_ = &state;
  configManager_ = &configManager;
  capture_ = capture;
  reboot_ = reboot;
  armState_ = armState;
  context_ = context;

  server_.on("/", HTTP_GET, [this]() { root(); });
  server_.on("/setup", HTTP_GET, [this]() { root(); });
  server_.on("/status.json", HTTP_GET, [this]() { statusJson(); });
  server_.on("/save", HTTP_POST, [this]() { save(); });
  server_.on("/capture", HTTP_POST, [this]() { this->capture(); });
  server_.on("/arm", HTTP_POST, [this]() { arm(); });
  server_.on("/disarm", HTTP_POST, [this]() { disarm(); });
  server_.on("/privacy", HTTP_POST, [this]() { privacy(); });
  server_.on("/reset-config", HTTP_POST, [this]() { resetConfig(); });
  server_.on("/reboot", HTTP_POST, [this]() { this->reboot(); });
  server_.on("/last.jpg", HTTP_GET, [this]() { lastPhoto(); });
  server_.onNotFound([this]() { notFound(); });
  server_.begin();
  LOGI("WEB", "started");
}

void WebPanel::handle() {
  server_.handleClient();
}

bool WebPanel::authorize() {
  if (state_ != nullptr && state_->apMode) {
    return true;
  }
  if (config_ == nullptr || !hasText(config_->panelPass)) {
    return true;
  }
  if (server_.authenticate(config_->panelUser, config_->panelPass)) {
    return true;
  }
  server_.requestAuthentication();
  return false;
}

void WebPanel::root() {
  if (!authorize()) {
    return;
  }

  server_.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server_.send(200, "text/html", "");
  server_.sendContent("<!doctype html><html><head><meta charset='utf-8'>"
                      "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                      "<title>ESP32-CAM</title><style>"
                      "body{font-family:system-ui,-apple-system,Segoe UI,sans-serif;margin:0;background:#f4f5f7;color:#14161a}"
                      "main{max-width:920px;margin:0 auto;padding:20px}"
                      "section{background:#fff;border:1px solid #d9dde5;border-radius:8px;padding:16px;margin:12px 0}"
                      "label{display:block;font-size:13px;margin:10px 0 4px;color:#3b4350}"
                      "input,select{width:100%;box-sizing:border-box;padding:9px;border:1px solid #b8bfcc;border-radius:6px}"
                      "input[type=checkbox]{width:auto;margin-right:8px}button{padding:9px 12px;border:1px solid #1e5aa8;background:#2367bf;color:#fff;border-radius:6px;margin:12px 4px 4px 0}"
                      ".danger{background:#a92323;border-color:#891818}.formgrid{display:grid;grid-template-columns:repeat(auto-fit,minmax(260px,1fr));gap:8px 14px}"
                      ".wide{grid-column:1/-1}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(160px,1fr));gap:8px}"
                      ".metric{padding:10px;border:1px solid #e1e4ea;border-radius:6px;background:#fafbfc}.metric b{display:block;font-size:12px;color:#596273}"
                      "</style></head><body><main><h1>ESP32-CAM</h1>");

  server_.sendContent("<section><h2>Network / Telegram</h2><form method='post' action='/save'>"
                      "<input type='hidden' name='setup' value='1'>"
                      "<div class='formgrid'><div><label>Hostname</label><input name='hostname' maxlength='31' value='");
  sendEscaped(config_->hostname);
  server_.sendContent("'></div><div><label>Wi-Fi SSID</label><input name='ssid' maxlength='32' autocomplete='off' value='");
  sendEscaped(config_->wifiSsid);
  server_.sendContent("'></div><div><label>Wi-Fi password</label><input name='wpass' maxlength='64' type='password' autocomplete='off' value='");
  sendEscaped(config_->wifiPass);
  server_.sendContent("'></div><div><label>Telegram chat ID</label><input name='tgchat' maxlength='31' inputmode='numeric' value='");
  sendEscaped(config_->telegramChatId);
  server_.sendContent("'></div><div class='wide'><label>Telegram token</label><input name='tgtok' maxlength='111' type='password' autocomplete='off' value='");
  sendEscaped(config_->telegramToken);
  server_.sendContent("'></div><div class='wide'><label><input name='tgalerts' type='checkbox' value='1' ");
  if (config_->sendAlertsToTelegram) {
    server_.sendContent("checked");
  }
  server_.sendContent("> Send motion alerts to Telegram</label><label><input name='tlsinsec' type='checkbox' value='1' ");
  if (config_->allowInsecureTls) {
    server_.sendContent("checked");
  }
  server_.sendContent("> Allow insecure TLS fallback</label></div></div><button>Save network config</button></form></section>");

  server_.sendContent("<section><h2>Status</h2><div class='grid'>");
  char item[192];
  snprintf(item, sizeof(item), "<div class='metric'><b>Wi-Fi</b>%s</div>", state_->wifiConnected ? WiFi.localIP().toString().c_str() : "offline");
  server_.sendContent(item);
  snprintf(item, sizeof(item), "<div class='metric'><b>AP</b>%s</div>", state_->apMode ? WiFi.softAPIP().toString().c_str() : "off");
  server_.sendContent(item);
  snprintf(item, sizeof(item), "<div class='metric'><b>Armed</b>%s</div>", armStateName(state_->armState));
  server_.sendContent(item);
  snprintf(item, sizeof(item), "<div class='metric'><b>Camera</b>%s</div>", state_->cameraReady ? "ready" : "error");
  server_.sendContent(item);
  snprintf(item, sizeof(item), "<div class='metric'><b>SD</b>%s</div>", state_->sdMounted ? "mounted" : "off");
  server_.sendContent(item);
  snprintf(item, sizeof(item), "<div class='metric'><b>Flash</b>%s / LED %s / light %u</div>",
           flashModeName(config_->flashMode), state_->flashOn ? "on" : "off", state_->motionBrightness);
  server_.sendContent(item);
  snprintf(item, sizeof(item), "<div class='metric'><b>Heap</b>%u / min %u</div>", ESP.getFreeHeap(), ESP.getMinFreeHeap());
  server_.sendContent(item);
  snprintf(item, sizeof(item), "<div class='metric'><b>Motion</b>%u/%u score %u th %u</div>",
           state_->motionChangedBlocks, state_->motionMinBlocks, state_->motionDiffSum, state_->motionThreshold);
  server_.sendContent(item);
  snprintf(item, sizeof(item), "<div class='metric'><b>Motion events</b>%lu</div>",
           static_cast<unsigned long>(state_->motionEvents));
  server_.sendContent(item);
  snprintf(item, sizeof(item), "<div class='metric'><b>Last motion sample</b>%lu ms</div>",
           static_cast<unsigned long>(state_->lastMotionSampleMillis));
  server_.sendContent(item);
  server_.sendContent("</div><form method='post' action='/arm'><button>Arm</button></form>"
                      "<form method='post' action='/disarm'><button>Disarm</button></form>"
                      "<form method='post' action='/privacy'><button>Privacy</button></form>"
                      "<form method='post' action='/capture'><button>Capture</button></form>");
  if (hasText(state_->lastPhotoPath)) {
    server_.sendContent("<p><a href='/last.jpg'>Last photo</a></p>");
  }
  server_.sendContent("</section>");

  server_.sendContent("<section><h2>Device</h2><form method='post' action='/save'>"
                      "<input type='hidden' name='device' value='1'>"
                      "<div class='formgrid'><div><label>Panel user</label><input name='puser' maxlength='19' value='");
  sendEscaped(config_->panelUser);
  server_.sendContent("'></div><div><label>Panel password</label><input name='ppass' maxlength='39' type='password' value='");
  sendEscaped(config_->panelPass);
  server_.sendContent("'></div><div><label>Start state</label><select name='arm'>");
  server_.sendContent("<option value='0'");
  if (config_->startState == ArmState::Disarmed) {
    server_.sendContent(" selected");
  }
  server_.sendContent(">disarmed</option><option value='1'");
  if (config_->startState == ArmState::Armed) {
    server_.sendContent(" selected");
  }
  server_.sendContent(">armed</option><option value='2'");
  if (config_->startState == ArmState::Privacy) {
    server_.sendContent(" selected");
  }
  server_.sendContent(">privacy</option></select></div><div><label>Flash</label><select name='flash'>");
  server_.sendContent("<option value='0'");
  if (config_->flashMode == FlashMode::Off) {
    server_.sendContent(" selected");
  }
  server_.sendContent(">off</option><option value='1'");
  if (config_->flashMode == FlashMode::On) {
    server_.sendContent(" selected");
  }
  server_.sendContent(">on</option><option value='2'");
  if (config_->flashMode == FlashMode::Auto) {
    server_.sendContent(" selected");
  }
  server_.sendContent(">auto dark</option><option value='3'");
  if (config_->flashMode == FlashMode::Motion) {
    server_.sendContent(" selected");
  }
  server_.sendContent(">motion</option></select></div><div><label>Dark flash threshold 5-220</label><input name='darkth' type='number' min='5' max='220' value='");
  snprintf(item, sizeof(item), "%u", config_->darkFlashThreshold);
  server_.sendContent(item);
  server_.sendContent("'></div><div><label>Motion sensitivity 1-10</label><input name='msens' type='number' min='1' max='10' value='");
  snprintf(item, sizeof(item), "%u", config_->motionSensitivity);
  server_.sendContent(item);
  server_.sendContent("'></div><div><label>Motion cooldown seconds</label><input name='mcool' type='number' min='2' max='3600' value='");
  snprintf(item, sizeof(item), "%u", config_->motionCooldownSec);
  server_.sendContent(item);
  server_.sendContent("'></div><div><label>JPEG quality 8-20</label><input name='jpeg' type='number' min='8' max='20' value='");
  snprintf(item, sizeof(item), "%u", config_->jpegQuality);
  server_.sendContent(item);
  server_.sendContent("'></div><div class='wide'><label><input name='savesd' type='checkbox' value='1' ");
  if (config_->savePhotosToSd) {
    server_.sendContent("checked");
  }
  server_.sendContent("> Save photos to SD</label></div></div><button>Save device config</button></form></section>");

  server_.sendContent("<section><form method='post' action='/reboot'><button>Reboot</button></form>"
                      "<form method='post' action='/reset-config'><button class='danger'>Reset config</button></form></section>");
  server_.sendContent("</main></body></html>");
  server_.sendContent("");
}

void WebPanel::statusJson() {
  if (!authorize()) {
    return;
  }

  const String ip = state_->wifiConnected ? WiFi.localIP().toString() : String("");
  const String apIp = state_->apMode ? WiFi.softAPIP().toString() : String("");
  char body[704];
  snprintf(body, sizeof(body),
           "{\"state\":\"%s\",\"wifi\":%s,\"ip\":\"%s\",\"ap\":%s,\"apIp\":\"%s\","
           "\"telegramReady\":%s,\"cameraReady\":%s,\"sdMounted\":%s,"
           "\"flashMode\":\"%s\",\"flashOn\":%s,\"brightness\":%u,\"darkFlashThreshold\":%u,"
           "\"heap\":%u,\"minHeap\":%u,\"motionEvents\":%lu,\"lastMotion\":%lu,\"lastMotionSample\":%lu,"
           "\"motionChangedBlocks\":%u,\"motionMinBlocks\":%u,\"motionScore\":%u,\"motionThreshold\":%u,"
           "\"lastPhoto\":%lu,\"lastTelegram\":%lu}",
           armStateName(state_->armState),
           state_->wifiConnected ? "true" : "false",
           ip.c_str(),
           state_->apMode ? "true" : "false",
           apIp.c_str(),
           state_->telegramReady ? "true" : "false",
           state_->cameraReady ? "true" : "false",
           state_->sdMounted ? "true" : "false",
           flashModeName(config_->flashMode),
           state_->flashOn ? "true" : "false",
           state_->motionBrightness,
           config_->darkFlashThreshold,
           ESP.getFreeHeap(),
           ESP.getMinFreeHeap(),
           static_cast<unsigned long>(state_->motionEvents),
           static_cast<unsigned long>(state_->lastMotionMillis),
           static_cast<unsigned long>(state_->lastMotionSampleMillis),
           state_->motionChangedBlocks,
           state_->motionMinBlocks,
           state_->motionDiffSum,
           state_->motionThreshold,
           static_cast<unsigned long>(state_->lastPhotoMillis),
           static_cast<unsigned long>(state_->lastTelegramMillis));
  server_.send(200, "application/json", body);
}

void WebPanel::save() {
  if (!authorize()) {
    return;
  }

  if (server_.hasArg("setup")) {
    copyArg("hostname", config_->hostname, sizeof(config_->hostname));
    copyArg("ssid", config_->wifiSsid, sizeof(config_->wifiSsid));
    copyArg("wpass", config_->wifiPass, sizeof(config_->wifiPass));
    copyArg("tgtok", config_->telegramToken, sizeof(config_->telegramToken));
    copyArg("tgchat", config_->telegramChatId, sizeof(config_->telegramChatId));
    config_->sendAlertsToTelegram = server_.hasArg("tgalerts");
    config_->allowInsecureTls = server_.hasArg("tlsinsec");
  }

  if (server_.hasArg("device")) {
    copyArg("puser", config_->panelUser, sizeof(config_->panelUser));
    copyArg("ppass", config_->panelPass, sizeof(config_->panelPass));
    config_->startState = static_cast<ArmState>(server_.arg("arm").toInt());
    config_->flashMode = static_cast<FlashMode>(server_.arg("flash").toInt());
    config_->motionSensitivity = static_cast<uint8_t>(constrain(server_.arg("msens").toInt(), 1, 10));
    config_->motionCooldownSec = static_cast<uint16_t>(constrain(server_.arg("mcool").toInt(), 2, 3600));
    config_->darkFlashThreshold = static_cast<uint8_t>(constrain(server_.arg("darkth").toInt(), 5, 220));
    config_->jpegQuality = static_cast<uint8_t>(constrain(server_.arg("jpeg").toInt(), 8, 20));
    config_->savePhotosToSd = server_.hasArg("savesd");
  }

  configManager_->save(*config_);
  server_.send(200, "text/html", "<meta http-equiv='refresh' content='2;url=/'><p>Saved. Reboot recommended after network changes.</p>");
}

void WebPanel::capture() {
  if (!authorize()) {
    return;
  }
  const bool ok = capture_ != nullptr && capture_(context_, PhotoReason::ManualWeb, false);
  server_.send(ok ? 200 : 500, "text/plain", ok ? "captured" : "capture failed");
}

void WebPanel::arm() {
  if (!authorize()) {
    return;
  }
  if (armState_ != nullptr) {
    armState_(context_, ArmState::Armed);
  }
  server_.send(200, "text/plain", "armed");
}

void WebPanel::disarm() {
  if (!authorize()) {
    return;
  }
  if (armState_ != nullptr) {
    armState_(context_, ArmState::Disarmed);
  }
  server_.send(200, "text/plain", "disarmed");
}

void WebPanel::privacy() {
  if (!authorize()) {
    return;
  }
  if (armState_ != nullptr) {
    armState_(context_, ArmState::Privacy);
  }
  server_.send(200, "text/plain", "privacy");
}

void WebPanel::resetConfig() {
  if (!authorize()) {
    return;
  }
  configManager_->clear();
  server_.send(200, "text/plain", "config cleared, rebooting");
  delay(300);
  if (reboot_ != nullptr) {
    reboot_(context_);
  }
}

void WebPanel::reboot() {
  if (!authorize()) {
    return;
  }
  server_.send(200, "text/plain", "rebooting");
  delay(300);
  if (reboot_ != nullptr) {
    reboot_(context_);
  }
}

void WebPanel::lastPhoto() {
  if (!authorize()) {
    return;
  }
  if (!hasText(state_->lastPhotoPath) || !state_->sdMounted) {
    server_.send(404, "text/plain", "no photo");
    return;
  }
  File file = SD_MMC.open(state_->lastPhotoPath, FILE_READ);
  if (!file) {
    server_.send(404, "text/plain", "not found");
    return;
  }
  server_.streamFile(file, "image/jpeg");
  file.close();
}

void WebPanel::copyArg(const char* name, char* dest, size_t destLen) {
  if (server_.hasArg(name)) {
    copyText(dest, destLen, server_.arg(name));
  }
}

void WebPanel::sendChunk(const char* text) {
  if (text != nullptr && text[0] != '\0') {
    server_.sendContent(text);
  }
}

void WebPanel::sendEscaped(const char* text) {
  if (text == nullptr || text[0] == '\0') {
    return;
  }

  char out[96];
  size_t pos = 0;
  for (const char* p = text; *p != '\0'; ++p) {
    const char* replacement = nullptr;
    switch (*p) {
      case '&':
        replacement = "&amp;";
        break;
      case '<':
        replacement = "&lt;";
        break;
      case '>':
        replacement = "&gt;";
        break;
      case '"':
        replacement = "&quot;";
        break;
      case '\'':
        replacement = "&#39;";
        break;
      default:
        break;
    }

    if (replacement != nullptr) {
      const size_t len = strlen(replacement);
      if (pos + len >= sizeof(out)) {
        out[pos] = '\0';
        sendChunk(out);
        pos = 0;
      }
      memcpy(out + pos, replacement, len);
      pos += len;
    } else {
      if (pos + 1 >= sizeof(out)) {
        out[pos] = '\0';
        sendChunk(out);
        pos = 0;
      }
      out[pos++] = *p;
    }
  }
  out[pos] = '\0';
  sendChunk(out);
}

void WebPanel::notFound() {
  server_.send(404, "text/plain", "not found");
}
