
#pragma once

#include "secrets.h"

void wifiSetup() {
  WiFi.begin(SSID, PASSWORD);
}

void wifiLoop() {
  static bool mdnsStarted = false;
  if (!mdnsStarted && WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.setHostname("wakeup");
#ifdef OTA_PASSWORD
    ArduinoOTA.setPassword(OTA_PASSWORD);
#endif

    // This also starts the mDNS server
    ArduinoOTA.begin();

    // Not sure whether we need this...
    MDNS.addService("http", "tcp", 80);
  }
}
