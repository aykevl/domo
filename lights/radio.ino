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
  //light2.sendState();
  ledstrip.sendState();

  radio.startListening();
}

void radioLoop() {
  if (!radio.available()) {

    // Update time.
    // Every second when it isn't known, otherwise every hour.
    static uint16_t lastRequest = 0;
    uint16_t now = millis() / 1000;
    uint16_t waitTime = 3600; // 1 hour
    if (Clock.timestamp() == 0) {
      waitTime = 1; // 1 second
    }
    if (uint16_t(now - lastRequest) >= waitTime) {
      lastRequest = now;
      radioRequestTime();
    }

    // No message available.
    return;
  }

  // There is a message available.

  uint8_t msg[32];
  radio.read(msg, 32);

  bool request = msg[0] >> 7;      // take the most significant bit (as 0 or 1)
  uint8_t command = msg[0] & 0x7f; // take all other bits
  uint8_t child = msg[1];          // child or subdevice number (starting from 1)
  uint8_t *arg = msg+2;            // the rest of the data (30 bytes) is the argument

#ifdef USE_SERIAL
  Serial.println("got message");
#endif
  if (request) {
#ifdef USE_SERIAL
    Serial.println("got request");
#endif
    switch (command) {
      case RADIO_MSG_LIGHT:
        if (child == 1) {
          light1.sendState();
        //} else if (child == 2) {
        //  light2.sendState();
        }
        break;
      case RADIO_MSG_LEDSTRIP:
        ledstrip.sendState();
        break;
    }
  } else {
    switch(command) {
      case RADIO_MSG_LIGHT:
        if (child == 1) {
          light1.gotMessage(arg);
        //} else if (child == 2) {
        //  light2.gotMessage(arg);
        }
        break;

      case RADIO_MSG_TIME:
        radioRecvTime(arg);
        break;

      case RADIO_MSG_LEDSTRIP:
        ledstrip.gotMessage(arg);
        break;
    }
  }
}

void radioRequestTime() {
#ifdef USE_SERIAL
  Serial.println("requesting time");
#endif
  uint8_t msg[2];
  msg[0] = RADIO_MSG_TIME | RADIO_MSG_REQUEST;
  msg[1] = 0;
  if (!radioSend(msg, sizeof(msg))) {
#ifdef USE_SERIAL
  Serial.println("failed to request time");
#endif
  }
}

void radioRecvTime(uint8_t *arg) {
  uint32_t timestamp = 0;
  for (uint8_t i=4; i; i--) { // handle 4 bytes (in little-endian format)
    timestamp <<= 8;
    timestamp |= arg[i-1];
  }
#ifdef USE_SERIAL
  Serial.print(F("time: "));
  Serial.println(timestamp);
#endif
  Clock.setTime(timestamp); // updated time
}


bool radioSend(uint8_t *msg, size_t length) {
  // Send the message via the nRF24L01.
  radio.stopListening();
  bool success = radio.write(msg, length);
  radio.startListening();

  return success;
}
