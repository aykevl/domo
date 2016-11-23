
#include <ArduinoJson.h>

#include "DHT.h"

const uint8_t HT_PIN = D3;
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
          mqtt.publish(MQTT_PREFIX "s/temperature", msg);
        }
        if (!isnan(humidity)) {
          root.set("value", humidity, 1);
          const size_t msgLen = root.printTo(msg, msgMaxLen);
          mqtt.publish(MQTT_PREFIX "s/humidity", msg);
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
