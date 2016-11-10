
#pragma once

#include "time.h"
#include "wifi.h"
#include "assets.h"
#include "base64.h"
#include "blake2.h"
#include "http-session.h"

ESP8266WebServer server(80);

const uint8_t LED_PINS[] = {D2, D3};
bool ledStatus[2] = {false, false};

const char CONTENT_TYPE_PLAIN[] PROGMEM = "text/plain";
const char CONTENT_TYPE_HTML[] PROGMEM = "text/html; charset=utf-8";
const char CONTENT_TYPE_CSS[] PROGMEM = "text/css";

void handleLogin() {
  WifiLed.busy();

  if (server.method() == HTTP_POST && server.hasArg(F("password"))) {
    if (httpSessionLogin(server)) {
      server.sendHeader(F("Location"), F("."));
      server.send(303, NULL);
      WifiLed.done();
      return;
    }
  }

  if (httpSessionIsAuthenticated(server)) {
    server.sendHeader(F("Location"), F("."));
    server.send(303, NULL);
    WifiLed.done();
    return;
  }

  // Not logged in - show login page.
  server.send(200, FPSTR(CONTENT_TYPE_HTML), FPSTR(asset_html_login));

  WifiLed.done();
}

void handleRoot() {
  WifiLed.busy();
  int freeHeapInt = ESP.getFreeHeap();

  if (!httpSessionIsAuthenticated(server)) {
    server.sendHeader(F("Location"), F("./login"));
    server.send(303, NULL);
    WifiLed.done();
    return;
  }

  if (server.method() == HTTP_POST) {
    // TODO: CSRF checking

    const char *ledKeys[] = {"led1", "led2"};
    for (int i=0; i<2; i++) {
      if (server.hasArg(ledKeys[i])) {
        ledStatus[i] = !ledStatus[i];
        digitalWrite(LED_PINS[i], ledStatus[i]);
      }
    }

    if (server.hasArg(F("off"))) {
      light.off();
    } else if (server.hasArg(F("wake"))) {
      light.wake();
    } else if (server.hasArg(F("on"))) {
      light.on();
    }

    if (server.hasArg(F("wakeup-hour")) && server.hasArg(F("wakeup-minute")) && server.hasArg("wakeup-duration")) {
      int32_t hour = String(server.arg(F("wakeup-hour"))).toInt();
      int32_t minute = String(server.arg(F("wakeup-minute"))).toInt();
      int32_t duration = String(server.arg(F("wakeup-duration"))).toInt();
      light.setWakeup(hour, minute, duration);
    }

    server.sendHeader(F("Location"), F("."));
    server.send(303, NULL);
    WifiLed.done();
    return;
  }

  // Use strings with the exact same length, as that replacement is much
  // faster (only needs to replace the required bytes).
  String root = String(FPSTR(asset_html_root));
  if (ledStatus[0]) {
    root.replace(F(":led1:"), F("HIGH  "));
  } else {
    root.replace(F(":led1:"), F("LOW   "));
  }
  if (ledStatus[1]) {
    root.replace(F(":led2:"), F("HIGH  "));
  } else {
    root.replace(F(":led2:"), F("LOW   "));
  }

  String freeHeap = String(freeHeapInt);
  freeHeap.reserve(10);
  while (freeHeap.length() < 10) {
    freeHeap.concat(' ');
  }
  root.replace(F(":freeheap:"), freeHeap);

  uint32_t now = Clock.timestamp();
  String timestamp(now);
  root.replace(F(":unixtime:"), timestamp);

  Time t = Time(now);
  root.replace(F(":time________:"), t.format());

  root.replace(F(":H"), light.getTime().formatHour());
  root.replace(F(":M"), light.getTime().formatMinute());
  root.replace(F(":D"), String(light.getDuration() / 60000));

  switch (light.currentState()) {
    case LIGHT_OFF: {
      root.replace(F(":wakeup_state:"), F("Light off     "));
      break;
    }
    case LIGHT_SLOWSTART: {
      root.replace(F(":wakeup_state:"), F("Waking up     "));
      break;
    }
    case LIGHT_ON: {
      root.replace(F(":wakeup_state:"), F("Light on      "));
      break;
    }
  }

  String p((uint8_t)(light.currentBrightness()*100.0));
  String percent(F("   %"));
  for (uint8_t i=0; i<p.length(); i++) {
    percent[i+(3-p.length())] = p[i];
  }
  root.replace(F(":wkp"), percent);

  server.send(200, FPSTR(CONTENT_TYPE_HTML), root);

  WifiLed.done();
}

void handleNotFound() {
  WifiLed.busy();
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += F("\nArguments: ");
  message += server.args();
  message += '\n';
  for (uint8_t i=0; i<server.args(); i++){
    message += ' ' + server.argName(i) + F(": ") + server.arg(i) + '\n';
  }
  server.send(404, FPSTR(CONTENT_TYPE_PLAIN), message);
  WifiLed.done();
}

void serverSetup() {
  for (int i=0; i<2; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], ledStatus[i]);
  }

  // For authentication / sessions.
  const char *headers[] = {"Cookie"};
  server.collectHeaders(headers, 1);

  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/style.css", []() {
    WifiLed.busy();
    server.sendHeader(F("Cache-Control"), F("public,max-age=3600")); // 1 hour
    // Send the mtime of the source file as the Last-Modified date.
    // This Last-Modified date (in __TIMESTAMP__) is in the ANSI C
    // asctime() format. It is obsolete, but still supported.
    // Reference: https://tools.ietf.org/html/rfc7231#section-7.1.1.1
    server.sendHeader(F("Last-Modified"), FPSTR(asset_date));
    server.send(200, FPSTR(CONTENT_TYPE_CSS), FPSTR(asset_css));
    WifiLed.done();
  });
  server.onNotFound(handleNotFound);
  server.begin();
}

void serverLoop() {
  server.handleClient();
}
