
const uint8_t LIGHT_PIN = D0;
const uint8_t BUTTON_PIN = D1;
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include "settings.h"
#include "wifi-led.h"
#include "time.h"
#include "button.h"
#include "light.h"
#include "htsensor.h"
WakeupLight light;
HTSensor htsensor;
#include "server.h"

Button button(BUTTON_PIN);

void setup() {
  analogWriteFreq(300);
  analogWriteRange(1023);

  Serial.begin(115200);
  Serial.println(F("begin"));

  Settings.begin();
  light.begin(LIGHT_PIN);
  WifiLed.begin(D5); // D4 is the onboard LED of the ESP-12E
  wifiSetup();
  htsensor.setup();
  serverSetup();
}

void loop() {
  wifiLoop();
  WifiLed.loop();
  Clock.loop();
  htsensor.loop();
  serverLoop();
  ArduinoOTA.handle();

  static bool buttonWasPressed = false;

  button.loop();
  light.loop();

  bool buttonPressed = button.pressed();
  if (buttonPressed && !buttonWasPressed) {
    switch (light.currentState()) {
      case LIGHT_OFF:
        Serial.println(F("button off -> on"));
        light.on();
        break;
      case LIGHT_SLOWSTART:
        Serial.println(F("button wake -> on"));
        light.on();
        break;
      case LIGHT_ON:
        Serial.println(F("button on -> off"));
        light.off();
        break;
    }
  }
  buttonWasPressed = buttonPressed;
}
