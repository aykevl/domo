
#include "ledstrip.h"
#include "math.h"
#include "settings.h"
#include "config.h"
#include "radio.h"

uint8_t flameHeat[NUM_LEDS];

#define FLAME_SPEED (1000 / 60) // 60fps, or 16ms per frame
#define FLAME_COOLING 30
#define FLAME_SPARKING 120

const uint8_t NUM_MODES_BUTTON = 6;
const uint8_t NUM_MODES_ALL = 7;

const TProgmemRGBPalette16 RedBlueColors_p FL_PROGMEM =
{
    0xff6600,
    0xff3300,
    0xff0000,
    0xff0011,
    0xff0044,
    0xee0077,
    0xbb00aa,
    0x8800dd,
    0x6600ff,
    0x4400ff,
    0x2200ff,
    0x0000ff,
    0x0000ff,
    0x2222ff,
    0x4444ff,
    0x6666ff
};

const TProgmemRGBPalette16 RedYellowColors_p FL_PROGMEM =
{
    0xff0088,
    0xff0066,
    0xff0044,
    0xff0022,
    0xff0000,
    0xff1100,
    0xff2200,
    0xee3300,
    0xdd4400,
    0xcc5500,
    0xbb6600,
    0xaa7700,
    0x998800,
    0x889900,
    0x77aa00,
    0x66bb00,
};

const uint8_t PROGMEM gamma8[] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   2,
    2,   3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   5,   5,   5,
    5,   6,   6,   6,   6,   7,   7,   7,   7,   8,   8,   8,   9,   9,   9,  10,
   10,  10,  11,  11,  11,  12,  12,  13,  13,  13,  14,  14,  15,  15,  16,  16,
   17,  17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  24,  24,  25,
   25,  26,  27,  27,  28,  29,  29,  30,  31,  32,  32,  33,  34,  35,  35,  36,
   37,  38,  39,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  50,
   51,  52,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  66,  67,  68,
   69,  70,  72,  73,  74,  75,  77,  78,  79,  81,  82,  83,  85,  86,  87,  89,
   90,  92,  93,  95,  96,  98,  99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
  115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
  144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
  177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
  215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255,
};



Ledstrip::Ledstrip(uint8_t pin) :
  strip(Adafruit_NeoPixel(NUM_LEDS, pin, NEO_GRBW + NEO_KHZ800))
{
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
}

void Ledstrip::begin()
{
  mode = Settings.data.ledstrip_mode;
  speed = Settings.data.ledstrip_speed;
  spread = Settings.data.ledstrip_spread;
  loadPalette(Settings.data.ledstrip_palette);
  white = Settings.data.ledstrip_white;
  sparkles = Settings.data.ledstrip_sparkles;
  updateBrightness(Settings.data.ledstrip_dim);
  strip.begin();
}

void Ledstrip::updateBrightness(uint8_t dim)
{
  if (dim > 20) {
    dim = 20;
  }
  this->dim = dim;
  uint8_t brightness = pgm_read_byte(gamma8 + (0xff - dim * 10));
  strip.setBrightness(brightness);
}

