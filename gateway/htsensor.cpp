
#include <ArduinoJson.h>
#include <DHT.h>

#include "config.h"
#include "mqtt.h"
#include "time.h"

float htsensorDecode(uint8_t *arg) {
  uint32_t value = 0;
  for (uint8_t i=4; i; i--) {
    Serial.println(arg[i-1]);
    value <<= 8;
    value |= arg[i-1];
  }
  if (value == 0) {
    return NAN;
  }

  return float(value) / 1000.0;
}

void htsensorSend(uint8_t *arg) {
  uint16_t interval = uint16_t(arg[0]) + uint16_t(arg[1]) * 256;
  float temperature = htsensorDecode(arg+2);
  float humidity = htsensorDecode(arg+6);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["time"] = Clock.timestamp();
  root["interval"] = interval;
  const size_t msgMaxLen = 60; // measured 46 bytes
  char msg[msgMaxLen];
  if (!isnan(temperature)) {
    root.set("value", temperature, 1);
    const size_t msgLen = root.printTo(msg, msgMaxLen);
    mqtt.publish(MQTT_PREFIX "s/temperature", msg);
  }
  if (!isnan(humidity)) {
    root.set("value", humidity, 1);
    const size_t msgLen = root.printTo(msg, msgMaxLen);
    mqtt.publish(MQTT_PREFIX "s/humidity", msg);
  }
}
