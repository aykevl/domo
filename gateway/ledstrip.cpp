
#include <ArduinoJson.h>

#include "ledstrip.h"
#include "mqtt.h"
#include "radio.h"
#include "config.h"

void ledstripSend(uint8_t *arg) {
  if (!mqtt.connected()) return;

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["origin"] = CLIENT_ID;
  JsonObject& values = root.createNestedObject("value");

  switch (arg[0]) {
    case LEDSTRIP_OFF:
      values["mode"] = "off";
      break;
    case LEDSTRIP_COLOR:
      values["mode"] = "color";
      break;
    case LEDSTRIP_NOISE:
      values["mode"] = "noise";
      break;
    case LEDSTRIP_RANDOMNOISE:
      values["mode"] = "randomnoise";
      break;
    case LEDSTRIP_WHITE:
      values["mode"] = "white";
      break;
    case LEDSTRIP_PALETTE:
      values["mode"] = "palette";
      break;
  }
  values["speed"] = 1.0-log(float(arg[1])+1.0)/5.545;
  values["white"] = float(arg[2]) / 255.0;
  values["palette"] = arg[3];

  const size_t messageMaxLen = 192; // TODO determine
  uint8_t message[messageMaxLen];
  size_t messageLen = root.printTo((char*)message, messageMaxLen);

  mqtt.publish(MQTT_PREFIX "a/ledstrip", message, messageLen, true);
}

void ledstripReceive(JsonObject &value) {
  uint8_t msg[6];
  msg[0] = RADIO_MSG_LEDSTRIP;
  msg[1] = 0;

  uint8_t *arg = msg+2;
  arg[0] = LEDSTRIP_OFF;
  const char *mode = value["mode"];
  if (mode != NULL) {
    if (strcmp(mode, "off") == 0) {
      arg[0] = LEDSTRIP_OFF;
    } else if (strcmp(mode, "color") == 0) {
      arg[0] = LEDSTRIP_COLOR;
    } else if (strcmp(mode, "noise") == 0) {
      arg[0] = LEDSTRIP_NOISE;
    } else if (strcmp(mode, "randomnoise") == 0) {
      arg[0] = LEDSTRIP_RANDOMNOISE;
    } else if (strcmp(mode, "white") == 0) {
      arg[0] = LEDSTRIP_WHITE;
    } else if (strcmp(mode, "palette") == 0) {
      arg[0] = LEDSTRIP_PALETTE;
    }
  }

  arg[1] = exp((1.0-float(value["speed"]))*5.545)-1.0 + 0.5;
  arg[2] = float(value["white"]) * 255.0 + 0.5;
  arg[3] = value["palette"];

  if (!radioSend(msg, sizeof(msg))) {
    log(F("couldn't sent ledstrip update"));
  }
}
