
#pragma once

typedef enum {
  LEDSTRIP_OFF,
  LEDSTRIP_COLOR,
  LEDSTRIP_NOISE,
  LEDSTRIP_WHITE,
  LEDSTRIP_PALETTE,
} ledstripMode_t;

void ledstripSend(uint8_t *arg);
void ledstripReceive(JsonObject &value);
