
#pragma once

#include "button.h"

#include <Adafruit_NeoPixel.h>

#define NUM_LEDS 30

class Ledstrip {
  Button button;
  uint8_t pin;
  Adafruit_NeoPixel strip;
  uint8_t mode;
  uint8_t loopCounter = 0;
  bool buttonWasPressed = false;

public:
  Ledstrip(uint8_t pin, Button button);
  void begin();
  void loop();

private:
  uint8_t applyGamma(uint8_t value) const;
};
void ledstripSetup();
void ledstripLoop();
