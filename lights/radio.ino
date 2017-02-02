#include <SPI.h>
#include <RF24.h>

#include "radio.h"
#include "config.h"

// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10.
RF24 radio(9, 10);

void radioSetup() {
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openReadingPipe(1, (const uint8_t*) "\02" RF24_ADDRESS);
  radio.openWritingPipe((const uint8_t*) "\00" RF24_ADDRESS);
  radio.setChannel(RF24_CHANNEL);
  radio.startListening();
}

void radioLoop() {
  if (!radio.available()) {
    return;
  }
  uint8_t msg[32];
  radio.read(msg, 32);

  bool request = msg[0] >> 7;      // take the most significant bit (as 0 or 1)
  uint8_t command = msg[0] & 0x7f; // take all other bits
  uint8_t child = msg[1];          // child or subdevice number (starting from 1)
  uint8_t *arg = msg+2;            // the rest of the data (30 bytes) is the argument

  Serial.println("got message");
  if (request) {
    Serial.println("got request");
    if (command == RADIO_MSG_LIGHT) {
      if (child == 1) {
        light1.sendState();
      } else if (child == 2) {
        light2.sendState();
      }
    }
  } else {
    if (command == RADIO_MSG_LIGHT) {
      if (child == 1) {
        light1.gotMessage(arg);
      } else if (child == 2) {
        light2.gotMessage(arg);
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
