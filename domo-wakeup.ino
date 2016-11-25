
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
#include "wifi-led.h"
#include "time.h"
#include "button.h"
#include "wakeup.h"
#include "htsensor.h"
HTSensor htsensor;
#include "server.h"

Button button(BUTTON_PIN);

void setup() {
  analogWriteFreq(300);
  analogWriteRange(1023);

  Serial.begin(115200);
  Serial.println(F("ESP8266 begin"));

  Settings.begin();
  wakeup.begin(WAKEUP_PIN);
  WifiLed.begin(D2); // D4 is the onboard LED of the ESP-12E
  htsensor.setup();
  wifi.setup();
  serverSetup();
  radioSetup();
  Serial.println(F("ESP8266 setup complete"));
}

void loop() {
  wifi.loop();
  WifiLed.loop();
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
