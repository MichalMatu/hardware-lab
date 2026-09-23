#pragma once

#include <Preferences.h>
#include "AppTypes.h"

class ConfigManager {
 public:
  bool begin();
  bool load(AppConfig& config);
  bool save(const AppConfig& config);
  bool clear();

 private:
  Preferences prefs_;
};
