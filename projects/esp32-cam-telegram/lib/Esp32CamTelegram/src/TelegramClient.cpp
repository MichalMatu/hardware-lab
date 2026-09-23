#include "TelegramClient.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "Logger.h"
#include "SafeString.h"
#include "TelegramCa.h"

namespace {
constexpr const char* TelegramHost = "api.telegram.org";
constexpr uint16_t TelegramPort = 443;

const char* findAfter(const char* start, const char* needle) {
  const char* found = strstr(start, needle);
  return found == nullptr ? nullptr : found + strlen(needle);
}

int64_t parseInt64(const char* text) {
  if (text == nullptr) {
    return 0;
  }
  while (*text != '\0' && *text != '-' && !isdigit(static_cast<unsigned char>(*text))) {
    ++text;
  }
  return atoll(text);
}

bool copyJsonString(const char* start, char* out, size_t outLen) {
  if (outLen == 0 || start == nullptr) {
    return false;
  }
  out[0] = '\0';
  const char* quote = strchr(start, '"');
  if (quote == nullptr) {
    return false;
  }
  ++quote;
  size_t n = 0;
  while (*quote != '\0' && *quote != '"' && n + 1 < outLen) {
    if (*quote == '\\' && quote[1] != '\0') {
      ++quote;
    }
    out[n++] = *quote++;
  }
  out[n] = '\0';
  return n > 0;
}
}  // namespace

void TelegramClient::begin(const char* token, const char* chatId, bool allowInsecureTls) {
  copyText(token_, sizeof(token_), token);
  copyText(chatId_, sizeof(chatId_), chatId);
  allowInsecureTls_ = allowInsecureTls;
}

bool TelegramClient::ready() const {
  return hasText(token_) && hasText(chatId_);
}

bool TelegramClient::poll(TelegramCommand* commands, size_t maxCommands, size_t& commandCount) {
  commandCount = 0;
  if (!ready() || commands == nullptr || maxCommands == 0) {
    return false;
  }

  const uint32_t now = millis();
  if (now - lastPollMillis_ < 1200) {
    return true;
  }
  lastPollMillis_ = now;

  char path[192];
  snprintf(path, sizeof(path), "/bot%s/getUpdates?timeout=3&limit=4&offset=%lld",
           token_, static_cast<long long>(offset_));

  char response[4096];
  if (!request("GET", path, nullptr, nullptr, response, sizeof(response))) {
    return false;
  }
  return parseUpdates(response, commands, maxCommands, commandCount);
}

bool TelegramClient::sendMessage(const char* text) {
  if (!ready() || !hasText(text)) {
    return false;
  }

  char encoded[512];
  if (!urlEncode(text, encoded, sizeof(encoded))) {
    return false;
  }

  char body[640];
  snprintf(body, sizeof(body), "chat_id=%s&disable_web_page_preview=true&text=%s", chatId_, encoded);
  char path[144];
  snprintf(path, sizeof(path), "/bot%s/sendMessage", token_);
  char response[512];
  return request("POST", path, body, "application/x-www-form-urlencoded", response, sizeof(response));
}

bool TelegramClient::sendPhoto(const uint8_t* jpeg, size_t length, const char* caption) {
  if (!ready() || jpeg == nullptr || length == 0) {
    return false;
  }

  WiFiClientSecure client;
  configureTls(client);
  client.setTimeout(20000);
  if (!connect(client)) {
    return false;
  }

  constexpr const char* boundary = "----esp32cam-telegram-boundary";
  char head[512];
  const int headLen = snprintf(
      head, sizeof(head),
      "--%s\r\n"
      "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n"
      "%s\r\n"
      "--%s\r\n"
      "Content-Disposition: form-data; name=\"caption\"\r\n\r\n"
      "%s\r\n"
      "--%s\r\n"
      "Content-Disposition: form-data; name=\"photo\"; filename=\"esp32cam.jpg\"\r\n"
      "Content-Type: image/jpeg\r\n\r\n",
      boundary, chatId_, boundary, caption == nullptr ? "" : caption, boundary);
  char tail[64];
  const int tailLen = snprintf(tail, sizeof(tail), "\r\n--%s--\r\n", boundary);
  if (headLen <= 0 || tailLen <= 0 || headLen >= static_cast<int>(sizeof(head)) || tailLen >= static_cast<int>(sizeof(tail))) {
    client.stop();
    return false;
  }

  char path[144];
  snprintf(path, sizeof(path), "/bot%s/sendPhoto", token_);
  const size_t contentLength = static_cast<size_t>(headLen) + length + static_cast<size_t>(tailLen);

  client.printf("POST %s HTTP/1.1\r\n", path);
  client.printf("Host: %s\r\n", TelegramHost);
  client.print("Connection: close\r\n");
  client.printf("Content-Type: multipart/form-data; boundary=%s\r\n", boundary);
  client.printf("Content-Length: %u\r\n\r\n", static_cast<unsigned>(contentLength));
  client.write(reinterpret_cast<const uint8_t*>(head), static_cast<size_t>(headLen));
  client.write(jpeg, length);
  client.write(reinterpret_cast<const uint8_t*>(tail), static_cast<size_t>(tailLen));

  char response[512];
  const bool ok = readHttpBody(client, response, sizeof(response)) && strstr(response, "\"ok\":true") != nullptr;
  client.stop();
  return ok;
}

