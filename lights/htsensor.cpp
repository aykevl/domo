
#include <Arduino.h>
#include <DHT.h>

#include "radio.h"
#include "config.h"

const uint8_t DHT_PIN = 7;
const uint32_t DHT_INTERVAL = 60000; // 1 minute

DHT dht(DHT_PIN, DHT22);

void htsensorSetup() {
  dht.begin();
}

void htsensorEncode(uint8_t *arg, float fvalue) {
  uint32_t value = fvalue * 1000.0;
  for (uint8_t i=0; i<4; i++) {
    arg[i] = value;
    value >>= 8;
  }
}

void htsensorLoop() {
  // Send once per DHT_INTERVAL
  static uint32_t lastMillis = 0;
  uint32_t currentMillis = millis();
  if (currentMillis - lastMillis <= DHT_INTERVAL) {
    return;
  }
  lastMillis += DHT_INTERVAL;

  // Read sensor
  float temperature = dht.readTemperature(false, false);
  float humidity = dht.readHumidity(false);

  // Send both temperature and humidity
  uint8_t msg[12];
  msg[0] = RADIO_MSG_HT;
  msg[1] = 0; // default
  uint8_t *arg = msg+2;
  uint16_t interval = DHT_INTERVAL / 1000;
  arg[0] = interval % 256; // little endian format
  arg[1] = interval / 256;
  htsensorEncode(&arg[2], temperature);
  htsensorEncode(&arg[6], humidity);

  if (!radioSend(msg, sizeof(msg))) {
#ifdef USE_SERIAL
    Serial.println(F("failed to send DHT message"));
#endif
  }
}
