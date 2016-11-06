
#pragma once

#include "time.h"
#include "wifi.h"

ESP8266WebServer server(80);

const uint8_t LED_PINS[] = {D2, D3};
bool ledStatus[2] = {false, false};

const char html_root[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<title>Wakeup</title>
<meta name="viewport" content="width=device-width">

<style>
table {
  border-collapse: collapse;
}
table.table-borders > tbody > tr > th,
table.table-borders > tbody > tr > td {
  border: 1px solid #aaa;
  align: left;
}
.input-time {
  border: 1px solid gray;
  white-space: nowrap;
}
input[type=number] {
  border: 1px solid gray;
}
.input-time input[type=number] {
  width: 2em;
  border: none;
  text-align: right;
}
</style>

<h1>Wakeup</h1>

<table class="table-borders">
  <!--<tr>
    <th>D2:</th>
    <td><form method=POST action=/>:led1: <input type=submit name=led1 value=Toggle></form></td>
  </tr>
  <tr>
    <th>D3:</th>
    <td><form method=POST action=/>:led2: <input type=submit name=led2 value=Toggle></form></td>
  </tr>-->
  <tr>
    <th>Wakeup:</th>
    <td>
      <form method="POST" action="/">
        <input type="submit" name="off" value="Off">
        <input type="submit" name="wake" value="Wake">
        <input type="submit" name="on" value="On">
      </form>
      <form method="POST" action="/">
        <table>
          <tr>
            <td>State:</td>
            <td>:wakeup_state: – :wkp</td>
          </tr>
          <tr>
            <td>Start time:</td>
            <td>
              <span class="input-time">
                <input type="number" name="wakeup-hour" value=":H" min="0" max="23">:<input type="number" name="wakeup-minute" value=":M" min="0" max="59">
              </span>
            </td>
          </tr>
          <tr>
            <td>Duration:</td>
            <td>
              <input type="number" name="wakeup-duration" value=":D" min="0" max="60" style="width: 2em">
            </td>
          </tr>
        </table>
        <input type=submit value="Change"/>
      </form>
    </td>
  </tr>
  <tr>
    <th>Current time:</th>
    <td>:time________: (:unixtime:)</td>
  </tr>
  <tr>
    <th>Free heap:</th>
    <td>:freeheap: bytes</td>
  </tr>
</table>
)=====";

const char CONTENT_TYPE_HTML[] PROGMEM = "text/html; charset=utf-8";

void handleRoot() {
  WifiLed.busy();
  int freeHeapInt = ESP.getFreeHeap();

  if (server.method() == HTTP_POST) {
    // TODO: CSRF checking

    char *ledKeys[] = {"led1", "led2"};
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

    if (server.hasArg(F("wakeup-hour")) && server.hasArg(F("wakeup-minute"))) {
      wakeupTime.setHour(String(server.arg(F("wakeup-hour"))).toInt());
      wakeupTime.setMinute(String(server.arg(F("wakeup-minute"))).toInt());
    }
    if (server.hasArg(F("wakeup-duration"))) {
      int32_t duration = String(server.arg(F("wakeup-duration"))).toInt();
      if (duration >= 0 && duration <= 60) {
        wakeupDuration = duration * 60000;
      }
    }

    server.sendHeader(F("Location"), F("/"));
    server.send(303, NULL);
    WifiLed.done();
    return;
  }

  // Use strings with the exact same length, as that replacement is much
  // faster (only needs to replace the required bytes).
  String root = String(FPSTR(html_root));
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

  uint64_t now = Clock.timestamp();
  String timestamp = String((uint32_t)(now / 1000000)) + String((uint32_t)(now % 1000000));
  root.replace(F(":unixtime:"), timestamp);

  Time t = Time(now);
  root.replace(F(":time________:"), t.format());

  root.replace(F(":H"), wakeupTime.formatHour());
  root.replace(F(":M"), wakeupTime.formatMinute());
  root.replace(F(":D"), String(wakeupDuration / 60000));

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

void handleNotFound(){
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
  server.send(404, "text/plain", message);
  WifiLed.done();
}

void serverSetup() {
  for (int i=0; i<2; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], ledStatus[i]);
  }

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
}

void serverLoop() {
  server.handleClient();
}
