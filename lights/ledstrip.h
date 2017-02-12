
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
  uint8_t speed; // slowness (higher is slower)
  uint8_t palette;
  uint8_t loopCounter = 0;
  bool buttonWasPressed = false;
  uint32_t rainbowMillis = 0;
  uint8_t rainbowColor = 0;
  uint8_t white = 0; // how much white is added

public:
  Ledstrip(uint8_t pin, Button button);
  void begin();
  void loop();
  void sendState() const;
  void gotMessage(uint8_t *arg);

private:
  uint8_t applyGamma(uint8_t value) const;
  void save() const;
};
void ledstripSetup();
void ledstripLoop();
