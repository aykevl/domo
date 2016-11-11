
#include "DHT.h"

const uint8_t HT_PIN = D6;
const uint32_t HT_INTERVAL = 5000; // 5s
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
      if (currentMillis - lastMillis > HT_INTERVAL) {
        lastMillis = currentMillis;
        temperature = _dht.readTemperature(false, false);
        humidity = _dht.readHumidity(false);
      }
    }

    float getTemperature() {
      return temperature;
    }

    float getHumidity () {
      return humidity;
    }
};
