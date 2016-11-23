
#pragma once

#include "secrets.h"

class WiFiClass {
  bool mdnsStarted = false;

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

      if (connected) {
        mqttLoop();
      }
    }
};

WiFiClass wifi;
