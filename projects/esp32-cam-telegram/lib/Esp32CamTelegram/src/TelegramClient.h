#pragma once

#include <Arduino.h>
#include <WiFiClientSecure.h>

struct TelegramCommand {
  int64_t chatId = 0;
  char text[48] = "";
};

class TelegramClient {
 public:
  void begin(const char* token, const char* chatId, bool allowInsecureTls);
  bool ready() const;
  bool poll(TelegramCommand* commands, size_t maxCommands, size_t& commandCount);
  bool sendMessage(const char* text);
  bool sendPhoto(const uint8_t* jpeg, size_t length, const char* caption);
  bool isAllowedChat(int64_t chatId) const;

 private:
  bool request(const char* method, const char* pathAndQuery, const char* body, const char* contentType, char* response, size_t responseLen);
  bool connect(WiFiClientSecure& client);
  bool readHttpBody(WiFiClientSecure& client, char* response, size_t responseLen);
  bool parseUpdates(char* body, TelegramCommand* commands, size_t maxCommands, size_t& commandCount);
  void configureTls(WiFiClientSecure& client);
  bool urlEncode(const char* in, char* out, size_t outLen);

  char token_[112] = "";
  char chatId_[32] = "";
  bool allowInsecureTls_ = false;
  int64_t offset_ = 0;
  uint32_t lastPollMillis_ = 0;
};
