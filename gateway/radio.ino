
#include <RF24.h>
#include <ArduinoJson.h>

#include "radio.h"
#include "config.h"
#include "ufloat8.h"
#include "colorlight.h"
#include "light.h"
#include "htsensor.h"

const uint8_t RADIO_MSG_REQUEST = 0x80;

const uint8_t CHILDREN_LEN = 3;
const char *CHILDREN[CHILDREN_LEN] = {NULL, "wakeup", "readinglight"};

RF24 radio(D4, D8);

void radioSetup() {
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openReadingPipe(1, (const uint8_t*) "\x00" RF24_ADDRESS);
  radio.openWritingPipe((const uint8_t*) "\x02" RF24_ADDRESS);
  radio.setChannel(RF24_CHANNEL);
  radio.startListening();
}

void radioLoop() {
  if (radio.available()) {
    uint8_t msg[32];
    radio.read(msg, 32);

    bool request = msg[0] >> 7;      // take the most significant bit (as 0 or 1)
    uint8_t command = msg[0] & 0x7f; // take all other bits
    uint8_t child = msg[1];
    uint8_t *arg = msg+2;            // the rest of the data (30 bytes) is the argument
    log("got radio message");

    if (!request) {
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
      }
    }
  }

  if (mqtt.connected()) {
    static uint16_t stage = 1; // 1, 2, 3, ...
    if (stage <= 2) {
      // stage 1 and 2 follows

      static uint32_t startMillis = 0;
      if (startMillis == 0) {
        startMillis = millis();
      }

      uint32_t elapsed = millis() - startMillis;
      if (elapsed > stage*100) { // do one stage per 100ms
        uint8_t msg[32];
        memset(msg, 0, 32);
        msg[0] = RADIO_MSG_LIGHT | RADIO_MSG_REQUEST;
        msg[1] = stage;
        log(String("requesting child ") + msg[1]);

        radio.stopListening();
        if (!radio.write(msg, 32)) {
          log(F("failed to send color request message"));
        }
        radio.startListening();

        // Next stage.
        stage++;
      }
    }
  }
}

bool radioSend(uint8_t *msg, size_t length) {
  // Send the message via the nRF24L01.
  radio.stopListening();
  bool success = radio.write(msg, length);
  radio.startListening();

  return success;
}
