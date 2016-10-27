
#pragma once

#include <Arduino.h>

// states in the state machine
const uint8_t RELEASED = 0;
const uint8_t RELEASED_BOUNCE = 1;
const uint8_t PRESSED = 2;
const uint8_t PRESSED_BOUNCE = 3;
const uint8_t LONGPRESSED = 4;
const uint8_t LONGPRESSED_BOUNCE = 5;

const unsigned long BOUNCE_TIMEOUT = 5;
const unsigned long LONGPRESS_TIMEOUT = 1000;

class Button {
public:
  inline Button();
  inline Button(uint8_t pin);
  void loop();
  inline uint8_t read();
  inline bool pressed();
private:
  uint8_t state = RELEASED;
  unsigned long time = 0;
  uint8_t pin = 0xff; // do not use this value!
};
