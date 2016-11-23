
const uint8_t LIGHT_PIN = D0;
const uint8_t BUTTON_PIN = D1;

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
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
  Serial.println(F("ESP8266 begin"));

  Settings.begin();
  light.begin(LIGHT_PIN);
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
  light.loop();

  bool buttonPressed = button.pressed();
  if (buttonPressed && !buttonWasPressed) {
    switch (light.currentState()) {
      case LIGHT_OFF:
        log(F("button off -> on"));
        light.on();
        break;
      case LIGHT_SLOWSTART:
        log(F("button wake -> on"));
        light.on();
        break;
      case LIGHT_ON:
        log(F("button on -> off"));
        light.off();
        break;
    }
  }
  buttonWasPressed = buttonPressed;
}
