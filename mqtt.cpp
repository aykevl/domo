
#include <ArduinoJson.h>

#include "mqtt.h"
#include "config.h"
#include "colorlight.h"
#include "light.h"

WiFiClient mqttClient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, mqttCallback, mqttClient);

const char *MQTT_MSG_ONLINE = "online version: " __DATE__ " " __TIME__;
const char *MQTT_MSG_OFFLINE = "offline";

bool mqttWasConnected = false;
uint32_t mqttLastTry = 0;

void log(String line) {
#ifdef SERIAL_ENABLED
  Serial.println(line);
#endif
#ifdef MQTT_LOG
  line = String(F(CLIENT_ID ": ")) + line;
  mqtt.publish(MQTT_LOG, line.c_str());
#endif
}

void mqttLoop() {
  if (!mqtt.loop()) {
    // Not connected, but connection available.
    // Try to reconnect, with a second delay between each try.
    uint32_t currentMillis = millis();
    if (mqttLastTry == 0 || currentMillis - mqttLastTry > 5000) {
      mqttLastTry = currentMillis;
      if (mqtt.connect(CLIENT_ID, MQTT_LOG, 1, false, MQTT_MSG_OFFLINE)) {
        mqtt.publish(MQTT_LOG, MQTT_MSG_ONLINE);
        if (!mqtt.subscribe(MQTT_PREFIX "a/+", 1)) {
          log(F("failed to subscribe"));
        }
      }
    }
  }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  if (length < 1) {
    // Odd packet
#ifdef SERIAL_ENABLED
    Serial.println("MQTT: got zero-length payload");
#endif
    return;
  }

  // Turn it into a C string (needed for the JSON parser).
  char json[length+1];
  json[length] = 0;
  memcpy(json, payload, length);

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

  const char *origin = root["origin"];
  if (origin != NULL && strcmp(origin, CLIENT_ID) == 0) {
    // Received my own message.
    return;
  }

  if (strcmp(topic, MQTT_PREFIX "a/colorlight") == 0) {
    colorLightReceive(value);
  } else if (strcmp(topic, MQTT_PREFIX "a/wakeup") == 0) {
    lightReceive(1, value);
  } else if (strcmp(topic, MQTT_PREFIX "a/readinglight") == 0) {
    lightReceive(2, value);
  } else {
    log(String("unknown actuator: ") + topic);
  }
}
