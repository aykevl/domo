
#include <Arduino.h>
#include <ArduinoJson.h>

#include "light.h"
#include "mqtt.h"
#include "config.h"
#include "radio.h"

void lightSend(const char *child, uint8_t *arg) {
  if (!mqtt.connected()) return;

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["origin"] = CLIENT_ID;
  JsonObject& values = root.createNestedObject("value");

  switch (arg[0] & LIGHT_FLAG_STATUS_MASK) {
    case LIGHT_OFF:
      values["state"] = "off";
      break;
    case LIGHT_WAKE:
      values["state"] = "wake";
      break;
    case LIGHT_ON:
      values["state"] = "on";
      break;
  }
  values["enabled"] = bool(arg[0] & LIGHT_FLAG_ENABLED);
  values["fullBrightness"] = arg[1] / 255.0;
  // ignore time
  values["time"] = Time(arg[6], arg[7], 0).dayTime(); // hour, minute, second
  values["duration"] = (float(arg[8]) + float(arg[9]) * 256.0) * 60.0;

  const size_t messageMaxLen = 192; // ~135 TODO
  uint8_t message[messageMaxLen];
  size_t messageLen = root.printTo((char*)message, messageMaxLen);

  String topic = String(MQTT_PREFIX "a/") + child;
  mqtt.publish(topic.c_str(), message, messageLen, true);
}

void lightReceive(uint8_t child, JsonObject &value) {
  // Message format:
  // 0: type
  // 1: child
  // 2: mode/flags, see LIGHT_FLAG_* constants
  // 3: 0-255 'float' for fullBrightness value (0%-100% max brightness)
  // 4-7: timestamp (uint32, Unix epoch)
  // 8: wakeup hour
  // 9: wakeup minute
  // 10-11: wakeup duration (dawn time)

  log("got light change");
  uint8_t msg[12];
  msg[0] = RADIO_MSG_LIGHT;
  msg[1] = child;

  uint8_t *arg = msg+2;
  arg[0] = LIGHT_UNDEFINED;
  const char *state = value["state"];
  if (state != NULL) {
    if (strcmp(state, "off") == 0) {
      arg[0] = LIGHT_OFF;
    } else if (strcmp(state, "wake") == 0) {
      arg[0] = LIGHT_WAKE;
    } else if (strcmp(state, "on") == 0) {
      arg[0] = LIGHT_ON;
    }
  }

  arg[0] |= (value["enabled"]) ? LIGHT_FLAG_ENABLED : 0;
  arg[1] = uint8_t(float(value["fullBrightness"]) * 255.5);
  uint32_t timestamp = Clock.timestamp(); // 4 bytes for the time, in little-endian format
  for (uint8_t i=0; i<4; i++) {
    arg[2+i] = uint8_t(timestamp);
    timestamp >>= 8;
  }
  Time time = Time(uint32_t(value["time"]));
  arg[6] = time.getHour();
  arg[7] = time.getMinute();
  uint32_t duration = uint32_t(float(value["duration"])) / 60;
  arg[8] = duration % 256;
  arg[9] = duration / 256;

  if (!radioSend(msg, sizeof(msg))) {
    log(F("couldn't sent light update"));
  }
}
