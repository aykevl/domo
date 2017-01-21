
const uint8_t WAKEUP_PIN = D0;
const uint8_t BUTTON_PIN = D1;

#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include "config.h"
#include "settings.h"
#include "mqtt.h"
#include "wifi.h"
#include "time.h"
#include "button.h"
#include "wakeup.h"
#include "htsensor.h"
HTSensor htsensor;
#include "server.h"

//#define SERIAL_ENABLED

Button button(BUTTON_PIN);

void setup() {
  analogWriteFreq(300);
  analogWriteRange(1023);

#ifdef SERIAL_ENABLED
  Serial.begin(115200);
  Serial.println(F("ESP8266 begin"));
#endif

  Settings.begin();
  wakeup.begin(WAKEUP_PIN);
  htsensor.setup();
  wifi.setup();
  serverSetup();
  radioSetup();
#ifdef SERIAL_ENABLED
  Serial.println(F("ESP8266 setup complete"));
#endif
}

void loop() {
  wifi.loop();
  Clock.loop();
  htsensor.loop();
  serverLoop();
  radioLoop();
  ArduinoOTA.handle();

  static bool buttonWasPressed = false;

  button.loop();
  wakeup.loop();

  bool buttonPressed = button.pressed();
  if (buttonPressed && !buttonWasPressed) {
    switch (wakeup.currentState()) {
      case LIGHT_OFF:
        log(F("button off -> on"));
        wakeup.on();
        break;
      case LIGHT_WAKE:
        log(F("button wake -> on"));
        wakeup.on();
        break;
      case LIGHT_ON:
        log(F("button on -> off"));
        wakeup.off();
        break;
    }
  }
  buttonWasPressed = buttonPressed;
}
