
#pragma once

#include "secrets.h"

void wifiSetup() {
  WiFi.begin(SSID, PASSWORD);
}

void wifiLoop() {
  static bool mdnsStarted = false;
  if (!mdnsStarted && WiFi.status() == WL_CONNECTED) {
    if (!MDNS.begin("wakeup")) {
      Serial.println("mDNS responder failed to start");
    } else {
      Serial.println("mDNS responder started");
      mdnsStarted = true;
    }
  }
}
