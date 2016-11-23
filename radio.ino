
#include <RF24.h>
#include <ArduinoJson.h>

#include "radio.h"
#include "config.h"
#include "ufloat8.h"

RF24 radio(D4, D8);

void radioSetup() {
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openReadingPipe(1, (const uint8_t*) "\x00" RF24_ADDRESS);
  radio.openWritingPipe((const uint8_t*) "\x01" RF24_ADDRESS);
  radio.setChannel(RF24_CHANNEL);
  radio.startListening();
}

void radioLoop() {
  if (radio.available()) {
    uint8_t msg[32];
    radio.read(msg, 32);

    bool request = msg[0] >> 7;      // take the most significant bit (as 0 or 1)
    uint8_t command = msg[0] & 0x7f; // take all other bits
    uint8_t *arg = msg+1;            // the rest of the data (31 bytes) is the argument

    if (request) {
    } else {
      if (command == RADIO_MSG_COLOR) {
        colorLightSend(arg);
      }
    }
  }

  // Request the current color once per minute.
  static uint32_t lastMillis = 0;
  uint32_t currentMillis = millis();
  if (lastMillis == 0 || currentMillis - lastMillis >= 60000) {
    lastMillis = currentMillis;

    uint8_t msg[32];
    memset(msg, 0, 32);
    msg[0] = RADIO_MSG_COLOR | RADIO_MSG_REQUEST;

    radio.stopListening();
    if (!radio.write(msg, 32)) {
      log(F("failed to send color request message"));
    }
    radio.startListening();
  }
}

bool radioSend(uint8_t messageType, uint8_t *data, size_t length) {
  if (length > 31) {
    // Message too big.
    return false;
  }

  // Create the message.
  uint8_t msg[32];
  msg[0] = messageType;
  memcpy(msg+1, data, length);

  // Send the message via the nRF24L01.
  radio.stopListening();
  bool success = radio.write(msg, 1+length);
  radio.startListening();

  return success;
}
