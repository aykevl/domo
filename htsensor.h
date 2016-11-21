
#include <ArduinoJson.h>

#include "DHT.h"

const uint8_t HT_PIN = D6;
const uint32_t HT_INTERVAL = 60000; // 1 minute
DHT _dht(HT_PIN, DHT22);

class HTSensor {
  uint32_t lastMillis = 0;
  float humidity = NAN;
  float temperature = NAN;

  public:
    void setup() {
      _dht.begin();
    }

    void loop() {
      uint32_t currentMillis = millis();
      if (lastMillis == 0 || currentMillis - lastMillis > HT_INTERVAL) {
        lastMillis = currentMillis;
        temperature = _dht.readTemperature(false, false);
        humidity = _dht.readHumidity(false);

        yield();

        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        root["time"] = Clock.timestamp();
        root["interval"] = (HT_INTERVAL / 1000);
        const size_t msgMaxLen = 60; // measured 46 bytes
        char msg[msgMaxLen];
        if (!isnan(temperature)) {
          root.set("value", temperature, 1);
          const size_t msgLen = root.printTo(msg, msgMaxLen);
          mqtt.publish(MQTT_PREFIX "s/temperature", (uint8_t*)msg, msgLen, 1);
        }
        if (!isnan(humidity)) {
          root.set("value", humidity, 1);
          const size_t msgLen = root.printTo(msg, msgMaxLen);
          mqtt.publish(MQTT_PREFIX "s/humidity", (uint8_t*)msg, msgLen, 1);
        }
      }
    }

    float getTemperature() {
      return temperature;
    }

    float getHumidity () {
      return humidity;
    }
};
