
#include "ledstrip.h"
#include "math.h"
#include "settings.h"
#include "radio.h"

#include <hsv2rgb.h> // FastLED
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define BRIGHTNESS 255

const uint8_t NUM_MODES_BUTTON = 4;
const uint8_t NUM_MODES_ALL = 5;

const uint8_t NUM_PALETTES = 3;
const CRGBPalette16 palettes[NUM_PALETTES] = {
  CRGBPalette16( // red-blue, green-red
    0x000000,
    0x000000,
    0xff0000,
    0xff8800,
    0x66aa00,
    0x000000,
    0x000000,
    0x000088,
    0x0000ff,
    0x8800ee,
    0x9900cc,
    0xff00aa,
    0xff0088,
    0xff0044,
    0xff0000,
    0xff3300),
  CRGBPalette16( // red-blue
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
    0x6666ff),
  CRGBPalette16( // RGB
    0xff0000,
    0x00ff00,
    0x0000ff,
    0xff0000),
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



Ledstrip::Ledstrip(uint8_t pin, Button button) :
  strip(Adafruit_NeoPixel(NUM_LEDS, pin, NEO_GRBW + NEO_KHZ800)),
  button(button)
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
  white = Settings.data.ledstrip_white;
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
}

void Ledstrip::loop()
{
  // Count numer of loops since start.
  loopCounter++;

  // Go to next mode on keypress.
  button.loop();
  bool buttonPressed = button.pressed();
  if (buttonPressed && !buttonWasPressed) {
    // Registered a press!
    if (mode == 2) {
      // loop through palettes when in color palette mode
      if (palette+1 < NUM_PALETTES) {
        palette++;
      } else {
        palette = 0;
        mode++;
      }
    } else if (mode+1 < NUM_MODES_BUTTON) {
      mode++;
    } else {
      // end of loop
      mode = 0;
    }
    stripChanged = true;
    save();
    sendState();
  }
  buttonWasPressed = buttonPressed;

  switch (mode) {
    // Turn off the LED strip.
    case 0: {
      for (uint8_t i=0; i<NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0, 0));
      }
      break;
    }

    // Show moving rainbow colors.
    case 1: {
      uint32_t currentMillis = millis();
      if (stripChanged || currentMillis - rainbowMillis >= uint32_t(speed)*4) {
        if (speed != 0xff) {
          rainbowColor++;
          if (currentMillis - rainbowMillis > uint32_t(speed)*4*2) {
            // we're just getting into this mode (or there was a long delay)
            rainbowMillis = currentMillis;
          } else {
            // we were already in this mode
            rainbowMillis += uint32_t(speed)*4;
          }
        }
        stripChanged = true;

        uint8_t color = rainbowColor;
        for (uint8_t i=0; i<NUM_LEDS; i++) {
          const CHSV fl_hsv = CHSV {
            color,
            0xff,
            0xff,
          };
          color -= spread/9;
          CRGB fl_rgb;
          hsv2rgb_rainbow(fl_hsv, fl_rgb);
          strip.setPixelColor(i, strip.Color(
                fl_rgb.red,
                fl_rgb.green,
                fl_rgb.blue,
                white));
        }
      }
      break;
    }

    // Show noise based on color palette.
    case 2:
    {
      stripChanged = true;

      // Move along the Y axis (time).
      uint32_t currentMillis = millis();
      if (currentMillis - noiseMillis > speed) {
        noiseMillis = currentMillis;
        noiseYScale += 256;
      }

      for (uint8_t i=0; i<NUM_LEDS; i++) {
        // X location is constant, but we move along the Y at the rate of millis()
        //uint8_t index = inoise8(i*noise_xscale,millis()*noise_yscale*NUM_LEDS/255);
        uint16_t index = inoise16(uint32_t(i)*128*spread, noiseYScale);

        CRGB fl_rgb = ColorFromPalette16(palettes[palette], index);
        strip.setPixelColor(i,strip.Color(
              applyGamma(fl_rgb.red),
              applyGamma(fl_rgb.green),
              applyGamma(fl_rgb.blue),
              white));
      }
      break;
    }

    case 3: { // white
      for (uint8_t i=0; i<NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0, 255));
      }
      break;
    }

    // Show color palette.
    case 4: {
      for (uint8_t i=0; i<NUM_LEDS; i++) {
        uint16_t index = i*8;
        if (index <= 0xff) {
          CRGB fl_rgb = ColorFromPalette(palettes[palette], index);
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

  if (stripChanged) {
    stripChanged = false;
    strip.show();
  }
}


uint8_t Ledstrip::applyGamma(uint8_t value) const {
  return value;
  return pgm_read_byte(gamma8 + value);
}

void Ledstrip::save() const {
  Settings.data.ledstrip_mode = mode;
  Settings.data.ledstrip_speed = speed;
  Settings.data.ledstrip_spread = spread;
  Settings.data.ledstrip_white = white;
  Settings.save(); // TODO: throttling
}

void Ledstrip::sendState() const {
  uint8_t msg[7];
  msg[0] = RADIO_MSG_LEDSTRIP;
  msg[1] = 0;
  uint8_t *arg = msg+2;
  arg[0] = mode;
  arg[1] = speed;
  arg[2] = spread;
  arg[3] = white;
  arg[4] = palette;

  if (!radioSend(msg, sizeof(msg))) {
    Serial.println(F("failed to send ledstrip message"));
  }
}

void Ledstrip::gotMessage(uint8_t *arg) {
  stripChanged = true;
  if (arg[0] < NUM_MODES_ALL) {
    mode = arg[0];
  }
  speed = arg[1];
  spread = arg[2];
  white = arg[3];
  if (arg[4] < NUM_PALETTES) {
    palette = arg[4];
  }
  save();
}
