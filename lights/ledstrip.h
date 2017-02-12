
#pragma once

#include "button.h"

#include <Adafruit_NeoPixel.h>

#define NUM_LEDS 30

const uint8_t LEDSTRIP_FLAG_RAINBOW_REVERSE = 0b10000000;
const uint8_t LEDSTRIP_FLAG_RAINBOW_RBG     = 0b01000000;
const uint8_t LEDSTRIP_FLAG_SPARKLES        = 0b00100000;
const uint8_t LEDSTRIP_MODE_MASK            = 0b00000111;


class Ledstrip {
  Button button;
  uint8_t pin;
  Adafruit_NeoPixel strip;
  bool stripChanged = true;
  uint8_t mode;
  uint8_t speed; // slowness (higher is slower)
  uint8_t spread; // higher is closer together
  uint8_t palette;
  uint8_t white; // how much white is added
  bool sparkles;
  bool rainbowReverseMovement;
  bool rainbowReverseColor;
  uint8_t loopCounter = 0;
  bool buttonWasPressed = false;
  uint32_t rainbowMillis = 0;
  uint8_t rainbowColor = 0;
  uint32_t noiseMillis = 0;
  uint32_t noiseYScale = 0;

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
