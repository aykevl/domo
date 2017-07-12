
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>

#include "wifi.h"
#include "config.h"
#include "mqtt.h"

WiFiClass wifi;

void WiFiClass::setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
}

void WiFiClass::loop() {
  bool connected = WiFi.status() == WL_CONNECTED;

#ifdef SERIAL_ENABLED
  static bool hasPrintedIP = false;
  if (connected && !hasPrintedIP) {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    hasPrintedIP = true;
  }
#endif

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
