
#pragma once

#include "time.h"
#include "wifi.h"
#include "assets.h"
#include "base64.h"
#include "blake2.h"
#include "http-session.h"

ESP8266WebServer server(80);

const char CONTENT_TYPE_PLAIN[] PROGMEM = "text/plain";
const char CONTENT_TYPE_HTML[] PROGMEM = "text/html; charset=utf-8";
const char CONTENT_TYPE_CSS[] PROGMEM = "text/css";

void handleLogin() {
  if (server.method() == HTTP_POST && server.hasArg(F("password"))) {
    if (httpSessionLogin(server)) {
      server.sendHeader(F("Location"), F("."));
      server.send(303, NULL);
      return;
    }
  }

  if (httpSessionIsAuthenticated(server)) {
    server.sendHeader(F("Location"), F("."));
    server.send(303, NULL);
    return;
  }

  // Not logged in - show login page.
  server.send(200, FPSTR(CONTENT_TYPE_HTML), FPSTR(asset_html_login));
}

void handleRoot() {
  int freeHeapInt = ESP.getFreeHeap();

  if (!httpSessionIsAuthenticated(server)) {
    server.sendHeader(F("Location"), F("./login"));
    server.send(303, NULL);
    return;
  }

  // Use strings with the exact same length, as that replacement is much
  // faster (only needs to replace the required bytes).
  String root = String(FPSTR(asset_html_root));

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

  root.replace(F(":*T:"), String(htsensor.getTemperature(), 1));
  root.replace(F(":*H:"), String(htsensor.getHumidity(), 1));

  server.send(200, FPSTR(CONTENT_TYPE_HTML), root);
}

void handleNotFound() {
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
}

void serverSetup() {
  // For authentication / sessions.
  const char *headers[] = {"Cookie"};
  server.collectHeaders(headers, 1);

  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/style.css", []() {
    server.sendHeader(F("Cache-Control"), F("public,max-age=3600")); // 1 hour
    // Send the mtime of the source file as the Last-Modified date.
    // This Last-Modified date (in __TIMESTAMP__) is in the ANSI C
    // asctime() format. It is obsolete, but still supported.
    // Reference: https://tools.ietf.org/html/rfc7231#section-7.1.1.1
    server.sendHeader(F("Last-Modified"), FPSTR(asset_date));
    server.send(200, FPSTR(CONTENT_TYPE_CSS), FPSTR(asset_css));
  });
  server.onNotFound(handleNotFound);
  server.begin();
}

void serverLoop() {
  server.handleClient();
}
