
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>

#include "wifi.h"
#include "config.h"
#include "mqtt.h"

WiFiClass wifi;

void WiFiClass::setup() {
  WiFi.begin(SSID, PASSWORD);
}

void WiFiClass::loop() {
  bool connected = WiFi.status() == WL_CONNECTED;
  if (!mdnsStarted && connected) {
    ArduinoOTA.setHostname(CLIENT_ID);
#ifdef OTA_PASSWORD
    ArduinoOTA.setPassword(OTA_PASSWORD);
#endif

    // This also starts the mDNS server
    ArduinoOTA.begin();

    // Not sure whether we need this...
    MDNS.addService("http", "tcp", 80);
  }

  if (connected) {
    mqttLoop();
  }
}
