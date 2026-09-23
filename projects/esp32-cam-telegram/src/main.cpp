#include <Arduino.h>
#include <Esp32CamTelegram.h>

AppController App;

void setup() {
  App.begin();
}

void loop() {
  App.loop();
}
