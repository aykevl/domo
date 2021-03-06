
#pragma once

typedef enum {
  LEDSTRIP_OFF,
  LEDSTRIP_RAINBOW,
  LEDSTRIP_NOISE,
  LEDSTRIP_FLAME,
  LEDSTRIP_WHITE,
  LEDSTRIP_PALETTE,
} ledstripMode_t;

const uint8_t LEDSTRIP_FLAG_RAINBOW_REVERSE = 0b10000000;
const uint8_t LEDSTRIP_FLAG_RAINBOW_RBG     = 0b01000000;
const uint8_t LEDSTRIP_FLAG_SPARKLES        = 0b00100000;
const uint8_t LEDSTRIP_MODE_MASK            = 0b00000111;

void ledstripSend(uint8_t *arg);
void ledstripReceive(JsonObject &value);
