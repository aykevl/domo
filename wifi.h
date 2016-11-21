
#pragma once

#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include "secrets.h"

WiFiClient mqttConn;
Adafruit_MQTT_Client mqtt(&mqttConn, MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASS);

class WiFiClass {
  bool mdnsStarted = false;
  bool mqttWasConnected = false;
  uint32_t mqttLastTry = 0;

  public:

    void setup() {
      WiFi.begin(SSID, PASSWORD);
    }

    void loop() {
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

      if (!mqtt.connected()) {
        // TCP connection has broken.
        mqttWasConnected = false;
      }

      if (!mqttWasConnected && connected) {
        // Not connected, but connection available.
        // Try to reconnect, with a second delay between each try.
        uint32_t currentMillis = millis();
        if (currentMillis - mqttLastTry > 1000) {
          mqttWasConnected = mqtt.connect() == 0;
          mqttLastTry = currentMillis;
        }
      }
      if (!connected) {
        mqttWasConnected = false;
      }
    }
};

WiFiClass wifi;
