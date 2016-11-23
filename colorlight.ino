
#include "radio.h"

const uint8_t COLOR_FLAG_DISABLED  = 0b10000000;
const uint8_t COLOR_FLAG_LOOPING   = 0b01000000;
const uint8_t COLOR_FLAG_REVERSE   = 0b00100000;
const uint8_t COLOR_MODE_MASK      = 0b00000011; // bit 7-8: mode
const uint8_t COLOR_MODE_RGB       = 0b00000000; // mode value 0
const uint8_t COLOR_MODE_HSV       = 0b00000001; // mode value 1
const uint8_t COLOR_MODE_HSV_MAX   = 0b00000010; // mode value 2
const uint8_t COLOR_MODE_UNDEFINED = 0b00000011; // mode value 3 (undefined)

void colorLightSend(uint8_t *arg) {
  if (!mqtt.connected()) return;

  const size_t messageMaxLen = 170; // ~150 max
  uint8_t message[messageMaxLen];
  size_t messageLen = 0;

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& values = root.createNestedObject("value");
  switch (arg[0] & COLOR_MODE_MASK) {
    case COLOR_MODE_RGB:
      values["mode"] = "rgb";
      break;
    case COLOR_MODE_HSV:
      values["mode"] = "hsv";
      break;
    case COLOR_MODE_HSV_MAX:
      values["mode"] = "hsv-max";
      break;
    case COLOR_MODE_UNDEFINED:
      values["mode"] = "undefined-1";
      break;
    default:
      // Unreachable. It looks like the compiler realizes this and
      // optimizes the following line out.
      values["mode"] = "!unreachable";
      break;
  }
  values.set("disabled",   (bool)(arg[0] & COLOR_FLAG_DISABLED));
  values.set("looping",    (bool)(arg[0] & COLOR_FLAG_LOOPING));
  values.set("reverse",    (bool)(arg[0] & COLOR_FLAG_REVERSE));
  values.set("time",       (ufloat8_dec(arg[1]) << 8) / 1000.0);
  values.set("hue",        arg[2] / 255.0, 3);
  values.set("saturation", arg[3] / 255.0, 3);
  values.set("value",      arg[4] / 255.0, 3);
  values.set("red",        arg[5] / 255.0, 3);
  values.set("green",      arg[6] / 255.0, 3);
  values.set("blue",       arg[7] / 255.0, 3);

  messageLen = root.printTo((char*)message, messageMaxLen);

  // We just want to update the status once in a while, so it doesn't
  // need a QoS (thus 0).
  mqtt.publish(MQTT_PREFIX "a/colorlight", message, messageLen, true);
}

void colorLightReceive(char *json) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success()) {
    log(F("colorlight: could not parse JSON"));
    return;
  }
  JsonObject& value = root["value"];
  if (!value.success()) {
    log(F("colorlight: no 'value' key"));
    return;
  }

  uint8_t msg[8];
  const char *mode = value["mode"];
  if (mode == NULL) {
    log(F("colorlight: no 'mode' key"));
    return;
  }
  if (strcmp(mode, "rgb") == 0) {
    msg[0] = COLOR_MODE_RGB;
  } else if (strcmp(mode, "hsv") == 0) {
    msg[0] = COLOR_MODE_HSV;
  } else if (strcmp(mode, "hsv-max") == 0) {
    msg[0] = COLOR_MODE_HSV_MAX;
  } else {
    msg[0] = COLOR_MODE_UNDEFINED;
  }
  if (value["disabled"]) {
    msg[0] |= COLOR_FLAG_DISABLED;
  }
  if (value["looping"]) {
    msg[0] |= COLOR_FLAG_LOOPING;
  }
  if (value["reverse"]) {
    msg[0] |= COLOR_FLAG_REVERSE;
  }
  float time = value["time"];
  msg[1] = ufloat8_enc((uint32_t)(time * 1000.0) >> 8);
  msg[2] = (float)value["hue"]        * 255.499;
  msg[3] = (float)value["saturation"] * 255.499;
  msg[4] = (float)value["value"]      * 255.499;
  msg[5] = (float)value["red"]        * 255.499;
  msg[6] = (float)value["green"]      * 255.499;
  msg[7] = (float)value["blue"]       * 255.499;

  if (!radioSend(RADIO_MSG_COLOR, msg, 8)) {
    log(F("couldn't sent light update"));
  }
}
