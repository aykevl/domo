
#pragma once

#include "colorlight.h"

const char *MQTT_MSG_ONLINE = "online";
const char *MQTT_MSG_OFFLINE = "offline";

void mqttCallback(char *topic, byte *payload, unsigned int length);

WiFiClient mqttClient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, mqttCallback, mqttClient);
bool mqttWasConnected = false;
uint32_t mqttLastTry = 0;

void log(String line) {
  Serial.println(line);
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
    Serial.println("MQTT: got zero-length payload");
    return;
  }

  // Turn it into a C string (needed for the JSON parser).
  char value[length+2];
  memset(value, 0, length+2);
  memcpy(value, payload, length);

  if (strcmp(topic, MQTT_PREFIX "a/colorlight") == 0) {
    colorLightReceive(value);
  } else {
    log(String("unknown actuator: ") + topic);
  }
}
