
#include <RF24.h>
#include <ArduinoJson.h>

#include "radio.h"
#include "config.h"
#include "ufloat8.h"
#include "colorlight.h"
#include "light.h"
#include "htsensor.h"
#include "ledstrip.h"

const uint8_t CHILDREN_LEN = 3;
const char *CHILDREN[CHILDREN_LEN] = {NULL, "wakeup", "readinglight"};

RF24 radio(D4, D8);

void radioSetup() {
#ifndef RADIO_ENABLED
  return;
#endif
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openReadingPipe(1, (const uint8_t*) "\x00" RF24_ADDRESS);
  radio.openWritingPipe((const uint8_t*) "\x02" RF24_ADDRESS);
  radio.setChannel(RF24_CHANNEL);
  radio.startListening();
}

void radioLoop() {
#ifndef RADIO_ENABLED
  return;
#endif
  if (radio.available()) {
    uint8_t msg[32];
    radio.read(msg, 32);

    bool request = msg[0] >> 7;      // take the most significant bit (as 0 or 1)
    uint8_t command = msg[0] & 0x7f; // take all other bits
    uint8_t child = msg[1];
    uint8_t *arg = msg+2;            // the rest of the data (30 bytes) is the argument

    if (request) {
      switch (command) {
        case RADIO_MSG_TIME:
          radioSendTime();
          break;
      }
    } else {
      switch (command) {
        case RADIO_MSG_COLOR:
          colorLightSend(arg);
          break;
        case RADIO_MSG_LIGHT:
          if (child > 0 && child < CHILDREN_LEN) {
            lightSend(CHILDREN[child], arg);
          }
          break;
        case RADIO_MSG_HT:
          htsensorSend(arg);
          break;
        case RADIO_MSG_LEDSTRIP:
          ledstripSend(arg);
          break;
      }
    }
  }

  if (mqtt.connected()) {
    static uint16_t stage = 1; // 1, 2, 3, ...
    if (stage <= 3) {
      // stage 1-3 follows

      static uint32_t startMillis = 0;
      if (startMillis == 0) {
        startMillis = millis();
      }

      uint32_t elapsed = millis() - startMillis;
      if (elapsed > stage*100) { // do one stage per 100ms
        uint8_t msg[2];
        if (stage == 1 || stage == 2) {
          msg[0] = RADIO_MSG_LIGHT | RADIO_MSG_REQUEST;
          msg[1] = stage;
        } else if (stage == 3) {
          msg[0] = RADIO_MSG_LEDSTRIP | RADIO_MSG_REQUEST;
          msg[1] = 0;
        }

        if (!radioSend(msg, sizeof(msg))) {
          log(F("failed to send request message"));
        }

        // Next stage.
        stage++;
      }
    }

    static uint32_t lastMillis = 0;
    uint32_t currentMillis = millis();
    if (currentMillis - lastMillis > 300000) { // 5 minutes
      lastMillis = currentMillis;
      radioSendTime();
    }
  }
}

void radioSendTime() {
  uint32_t timestamp = Clock.timestamp();
  if (timestamp == 0) return;

  // send current time
  uint8_t msg[6];
  msg[0] = RADIO_MSG_TIME;
  msg[1] = 0;

  // 4 bytes for the time, in little-endian format
  for (uint8_t i=0; i<4; i++) {
    msg[2+i] = uint8_t(timestamp);
    timestamp >>= 8;
  }

  if (!radioSend(msg, sizeof(msg))) {
    log(F("could not send time via radio"));
  }
}

bool radioSend(uint8_t *msg, size_t length) {
#ifndef RADIO_ENABLED
  return false;
#endif
  // Send the message via the nRF24L01.
  radio.stopListening();
  bool success = radio.write(msg, length);
  radio.startListening();

  return success;
}
