#include <core/ESP32SvelteKit.h>
#include <PsychicHttpServer.h>
#include "config/AppConfig.h"
#include "system/Application.h"
#include "system/Logging.h"

using namespace COM;
using namespace HW;
using namespace APP;

PsychicHttpServer server;
ESP32SvelteKit esp32sveltekit(&server, 120);

#undef LOG_TAG
#define LOG_TAG "Main"

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    LOGI("\n\n* * * %s %s v%s * * *\n", DEVICE, NAME, VERSION);
    Application::instance().setup(server, esp32sveltekit);
}

void loop() {
    Application::instance().loop();
}