void Ledstrip::loop()
{
  switch (mode) {
    // Turn off the LED strip.
    case MODE_OFF: {
      for (uint8_t i=0; i<NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0, 0));
      }
      break;
    }

    // Show moving rainbow colors.
    case MODE_RAINBOW: {
      uint8_t currentMillis = millis();
      if (uint8_t(currentMillis - lastMillis) >= uint8_t(0xe0 >> speed)) { // may also be: 7 << speed
        stripChanged = true;
        lastMillis = currentMillis;
        if ((!reverse != !rainbowReverseColor)) {
          rainbowColor--;
        } else {
          rainbowColor++;
        }
      }

      uint8_t color = rainbowColor;
      uint8_t i = NUM_LEDS;
      do {
        i--;
        CRGB led = CHSV {
          color,
          0xff,
          0xff,
        };
        if (rainbowReverseColor) {
          color -= spread/9;
        } else {
          color += spread/9;
        }
        strip.setPixelColor(i, strip.Color(
              led.red,
              led.green,
              led.blue,
              white));
      } while (i);

      break;
    }

    // Show noise based on color palette.
    case MODE_NOISE:
    {
      // Move along the Y axis (time).
      uint8_t currentMillis = millis();
      if (uint8_t(currentMillis - lastMillis) > 8) {
        stripChanged = true;
        lastMillis = currentMillis;
        noiseYScale += uint16_t(63) << speed;
      }

      for (uint8_t i=0; i<NUM_LEDS; i++) {
        // X location is constant, but we move along the Y at the rate of millis()
        //uint8_t index = inoise8(i*noise_xscale,millis()*noise_yscale*NUM_LEDS/255);
        uint16_t index = inoise16(uint32_t(i)*128*spread, noiseYScale);

        CRGB fl_rgb = ColorFromPalette16(palette, index);
        strip.setPixelColor(i,strip.Color(
              applyGamma(fl_rgb.red),
              applyGamma(fl_rgb.green),
              applyGamma(fl_rgb.blue),
              white));
      }
      break;
    }

    case MODE_FLAME: {
      uint8_t currentMillis = millis();
      if (uint8_t(currentMillis - lastMillis) > 8) {
        stripChanged = true;
        lastMillis = currentMillis;

        // Step 2.  Heat from each cell drifts 'up' and diffuses a little
        for (uint8_t i = NUM_LEDS - 1; i >= 2; i--) {
          flameHeat[i] = (flameHeat[i - 1] + flameHeat[i - 2] + flameHeat[i - 2] ) / 3;
        }

        // Step 3.  Randomly ignite new 'sparks' of flameHeat near the bottom
        if (random8() < FLAME_SPARKING) {
          uint8_t y = random8(4);
          flameHeat[y] = qadd8( flameHeat[y], random8(160, 255) );
        }

        // Step 4.  Map from heat cells to LED colors
        for (uint8_t i = 0; i < NUM_LEDS; i++) {
          //uint8_t colorindex = scale8(flameHeat[i], 200);
          //leds[i] = ColorFromPalette(palette, colorindex);
          // Optimization: use HeatColor instead of ColorFromPalette.
          // When we use HeatColors_p somewhere else in the sketch, it's
          // probably more space-efficient to use ColorFromPalette16.
          CRGB color = HeatColor(flameHeat[i]);
          strip.setPixelColor(i, strip.Color(
                color.red,
                color.green,
                color.blue,
                white));

          // Step 1.  Cool down every cell a little
          // Optimization: do this in the same loop here.
          flameHeat[i] = qsub8(flameHeat[i], random8(0, (FLAME_COOLING + 2)));
        }
      }

      break;
    }

    case MODE_WHITE: {
      for (uint8_t i=0; i<NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0, 255));
      }
      break;
    }

    // Show color palette.
    case MODE_PALETTE: {
      for (uint8_t i=0; i<NUM_LEDS; i++) {
        uint16_t index = i*8;
        if (index <= 0xff) {
          CRGB fl_rgb = ColorFromPalette(palette, index);
          strip.setPixelColor(i, strip.Color(
                applyGamma(fl_rgb.red),
                applyGamma(fl_rgb.green),
                applyGamma(fl_rgb.blue),
                white));
        } else {
          strip.setPixelColor(i, strip.Color(0, 0, 0, white));
        }
      }
      break;
    }
  }

  // Display sparkle-like flickering
  if (sparkles) {
    stripChanged = true;
    uint16_t currentMillis = millis();
    for (uint8_t i=0; i<NUM_LEDS; i++) {
      // Vary the intensity of the flickering over time and distance.
      uint8_t intensity = inoise8(currentMillis / 4, uint16_t(i)*8);
      if (intensity < 96) {
        intensity = 0;
      } else {
        intensity = (uint16_t(intensity) - 96);
        if (intensity < 128) {
          intensity *= 2;
        } else {
          intensity = 255;
        }
      }
      if (intensity == 0) continue;

      // Make sparkles: display the highest peaks of a fast-moving noise
      // function with a very tight spread, and amplify these peaks.
      // Adjust them for intensity.
      uint8_t amplitude = inoise8(millis()*2+uint16_t(i)*16, uint16_t(i)*128);
      if (amplitude >= 192) {
        amplitude = (amplitude - 192) * 4;;
        amplitude = uint16_t(amplitude) * intensity / 256;
        uint32_t color = strip.getPixelColor(i);
        color |= uint32_t(amplitude) << 24;
        strip.setPixelColor(i, color);
      }
    }
  }

  if (stripChanged) {
    stripChanged = false;
    strip.show();
  }
}


