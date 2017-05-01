
#pragma once

#include <Adafruit_NeoPixel.h>
#include <FastLED.h>

#define NUM_LEDS 30

const uint8_t LEDSTRIP_FLAG_REVERSE     = 0b10000000;
const uint8_t LEDSTRIP_FLAG_RAINBOW_RBG = 0b01000000;
const uint8_t LEDSTRIP_FLAG_SPARKLES    = 0b00100000;
const uint8_t LEDSTRIP_MODE_MASK        = 0b00000111;

typedef enum {
  PALETTE_Rainbow,
  PALETTE_Heat,
  PALETTE_Lava,
  PALETTE_RedBlue,
  PALETTE_RedYellow,
  PALETTE_Ocean,
  PALETTE_EOF,
} palette_t;

typedef enum {
  MODE_OFF,
  MODE_RAINBOW,
  MODE_NOISE,
  MODE_FLAME,
  MODE_WHITE,
  MODE_PALETTE,
} mode_t;


class Ledstrip {
  uint8_t pin;
  Adafruit_NeoPixel strip;
  bool stripChanged = true;
  uint8_t mode;
  uint8_t speed; // speed (higher is faster)
  uint8_t spread; // higher is closer together
  uint8_t paletteIndex;
  CRGBPalette16 palette;
  uint8_t white; // how much white is added
  bool sparkles;
  bool reverse; // reverse the whole strip
  bool rainbowReverseColor;
  uint8_t lastMillis = 0;
  uint8_t rainbowColor = 0;
  uint32_t noiseYScale = 0;

public:
  Ledstrip(uint8_t pin);
  void begin();
  void loop();
  void sendState() const;
  void gotMessage(uint8_t *arg);

private:
  uint8_t applyGamma(uint8_t value) const;
  void loadPalette(palette_t index);
  void save() const;
};

void ledstripSetup();
void ledstripLoop();
