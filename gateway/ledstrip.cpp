
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

  switch (arg[0] & LEDSTRIP_MODE_MASK) {
    case LEDSTRIP_OFF:
      values["mode"] = "off";
      break;
    case LEDSTRIP_RAINBOW:
      values["mode"] = "rainbow";
      break;
    case LEDSTRIP_NOISE:
      values["mode"] = "noise";
      break;
    case LEDSTRIP_FLAME:
      values["mode"] = "flame";
      break;
    case LEDSTRIP_WHITE:
      values["mode"] = "white";
      break;
    case LEDSTRIP_PALETTE:
      values["mode"] = "palette";
      break;
  }
  values["sparkles"] = (arg[0] & LEDSTRIP_FLAG_SPARKLES) != 0;
  values["rainbowBackwards"] = (arg[0] & LEDSTRIP_FLAG_RAINBOW_REVERSE) != 0;
  values["rainbowRBG"] = (arg[0] & LEDSTRIP_FLAG_RAINBOW_RBG) != 0;
  values["slowness"] = arg[1];
  values["spread"] = float(arg[2]) / 255.0;
  values["white"] = float(arg[3]) / 255.0;
  values["palette"] = arg[4];

  const size_t messageMaxLen = 192; // TODO determine
  uint8_t message[messageMaxLen];
  size_t messageLen = root.printTo((char*)message, messageMaxLen);

  mqtt.publish(MQTT_PREFIX "a/ledstrip", message, messageLen, true);
}

void ledstripReceive(JsonObject &value) {
  uint8_t msg[7];
  msg[0] = RADIO_MSG_LEDSTRIP;
  msg[1] = 0;

  uint8_t *arg = msg+2;
  arg[0] = LEDSTRIP_OFF;
  const char *mode = value["mode"];
  if (mode != NULL) {
    if (strcmp(mode, "off") == 0) {
      arg[0] = LEDSTRIP_OFF;
    } else if (strcmp(mode, "rainbow") == 0) {
      arg[0] = LEDSTRIP_RAINBOW;
    } else if (strcmp(mode, "noise") == 0) {
      arg[0] = LEDSTRIP_NOISE;
    } else if (strcmp(mode, "flame") == 0) {
      arg[0] = LEDSTRIP_FLAME;
    } else if (strcmp(mode, "white") == 0) {
      arg[0] = LEDSTRIP_WHITE;
    } else if (strcmp(mode, "palette") == 0) {
      arg[0] = LEDSTRIP_PALETTE;
    }
  }

  if (bool(value["sparkles"])) {
    arg[0] |= LEDSTRIP_FLAG_SPARKLES;
  }
  if (bool(value["rainbowBackwards"])) {
    arg[0] |= LEDSTRIP_FLAG_RAINBOW_REVERSE;
  }
  if (bool(value["rainbowRBG"])) {
    arg[0] |= LEDSTRIP_FLAG_RAINBOW_RBG;
  }

  arg[1] = value["speed"];
  arg[2] = float(value["spread"]) * 255.0 + 0.5;
  arg[3] = float(value["white"]) * 255.0 + 0.5;
  arg[4] = value["palette"];

  if (!radioSend(msg, sizeof(msg))) {
    log(F("couldn't sent ledstrip update"));
  }
}