uint8_t Ledstrip::applyGamma(uint8_t value) const {
  return value;
  return pgm_read_byte(gamma8 + value);
}

void Ledstrip::loadPalette(palette_t index) {
  this->paletteIndex = index;
  switch (index) {
    case PALETTE_Rainbow:
      palette = RainbowColors_p;
      break;
    case PALETTE_Heat:
      palette = HeatColors_p;
      break;
    case PALETTE_Lava:
      palette = LavaColors_p;
      break;
    case PALETTE_RedBlue:
      palette = RedBlueColors_p;
      break;
    case PALETTE_RedYellow:
      palette = RedYellowColors_p;
      break;
    case PALETTE_Ocean:
      palette = OceanColors_p;
      break;
  }
}

void Ledstrip::save() const {
  Settings.data.ledstrip_mode = mode;
  Settings.data.ledstrip_speed = speed;
  Settings.data.ledstrip_spread = spread;
  Settings.data.ledstrip_palette = paletteIndex;
  Settings.data.ledstrip_white = white;
  Settings.data.ledstrip_sparkles = sparkles;
  Settings.data.ledstrip_reverse = reverse;
  Settings.data.ledstrip_rainbowReverseColor = rainbowReverseColor;
  Settings.save(); // TODO: throttling
}

void Ledstrip::sendState() const {
  uint8_t msg[8];
  msg[0] = RADIO_MSG_LEDSTRIP;
  msg[1] = 0;
  uint8_t *arg = msg+2;
  arg[0] = mode;
  if (sparkles) {
    arg[0] |= LEDSTRIP_FLAG_SPARKLES;
  }
  if (reverse) {
    arg[0] |= LEDSTRIP_FLAG_REVERSE;
  }
  if (rainbowReverseColor) {
    arg[0] |= LEDSTRIP_FLAG_RAINBOW_RBG;
  }
  arg[1] = speed;
  arg[2] = spread;
  arg[3] = white;
  arg[4] = paletteIndex;
  arg[5] = dim;

  if (!radioSend(msg, sizeof(msg))) {
#ifdef USE_SERIAL
    Serial.println(F("failed to send ledstrip message"));
#endif
  }
}

void Ledstrip::gotMessage(uint8_t *arg) {
#ifdef USE_SERIAL
  Serial.println(arg[0], BIN);
#endif
  if ((arg[0] & LEDSTRIP_MODE_MASK) < NUM_MODES_ALL) {
    mode = arg[0] & LEDSTRIP_MODE_MASK;
  }
  sparkles = (arg[0] & LEDSTRIP_FLAG_SPARKLES) != 0;
  reverse = (arg[0] & LEDSTRIP_FLAG_REVERSE) != 0;
  rainbowReverseColor = (arg[0] & LEDSTRIP_FLAG_RAINBOW_RBG) != 0;
  speed = arg[1];
  spread = arg[2];
  white = arg[3];
  if (arg[4] < PALETTE_EOF) {
    loadPalette(arg[4]);
  }
  if (arg[5] != dim) {
    updateBrightness(arg[5]);
  }

  stripChanged = true;
  save();
}
