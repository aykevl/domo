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

  light1.sendState();
  light2.sendState();

  radio.startListening();
}

void radioLoop() {
  if (!radio.available()) {

    // Keep requesting the time every 5 seconds when it isn't yet known.
    if (Clock.timestamp() == 0) {
      static uint32_t lastRequest = 0;
      uint32_t now = millis();
      if (now - lastRequest > 5000) {
        lastRequest = now;
        radioRequestTime();
      }
    }

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
    switch(command) {
      case RADIO_MSG_LIGHT:
        if (child == 1) {
          light1.gotMessage(arg);
        } else if (child == 2) {
          light2.gotMessage(arg);
        }
        break;

      case RADIO_MSG_TIME:
        radioRecvTime(arg);
        break;
    }
  }
}

void radioRequestTime() {
  Serial.println("requesting time");
  uint8_t msg[2];
  msg[0] = RADIO_MSG_TIME | RADIO_MSG_REQUEST;
  msg[1] = 0;
  radioSend(msg, sizeof(msg));
}

void radioRecvTime(uint8_t *arg) {
  uint32_t timestamp = 0;
  for (uint8_t i=4; i; i--) { // handle 4 bytes (in little-endian format)
    timestamp <<= 8;
    timestamp |= arg[i-1];
  }
  Serial.print(F("time: "));
  Serial.println(timestamp);
  Clock.setTime(timestamp); // updated time
}


bool radioSend(uint8_t *msg, size_t length) {
  // Send the message via the nRF24L01.
  radio.stopListening();
  bool success = radio.write(msg, length);
  radio.startListening();

  return success;
}
