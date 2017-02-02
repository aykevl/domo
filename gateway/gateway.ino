
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
#include "htsensor.h"
HTSensor htsensor;
#include "server.h"

//#define SERIAL_ENABLED

void setup() {
#ifdef SERIAL_ENABLED
  Serial.begin(115200);
  Serial.println(F("ESP8266 begin"));
#endif

  Settings.begin();
  htsensor.setup();
  radioSetup();
  wifi.setup();
  serverSetup();
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
}
