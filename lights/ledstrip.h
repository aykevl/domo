
#pragma once

#include "button.h"

#include <Adafruit_NeoPixel.h>

#define NUM_LEDS 30

class Ledstrip {
  Button button;
  uint8_t pin;
  Adafruit_NeoPixel strip;
  bool stripChanged = true;
  uint8_t mode;
  uint8_t loopCounter = 0;
  bool buttonWasPressed = false;
  uint32_t lastMillis = 0;
  uint8_t rainbowColor = 0;

public:
  Ledstrip(uint8_t pin, Button button);
  void begin();
  void loop();

private:
  uint8_t applyGamma(uint8_t value) const;
};
void ledstripSetup();
void ledstripLoop();
