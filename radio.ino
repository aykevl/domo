
#include <RF24.h>
#include <ArduinoJson.h>

#include "config.h"
#include "ufloat8.h"

RF24 radio(D4, D8);

enum {
  MSG_NONE,
  MSG_COLOR,
};
uint8_t MSG_REQUEST = 0x80;

const uint8_t COLOR_FLAG_LOOPING   = 0b01000000;
const uint8_t COLOR_MODE_MASK      = 0b00000011; // bit 7-8: mode
const uint8_t COLOR_MODE_RGB       = 0b00000000; // mode value 0
const uint8_t COLOR_MODE_HSV       = 0b00000001; // mode value 1
const uint8_t COLOR_MODE_HSV_MAX   = 0b00000010; // mode value 2
const uint8_t COLOR_MODE_UNDEFINED = 0b00000011; // mode value 3 (undefined)

void radioSetup() {
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openReadingPipe(1, (const uint8_t*) "\x00" RF24_ADDRESS);
  radio.openWritingPipe((const uint8_t*) "\x01" RF24_ADDRESS);
  radio.setChannel(RF24_CHANNEL);
  radio.printDetails();
  radio.startListening();
}

void radioLoop() {
  if (radio.available()) {
    uint8_t msg[32];
    radio.read(msg, 32);

    radioHandleMessage(msg);
  }

  // Request the current color once per minute.
  static uint32_t lastMillis = 0;
  uint32_t currentMillis = millis();
  if (lastMillis == 0 || currentMillis - lastMillis >= 60000) {
    lastMillis = currentMillis;

    uint8_t msg[32];
    memset(msg, 0, 32);
    msg[0] = MSG_COLOR | MSG_REQUEST;

    radio.stopListening();
    if (!radio.write(msg, 32)) {
      mqttLog("failed to send color request message");
    }
    radio.startListening();
  }
}

void radioHandleMessage(uint8_t *msg) {
  bool request = msg[0] >> 7;      // take the most significant bit (as 0 or 1)
  uint8_t command = msg[0] & 0x7f; // take all other bits
  uint8_t *arg = msg+1;            // the rest of the data (31 bytes) is the argument

  if (request) {
  } else {
    if (command == MSG_COLOR) {
      if (!mqtt.connected()) return;

      const size_t messageMaxLen = 170; // ~150 max
      char message[messageMaxLen];
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
      values.set("looping",    (bool)(arg[0] & COLOR_FLAG_LOOPING));
      values.set("time",       (ufloat8_dec(arg[1]) << 8) / 1000.0);
      values.set("hue",        arg[2] / 255.0, 3);
      values.set("saturation", arg[3] / 255.0, 3);
      values.set("value",      arg[4] / 255.0, 3);
      values.set("red",        arg[5] / 255.0, 3);
      values.set("green",      arg[6] / 255.0, 3);
      values.set("blue",       arg[7] / 255.0, 3);

      messageLen = root.printTo(message, messageMaxLen);

      // We just want to update the status once in a while, so it doesn't
      // need a QoS (thus 0).
      // TODO: set the 'retain' bit, once the MQTT library supports it...
      mqtt.publish(MQTT_PREFIX "a/colorlight", (uint8_t*)message, messageLen, 0);
    }
  }
}