bool TelegramClient::isAllowedChat(int64_t chatId) const {
  return atoll(chatId_) == chatId;
}

bool TelegramClient::request(const char* method, const char* pathAndQuery, const char* body, const char* contentType, char* response, size_t responseLen) {
  WiFiClientSecure client;
  configureTls(client);
  client.setTimeout(18000);
  if (!connect(client)) {
    return false;
  }

  const bool hasBody = body != nullptr && body[0] != '\0';
  client.printf("%s %s HTTP/1.1\r\n", method, pathAndQuery);
  client.printf("Host: %s\r\n", TelegramHost);
  client.print("Connection: close\r\n");
  if (hasBody) {
    client.printf("Content-Type: %s\r\n", contentType);
    client.printf("Content-Length: %u\r\n", static_cast<unsigned>(strlen(body)));
  }
  client.print("\r\n");
  if (hasBody) {
    client.print(body);
  }

  const bool ok = readHttpBody(client, response, responseLen);
  client.stop();
  return ok && strstr(response, "\"ok\":true") != nullptr;
}

bool TelegramClient::connect(WiFiClientSecure& client) {
  if (client.connect(TelegramHost, TelegramPort, 12000)) {
    return true;
  }
  LOGW("TG", "connect failed");
  return false;
}

bool TelegramClient::readHttpBody(WiFiClientSecure& client, char* response, size_t responseLen) {
  if (responseLen == 0) {
    return false;
  }
  response[0] = '\0';

  const uint32_t started = millis();
  bool headersDone = false;
  size_t pos = 0;
  char last4[5] = "";

  while (client.connected() || client.available()) {
    while (client.available()) {
      const char c = static_cast<char>(client.read());
      if (!headersDone) {
        memmove(last4, last4 + 1, 3);
        last4[3] = c;
        last4[4] = '\0';
        if (strcmp(last4, "\r\n\r\n") == 0) {
          headersDone = true;
        }
      } else if (pos + 1 < responseLen) {
        response[pos++] = c;
      }
    }
    if (millis() - started > 22000) {
      break;
    }
    delay(1);
  }
  response[pos] = '\0';
  return pos > 0;
}

bool TelegramClient::parseUpdates(char* body, TelegramCommand* commands, size_t maxCommands, size_t& commandCount) {
  commandCount = 0;
  char* cursor = body;
  while (commandCount < maxCommands) {
    char* update = strstr(cursor, "\"update_id\":");
    if (update == nullptr) {
      break;
    }
    const int64_t updateId = parseInt64(update + 12);
    if (updateId >= offset_) {
      offset_ = updateId + 1;
    }

    char* nextUpdate = strstr(update + 12, "\"update_id\":");
    char* textKey = strstr(update, "\"text\":");
    char* chatKey = strstr(update, "\"chat\":{\"id\":");
    if (textKey != nullptr && chatKey != nullptr && (nextUpdate == nullptr || (textKey < nextUpdate && chatKey < nextUpdate))) {
      const int64_t chatId = parseInt64(chatKey + 13);
      if (isAllowedChat(chatId)) {
        TelegramCommand& command = commands[commandCount];
        command.chatId = chatId;
        if (copyJsonString(textKey + 7, command.text, sizeof(command.text))) {
          ++commandCount;
        }
      }
    }
    if (nextUpdate == nullptr) {
      break;
    }
    cursor = nextUpdate;
  }
  return true;
}

void TelegramClient::configureTls(WiFiClientSecure& client) {
  if (allowInsecureTls_) {
    client.setInsecure();
  } else {
    client.setCACert(TELEGRAM_ROOT_CA);
  }
}

bool TelegramClient::urlEncode(const char* in, char* out, size_t outLen) {
  if (outLen == 0) {
    return false;
  }
  size_t pos = 0;
  for (const char* p = in; *p != '\0'; ++p) {
    const uint8_t c = static_cast<uint8_t>(*p);
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      if (pos + 1 >= outLen) {
        return false;
      }
      out[pos++] = static_cast<char>(c);
    } else if (c == ' ') {
      if (pos + 1 >= outLen) {
        return false;
      }
      out[pos++] = '+';
    } else {
      if (pos + 3 >= outLen) {
        return false;
      }
      static constexpr char hex[] = "0123456789ABCDEF";
      out[pos++] = '%';
      out[pos++] = hex[(c >> 4) & 0x0F];
      out[pos++] = hex[c & 0x0F];
    }
  }
  out[pos] = '\0';
  return true;
}
